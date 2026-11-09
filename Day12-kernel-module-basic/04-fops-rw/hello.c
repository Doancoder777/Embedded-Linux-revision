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

static struct file_operations fops =
{
    .owner      = THIS_MODULE,     // module sở hữu fops, chống bị rmmod khi đang dùng
    .read       = m_read,
    .write      = m_write,
    .open       = m_open,
    .release    = m_release,       // release: ứng với close()
    .llseek     = default_llseek,  // llseek: ứng với lseek()
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
static ssize_t m_read(struct file *filp, char __user *user_buffer, size_t size, loff_t *offset)
{
    size_t to_read;                // số byte thật sự sẽ đọc
    pr_info("System call read() called...!!!\n");
    to_read = (size > mdev.size - *offset) ? (mdev.size - *offset) : size; 
    if (copy_to_user(user_buffer, mdev.kmalloc_ptr + *offset, to_read))
        return -EFAULT;            
    *offset += to_read;          
    return to_read;               
}

static ssize_t m_write(struct file *filp, const char __user *user_buffer, size_t size, loff_t *offset)
{
    size_t to_write;               
    pr_info("System call write() called...!!!\n");
    /* Check size doesn't exceed our mapped area size */
    to_write = (size + *offset > NPAGES * PAGE_SIZE) ? (NPAGES * PAGE_SIZE - *offset) : size;
    memset(mdev.kmalloc_ptr, 0, NPAGES * PAGE_SIZE);  
    if (copy_from_user(mdev.kmalloc_ptr + *offset, user_buffer, to_write) != 0)
        return -EFAULT;
    pr_info("Data from usr: %s\n", mdev.kmalloc_ptr);
    /*
    Thêm \n để log không bị delay khi write.
    Thiếu \n, kernel (printk) nghĩ dòng chưa kết thúc nên giữ lại trong bộ đệm,
    chờ dòng log kế tiếp mới đẩy ra dmesg, nên log luôn hiện chậm một nhịp.
    */
    *offset += to_write;
    mdev.size = *offset;      
    return to_write;               
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
    cdev_init(&mdev.m_cdev, &fops);

    /* 02.2 Adding character device to the system*/
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



