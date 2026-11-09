
#include <linux/module.h> /* Thu vien nay dinh nghia cac macro nhu module_init va module_exit */
#include <linux/fs.h>     /* Thu vien nay dinh nghia cac ham allocate major & minor number */

#define DRIVER_AUTHOR "Sigma xxxxxxxx@gmail.com" // thông tin tác giả (author)
#define DRIVER_DESC "Hello world kernel module"  // mô tả module (description)
#define DRIVER_VERS "1.0"                        // phiên bản (version)

/* Constructor */
    static int __init hello_world_init(void) // __init: hàm chỉ chạy 1 lần khi nạp module (insmod), xong kernel giải phóng bộ nhớ
{
    printk(KERN_INFO "Hello world kernel module\n");
    return 0;                                        // trả 0 = nạp thành công; số khác 0 = lỗi, module không được nạp
}

/* Destructor */
static void __exit hello_world_exit(void) // __exit: hàm chạy khi gỡ module (rmmod)
{
    printk(KERN_INFO "Goodbye\n"); // in log lúc gỡ module
}

module_init(hello_world_init); // đăng ký hàm chạy khi nạp module (entry point)
module_exit(hello_world_exit); // đăng ký hàm chạy khi gỡ module

MODULE_LICENSE("GPL");           // giấy phép cho phép sử dụng free (GPL: General Public License) nếu không có dòng này, kernel sẽ cảnh báo khi nạp module
MODULE_AUTHOR(DRIVER_AUTHOR);    // gắn tác giả vào module (xem bằng modinfo)
MODULE_DESCRIPTION(DRIVER_DESC); // gắn mô tả vào module
MODULE_VERSION(DRIVER_VERS);     // gắn phiên bản vào module


/*
Các loại macro printk trong kernel:

KERN_EMERG: mức khẩn cấp (emergency)
KERN_ALERT: mức cảnh báo (alert)
KERN_CRIT: mức nghiêm trọng (critical)
KERN_ERR: mức lỗi (error)
KERN_WARNING: mức cảnh báo (warning)
KERN_NOTICE: mức thông báo (notice)
KERN_INFO: mức thông tin (informational)



Câu lệnh chạy

sudo insmod hello.ko: nạp module vào kernel
sudo rmmod hello: gỡ module khỏi kernel

dmesg: xem log của kernel
dmesg | tail: xem 10 dòng log cuối cùng của kernel

lsmod: xem danh sách các module đang nạp trong kernel
lsmod | grep hello: xem thông tin module hello

modinfo hello.ko: xem thông tin module hello




Để chạy được build Kernel module cần tải source header của thiết đó 

câu lệnh cài đặt source header của kernel trên Ubuntu:
sudo apt-get install linux-headers-$(uname -r)

sau khi cài nó sẽ nằm ở 
/lib/modules/$(uname -r)/build


Native compiler (biên dịch gốc)
  - Build trên chính máy sẽ chạy chương trình (host = target).
  - Ví dụ: laptop x86 build ra file chạy trên laptop x86 (gcc).

Cross compiler (biên dịch chéo)
  - Build trên máy này (host), chương trình chạy trên máy khác (target)
    có kiến trúc CPU khác (ARM, RISC-V, ...).
  - Cần cross toolchain, ví dụ: arm-linux-gnueabihf-gcc, riscv64-linux-gnu-gcc.
  - Với kernel module: khai báo thêm ARCH=<kiến trúc> CROSS_COMPILE=<tiền tố toolchain>,
    và KDIR trỏ tới source kernel của target (không phải kernel của laptop).

*/

