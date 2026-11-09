=====
#include <linux/module.h>  /* Thu vien nay dinh nghia cac macro nhu module_init/module_exit */
#include <linux/fs.h>      /* Thu vien nay dinh nghia cac ham allocate major/minor number */
#include <linux/device.h>  /* Thu vien nay dinh nghia cac ham class_create/device_create */

#define DRIVER_AUTHOR "Sigma xxxxxxxx@gmail.com"
#define DRIVER_DESC   "Hello world kernel module"

struct m_foo_dev {
    dev_t dev_num;
    struct class *m_class;   // con trỏ tới class (nhóm thiết bị), hiện ở /sys/class/
} mdev;

static int __init hello_world_init(void)   /* Constructor */
{
    /* 1. Allocating device number (cat /proc/devices) */
    if (alloc_chrdev_region(&mdev.dev_num, 0, 1, "m-cdev") < 0) {
        pr_err("Failed to alloc chrdev region\n");
        return -1;
    }
    pr_info("Major = %d Minor = %d\n", MAJOR(mdev.dev_num), MINOR(mdev.dev_num));

    /* 02. Creating struct class */

    /* class_create: tạo class trong /sys/class/m_class
     * udev: dịch vụ chạy nền ở user space, nghe kernel báo có thiết bị mới
     *       rồi tự tạo file trong /dev (kernel không tự tạo file /dev)
     * kernel cũ (trước 6.4)  : class_create(THIS_MODULE, "m_class")   // 2 tham số
     * kernel mới (từ 6.4) chỉ còn 1 tham số, không có THIS_MODULE */
    if ((mdev.m_class = class_create("m_class")) == NULL) {
        pr_err("Cannot create the struct class for my device\n");
        goto rm_device_numb;                 // lỗi: nhảy xuống dọn Step 1
    }

    /* 03. Creating device */

    /* device_create(class, parent, dev_num, drvdata, tên)
     * parent = NULL: không có thiết bị cha; drvdata = NULL: không gắn dữ liệu riêng
     * tên "m_device": tạo ra file /dev/m_device */
    if ((device_create(mdev.m_class, NULL, mdev.dev_num, NULL, "m_device")) == NULL) {
        pr_err("Cannot create my device\n");
        goto rm_class;                       // lỗi: nhảy xuống dọn class, rồi device number
    }

    pr_info("Hello world kernel module\n");
    return 0;

/* Dọn dẹp khi lỗi: chỉ hủy những thứ ĐÃ tạo xong, theo thứ tự ngược
 * (cái tạo sau thì hủy trước). Cái vừa bị lỗi thì chưa tạo vì tạo không thành công, không cần hủy.
 * nhảy vào nhãn nào thì chạy từ nhãn đó trở xuống */
rm_class:
    class_destroy(mdev.m_class);                // hủy class
rm_device_numb:
    unregister_chrdev_region(mdev.dev_num, 1);  // trả device number
    return -1;
}

static void __exit hello_world_exit(void)  /* Destructor */
{
    /* Gỡ module: cũng hủy ngược thứ tự 03 -> 02 -> 01 */
    device_destroy(mdev.m_class, mdev.dev_num);     /* 03 */   // xóa device file /dev/m_device
    class_destroy(mdev.m_class);                    /* 02 */   // xóa class
    unregister_chrdev_region(mdev.dev_num, 1);      /* 01 */   // trả device number

    pr_info("Goodbye\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);