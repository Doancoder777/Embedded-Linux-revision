#include <linux/module.h>  /* Thu vien nay dinh nghia cac macro nhu module_init va module_exit */
#include <linux/fs.h>      /* Thu vien nay dinh nghia cac ham allocate major & minor number */

#define DRIVER_AUTHOR "Sigma xxxxxxxx@gmail.com"
#define DRIVER_DESC   "Hello world kernel module"

struct m_foo_dev {
    /* dev_t: kiểu chứa major + minor; MKDEV(major, minor) ghép chúng lại */
    dev_t dev_num; /* dev_t dev = MKDEV(173, 0); */
} mdev;            // biến toàn cục giữ thông tin thiết bị

static int __init hello_world_init(void)   /* Constructor */
{
    /* 1.1 Dynamic allocating device number (cat /proc/devices) */
    /* alloc_chrdev_region: kernel tự chọn major
     * tham số: &dev_t, minor đầu = 0, số lượng = 1,
     * tên hiện trong /proc/devices */
    if (alloc_chrdev_region(&mdev.dev_num, 0, 1, "m-cdev") < 0) {
        pr_err("Failed to alloc chrdev region\n");   // pr_err = printk(KERN_ERR ...)
        return -1;                                   // cấp số thất bại: không nạp module
    }


    /* 1.2 Static allocating device number (cat /proc/devices) */
    /* register_chrdev_region(dev_t first, unsigned count, const char *name)
    * first : dev_t đã tự chọn bằng MKDEV(major, minor), truyền GIÁ TRỊ (không có &)
    * count : số lượng device number muốn đăng ký (ở đây là 1)
    * name  : tên hiện trong /proc/devices
    * Trả 0 = thành công; số âm = lỗi (ví dụ major đã có người dùng) */
    // mdev.dev_num = MKDEV(173, 0);
    // register_chrdev_region(mdev.dev_num, 1, "m-cdev");

    /* MAJOR/MINOR: tách major, minor ra từ dev_t; pr_info = printk(KERN_INFO ...) */
    pr_info("Major = %d Minor = %d\n", MAJOR(mdev.dev_num), MINOR(mdev.dev_num));

    pr_info("Hello world kernel module\n");
    return 0;
}

static void __exit hello_world_exit(void)  /* Destructor */
{
    /* 1. Unallocating device number */
    unregister_chrdev_region(mdev.dev_num, 1);   // trả device number (số lượng phải khớp)
    pr_info("Goodbye\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);





/*
Major number: số định danh của driver, kernel dùng để phân biệt các driver khác nhau
Minor number: số định danh của thiết bị, kernel dùng để phân biệt các thiết bị khác nhau
              của cùng 1 driver
dev_t có 32 bit: major 12 bit (0 - 4095), minor 20 bit (0 - 1048575)


Để cấp device number cho driver, có 2 cách:
1. Dynamic allocating: kernel tự chọn major number (khuyến nghị)
2. Static allocating: người lập trình tự chọn major number (không khuyến nghị,
   vì có thể trùng với driver khác; nếu trùng thì hàm trả về số âm = lỗi)


Kiểm tra driver đã đăng ký major chưa:

cat /proc/devices  : liệt kê các major đang được đăng ký, kèm tên driver
                     chia 2 nhóm: "Character devices" và "Block devices"
                     (chỉ có major + tên, KHÔNG có minor)
                     ví dụ: cat /proc/devices | grep m-cdev

Kiểm tra device file đã được tạo chưa:

ls -l /dev/        : liệt kê device file trong /dev; cột thứ 5 là "major, minor"
                     ví dụ: crw-rw---- 1 root tty 4, 0 ...   -> major = 4, minor = 0
                     ký tự đầu: c = character device, b = block device
ls -l /dev/ | grep "511, 0"   : lọc device file theo major, minor
                                (nếu không ra, thử: ls -l /dev/ | grep 511)

Trong Linux, mọi thứ đều được xem như file (everything is a file).
Application không nói chuyện trực tiếp với driver, mà phải đi qua device file:

application --> device file (device node, /dev/...)-->device number (major, minor)--> device driver--> phần cứng (hardware)
- device file    : cửa để application mở (open/read/write)
- device number  : kernel nhìn major để tìm đúng driver, minor để biết thiết bị nào
- device driver  : code điều khiển thiết bị, nhận open/read/write từ application

ls -l /dev/tty*    : ví dụ các device file của nhóm tty


Sau khi chạy lệnh insmod hello.ko và chạy dmesg để kiểm tra thu được:

[ 6769.184621] Major = 511 Minor = 0
[ 6769.184633] Hello world kernel module

Đã có major = 511, minor = 0 (cat /proc/devices thấy "511 m-cdev"),
nhưng khi chạy lệnh:

ls -l /dev/ | grep 511    : lọc theo major (giữa major và minor có nhiều dấu cách,
                            nên KHÔNG dùng grep "511, 0")
ls -l /dev/m_device       : xem thẳng theo tên (khi đã biết tên file)

Vì vậy ta cần sang step tiếp theo: tạo device file trong /dev/
để application có thể giao tiếp với driver.

*/