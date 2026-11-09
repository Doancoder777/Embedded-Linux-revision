/******************************************************************************
*  @brief      Simple Linux device driver
*
*  @author     Sigma - sigmaxxxxx@gmail.com
*******************************************************************************/

#include <linux/module.h>  /* Thu vien nay dinh nghia cac macro nhu module_init/module_exit */
#include <linux/fs.h>      /* Thu vien nay dinh nghia cac ham allocate major/minor number */
#include <linux/device.h>  /* Thu vien nay dinh nghia cac ham class_create/device_create */
#include <linux/cdev.h>    /* Thu vien nay dinh nghia cac ham cdev_init/cdev_add */
#include <linux/slab.h>    /* Thu vien nay dinh nghia cac ham kmalloc/kfree */
#include <linux/uaccess.h> /* Thu vien nay dinh nghia cac ham copy_to_user/copy_from_user */

#define DRIVER_AUTHOR "sigma xxxxxxxx@gmail.com"
#define DRIVER_DESC   "Hello world kernel module"

#define NPAGES  1                  // số page bộ nhớ cần cấp (1 page thường = 4096 byte)

struct m_foo_dev {
    int size;                      // số byte dữ liệu hiện có trong buffer
    char *kmalloc_ptr;             // con trỏ tới vùng nhớ kernel (cấp bằng kmalloc)
    dev_t dev_num;
    struct class *m_class;
    struct cdev m_cdev;            // cdev: đại diện character device trong kernel
} mdev;

/*  Function Prototypes */
static int      __init hello_world_init(void);
static void     __exit hello_world_exit(void);
static int      m_open(struct inode *inode, struct file *file);
static int      m_release(struct inode *inode, struct file *file);
static ssize_t  m_read(struct file *filp, char __user *user_buf, size_t size, loff_t *offset);
static ssize_t  m_write(struct file *filp, const char __user *user_buf, size_t size, loff_t *offset);

/* fops: bảng nối "system call của application" với "hàm của driver"
 * application gọi open/read/write/close -> kernel gọi m_open/m_read/m_write/m_release */
static struct file_operations fops =
{
    .owner      = THIS_MODULE,     // module sở hữu fops, chống bị rmmod khi đang dùng
    .read       = m_read,
    .write      = m_write,
    .open       = m_open,
    .release    = m_release,       // release: ứng với close()
};

/* This function will be called when we open the Device file */
static int m_open(struct inode *inode, struct file *file)
{
    pr_info("System call open() called...!!!\n");
    return 0;                      // 0 = cho phép mở
}

/* This function will be called when we close the Device file */
static int m_release(struct inode *inode, struct file *file)
{
    pr_info("System call close() called...!!!\n");
    return 0;
}

/* This function will be called when we read the Device file */
/* __user: con trỏ thuộc user space, kernel không được truy cập trực tiếp */
static ssize_t m_read(struct file *filp, char __user *user_buffer, size_t size, loff_t *offset)
{
    size_t to_read;                // số byte thật sự sẽ đọc

    pr_info("System call read() called...!!!\n");

    /* Check size doesn't exceed our mapped area size */
    /* không đọc quá số byte đang có */
    to_read = (size > mdev.size - *offset) ? (mdev.size - *offset) : size; // còn bao nhiêu byte thì đọc bấy nhiêu

    /* Copy from mapped area to user buffer */
    //offset: vị trí đọc hiện tại, do kernel tự quản lý offset, không cần user quản lý
    //để  kernel biết lần read sau đọc tiếp từ đâu và bộ nhớ kernel còn bao nhiêu byte để đọc, mỗi lần đọc xong phải cập nhật offset và size
    /* copy_to_user(đích user, nguồn kernel, số byte): trả 0 = OK, khác 0 = lỗi */
    if (copy_to_user(user_buffer, mdev.kmalloc_ptr + *offset, to_read))
        return -EFAULT;            // EFAULT: địa chỉ user không hợp lệ

    *offset += to_read;            // dời vị trí đọc, lần read sau đọc tiếp từ đây

    return to_read;                // trả số byte đã đọc; 0 = hết dữ liệu (EOF)
}

/* This function will be called when we write the Device file */

/*
 * filp: con trỏ tới struct file, đại diện MỘT lần mở file
 * (mỗi lần application gọi open thì kernel tạo một struct file riêng)
 * struct file chứa:
 * - f_inode      : con trỏ tới inode của file
 * - f_op         : con trỏ tới file_operations (fops của driver)
 * - f_pos        : vị trí đọc/ghi hiện tại (offset)
 * - f_flags      : cờ mở file (O_RDONLY, O_WRONLY, O_RDWR...) 
 * - private_data : driver tự lưu dữ liệu riêng (bài này chưa dùng)
 */
static ssize_t m_write(struct file *filp, const char __user *user_buffer, size_t size, loff_t *offset)
{
    size_t to_write;               // số byte thật sự sẽ ghi

    pr_info("System call write() called...!!!\n");

    /* Check size doesn't exceed our mapped area size */
    /* không ghi quá kích thước buffer: cắt bớt nếu vượt */
    to_write = (size + *offset > NPAGES * PAGE_SIZE) ? (NPAGES * PAGE_SIZE - *offset) : size;
    // 1 PAGE = 4096 byte, NPAGES = 1, tổng cộng buffer có thể chứa tối đa 4096 byte
    /* Copy from user buffer to mapped area */
    memset(mdev.kmalloc_ptr, 0, NPAGES * PAGE_SIZE);   // xóa buffer về 0 trước khi ghi
    /* copy_from_user(đích kernel, nguồn user, số byte): trả 0 = OK */
    if (copy_from_user(mdev.kmalloc_ptr + *offset, user_buffer, to_write) != 0)
        return -EFAULT;

    pr_info("Data from usr: %s", mdev.kmalloc_ptr);

    *offset += to_write;
    mdev.size = *offset;           // cập nhật số byte đang có để m_read dùng

    return to_write;               // trả số byte đã ghi
}

static int __init hello_world_init(void)   /* Constructor */
{
    /* 1. Allocating device number (cat /proc/devices)*/
    if (alloc_chrdev_region(&mdev.dev_num, 0, 1, "m-cdev") < 0) {
        pr_err("Failed to alloc chrdev region\n");
        return -1;
    }
    pr_info("Major = %d Minor = %d\n", MAJOR(mdev.dev_num), MINOR(mdev.dev_num));

    /* 02.1 Creating cdev structure */
    /* cdev_init: gắn bảng fops vào cdev */
    cdev_init(&mdev.m_cdev, &fops);

    /* 02.2 Adding character device to the system*/
    /* cdev_add(cdev, device number, số lượng): đăng ký cdev với kernel
     * từ lúc này application có thể gọi open/read/write vào device */
    if ((cdev_add(&mdev.m_cdev, mdev.dev_num, 1)) < 0) {
        pr_err("Cannot add the device to the system\n");
        goto rm_device_numb;
    }

    /* 03. Creating struct class */
    /* kernel từ 6.4 chỉ còn 1 tham số (bỏ THIS_MODULE) */
    if ((mdev.m_class = class_create("m_class")) == NULL) {
        pr_err("Cannot create the struct class for my device\n");
        goto rm_cdev;
    }

    /* 04. Creating device*/
    if ((device_create(mdev.m_class, NULL, mdev.dev_num, NULL, "m_device")) == NULL) {
        pr_err("Cannot create my device\n");
        goto rm_class;
    }

    /* 05. Creating Physical memory*/
    /* kmalloc(số byte, cờ): cấp vùng nhớ liên tục trong kernel
     * GFP_KERNEL: cho phép ngủ chờ nếu thiếu bộ nhớ
     * thất bại trả NULL (0) */
    if ((mdev.kmalloc_ptr = kmalloc(NPAGES * PAGE_SIZE, GFP_KERNEL)) == 0) {
        pr_err("Cannot allocate memory in kernel\n");
        goto rm_device;
    }

    pr_info("Hello world kernel module\n");
    return 0;

/* Dọn dẹp khi lỗi: hủy ngược thứ tự đã tạo, nhảy vào nhãn nào chạy từ đó trở xuống */
rm_device:
    device_destroy(mdev.m_class, mdev.dev_num);
rm_class:
    class_destroy(mdev.m_class);
rm_cdev:
    cdev_del(&mdev.m_cdev);
rm_device_numb:
    unregister_chrdev_region(mdev.dev_num, 1);
    return -1;
}

static void __exit hello_world_exit(void)  /* Destructor */
{
    kfree(mdev.kmalloc_ptr);                        /* 05 */   // kfree: trả vùng nhớ kmalloc
    device_destroy(mdev.m_class, mdev.dev_num);     /* 04 */
    class_destroy(mdev.m_class);                    /* 03 */
    cdev_del(&mdev.m_cdev);                         /* 02 */   // cdev_del: gỡ cdev khỏi kernel
    unregister_chrdev_region(mdev.dev_num, 1);      /* 01 */

    pr_info("Goodbye\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);



/*

5 Ô (luồng chạy: application dùng thiết bị, đi từ trên xuống)

Ô 1. application
     chương trình người dùng: open("/dev/m_device"), read(), write(), close()
     |
     v
Ô 2. device file (device node, /dev/...)
     file /dev/m_device, là "cửa" để application mở
     |
     v
Ô 3. device number (major, minor)
     ví dụ 511, 0; kernel nhìn số này để tìm đúng driver
     |
     v
Ô 4. device driver
     các hàm m_open / m_read / m_write / m_release (gọi qua fops)
     |
     v
Ô 5. phần cứng (hardware)
     bài này chưa có phần cứng thật, buffer kmalloc đóng vai thiết bị




Step 1. Allocate device number   -> dựng ô 3: xin số quầy 511,0
                                    (alloc_chrdev_region)
Step 2. Create Device File       -> dựng ô 2: dán bảng tên /dev/m_device
                                    (class_create + device_create)
Step 3. Register File operations -> dựng ô 4: thuê nhân viên, đăng ký
                                    họ làm quầy 511,0
                                    (cdev_init + cdev_add, fops)


inode table đại diện cho tất cả các file trong hệ thống, mỗi file có 1 inode riêng.
open file table đại diện cho tất cả các file đang được mở, mỗi file đang mở có 1 entry riêng.

struct cdev là cấu trúc dữ liệu đại diện cho 1 thiết bị ký tự (character device) trong kernel là 1 phần của inode table.
struct cdev được liên kết với 1 file_operations (fops) để xác định các hành vi của thiết bị ký tự đó.

struct cdev {
    struct kobject kobj;          // đại diện cho thiết bị trong sysfs
    struct module *owner;         // con trỏ đến module sở hữu thiết bị
    const struct file_operations *ops;  // con trỏ đến các hàm xử lý file operations (read, write, open, release, ioctl, ...)
    struct list_head list;        // danh sách liên kết các cdev trong kernel
    dev_t dev;                    // số thiết bị (major/minor) của cdev
}
*/