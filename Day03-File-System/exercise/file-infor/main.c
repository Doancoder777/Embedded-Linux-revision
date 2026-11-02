// ============================================================================
// file-infor: in thông tin của một file (loại file, số i-node, kích thước,
// 3 mốc thời gian) bằng hàm lstat().
// Cách chạy: ./app <đường dẫn file>      ví dụ: ./app hello.txt
// ============================================================================

#include <stdio.h>          // printf, vfprintf, fflush, kiểu FILE
#include <fcntl.h>          // (chưa dùng trong bài này) open() và các cờ O_*
#include <stdint.h>         // uintmax_t, intmax_t (số nguyên cỡ lớn nhất)
#include <stdlib.h>         // exit, EXIT_FAILURE
#include <stdarg.h>         // va_list, va_start, va_end (hàm nhận số đối số tùy ý)
#include <unistd.h>         // (chưa dùng trong bài này) read, write, close...
#include <time.h>           // ctime (đổi số giây thành chuỗi ngày giờ)
#include <sys/types.h>      // các kiểu dữ liệu hệ thống (ino_t, off_t...)
#include <sys/stat.h>       // lstat, struct stat, các hằng S_IFMT, S_IFREG...
#include <sys/sysmacros.h>  // (chưa dùng trong bài này) major(), minor()

/**************************************************************************************************************
Macro definitions
***************************************************************************************************************/
/* for debbugging purposees only */
//#define DEBUG   0x01

// DEBUG(fmt, ...): macro in log kèm số dòng, dùng giống printf.
//   Nếu bật DEBUG (bỏ dấu // ở dòng #define phía trên): in ra stderr (LOG_ERROR).
//   Nếu không bật: in ra stdout (LOG_NORMAL).
//   "args..." là cách viết macro nhận số đối số tùy ý (kiểu của GNU C),
//   "## args" giúp bỏ dấu phẩy thừa khi không có đối số nào.
// Bài này chưa gọi DEBUG ở đâu.
#ifdef DEBUG
#undef DEBUG
#define DEBUG(fmt, args...) { log_printf (LOG_ERROR, "%d: ", __LINE__); log_printf (LOG_ERROR, fmt, ## args); }
#else
#undef DEBUG
#define DEBUG(fmt, args...) { log_printf (LOG_NORMAL, "%d: ", __LINE__); log_printf (LOG_NORMAL, fmt, ## args); }
#endif

/* errors level */
// Mức log: LOG_NORMAL in ra stdout (bình thường), LOG_ERROR in ra stderr (lỗi)
#define LOG_NORMAL      0x01
#define LOG_ERROR       0x02

/**************************************************************************************************************
Constants variables

/**************************************************************************************************************
Global variables
***************************************************************************************************************/
// Đường dẫn phân vùng kernel và rootfs trên bộ nhớ flash (MTD) của board nhúng.
// Bài này chưa dùng.
#define KERNEL_PARTITION    "/dev/mtd1"
#define ROOTFS_PARTITION    "/dev/mtd2"

// LOG bật/tắt toàn bộ việc in log: 0x01 = bật, 0 = tắt
#define LOG     0x01

// log(...): in một dòng log dạng  [tên file:tên hàm:số dòng] ==>> nội dung
//   "..." / __VA_ARGS__ : nhận số đối số tùy ý và đưa nguyên cho printf,
//                         nên dùng giống printf: log("Xin chao %s", ten)
//   __FILE__, __func__, __LINE__ : trình biên dịch tự thay bằng tên file,
//                         tên hàm, số dòng NƠI GỌI log
//   do { ... } while (0) : gom nhiều câu lệnh vào một khối, chạy đúng 1 lần
#define log(...) {  \
    do {    \
        if ((LOG)) { \
            printf("\n[%s:%s:%d] ==>> ", __FILE__, __func__, __LINE__); \
            printf(__VA_ARGS__); \
            printf("\n"); \
        } \
    } while (0); \
} // Logging

/**************************************************************************************************************
Function declaration
***************************************************************************************************************/


/**************************************************************************************************************
Function implementations
***************************************************************************************************************/

/*!
*   @brief          Print log
*                   <BR>
*                   This function will print log
*
*   @param[in]      level   LOG_NORMAL/LOG_ERROR
*   @param[in]      fmt     Format to print   
*   @return         NONE
*/
static void log_printf(int level, const char *fmt, ...) 
{
    // level là LOG_NORMAL thì in ra stdout, ngược lại in ra stderr
    FILE *fp = level == LOG_NORMAL ? stdout : stderr;
    va_list ap;                 // ap (argument pointer): danh sách các đối số đi sau fmt, số lượng tùy ý
    va_start(ap, fmt);          // bắt đầu đọc các đối số, bắt đầu ngay sau đối số fmt
    vfprintf(fp, fmt, ap);      // in giống fprintf nhưng nhận danh sách đối số ap
    va_end(ap);                 // kết thúc đọc đối số
    fflush(fp);                 // đẩy dữ liệu ra ngay, không để nằm lại trong vùng đệm
}

/*
how to run?


sigma@sigma-GF63-Thin-9RCX:~/Desktop/working_space/buoi3/exercise/file-infor$ ./app hello.txt
File Type: regular file

I-node number: 1310756, sizeof(uintmax_t): 8

File size: 6 bytes

Last status change:     Wed Oct 28 23:32:16 2026

Last file access:       Sun Jul  5 23:02:08 2026

Last file modification: Wed Oct 28 23:32:16 2026


*/
int main(int argc, char const *argv[])
{
    struct stat sb;     // sb (stat buffer): nơi kernel điền thông tin file (loại, i-node, kích thước, thời gian...)

    // argc = số tham số dòng lệnh, tính cả tên chương trình.
    // Cần đúng 2: argv[0] = ./app, argv[1] = tên file cần xem.
    if (argc != 2) {
        // argv[0] là tên chương trình đúng như bạn gõ lúc chạy (./app, ./file-infor...),
        // nên thông báo luôn khớp tên. %s là chỗ trống để điền argv[0].
        log("Usage: %s /path/to/file", argv[0]);
        exit(EXIT_FAILURE);     // thoát chương trình với mã lỗi (1)
    }

    // lstat: lấy thông tin của file argv[1], điền vào sb. Thành công trả về 0, lỗi trả về -1.
    // Khác stat: nếu file là symlink thì lstat báo thông tin của chính symlink, không đi theo nó.
    if (-1 == lstat(argv[1], &sb)) {
        log("lstat");           // chỉ in "lstat", chưa cho biết lý do lỗi (không tồn tại, không có quyền...)
        exit(EXIT_FAILURE);
    }

    printf("File Type: ");
    // st_mode chứa cả loại file lẫn quyền (rwx).
    // S_IFMT là mặt nạ (mask): "& S_IFMT" chỉ giữ lại phần loại file, rồi so với từng loại.
    // Chữ cái ở cuối mỗi dòng là ký tự đầu tiên khi chạy ls -l.
    switch (sb.st_mode & S_IFMT) {
        case S_IFBLK:   printf("block device\n");       break;  // b
        case S_IFCHR:   printf("character device\n");   break;  // c
        case S_IFDIR:   printf("directory\n");          break;  // d
        case S_IFIFO:   printf("FIFO/pipe\n");          break;  // p
        case S_IFLNK:   printf("symlink\n");            break;  // l
        case S_IFREG:   printf("regular file\n");       break;  // -
        case S_IFSOCK:  printf("socket\n");             break;  // s
        default:        printf("unknown?\n");
    }
    
    // st_ino  : số i-node của file.
    //           Kiểu của st_ino tùy hệ thống nên ép về (uintmax_t) rồi in bằng %ju.
    // sizeof(uintmax_t): kích thước của kiểu uintmax_t (8 byte), in thêm để tham khảo.
    printf("\nI-node number: %ju, sizeof(uintmax_t): %jd\n\n", (uintmax_t)sb.st_ino, sizeof(uintmax_t));

    // st_size : kích thước file, đơn vị byte. Ép về (intmax_t) rồi in bằng %jd.
    printf("File size: %jd bytes\n\n", (intmax_t)sb.st_size);

    // st_ctim, st_atim, st_mtim là 3 mốc thời gian (kiểu struct timespec);
    // .tv_sec là phần giây.
    //   st_ctim: lần đổi trạng thái cuối (đổi quyền, đổi chủ...)
    //   st_atim: lần truy cập cuối
    //   st_mtim: lần sửa nội dung cuối
    // ctime(): đổi số giây thành chuỗi ngày giờ. Chuỗi này đã có sẵn ký tự \n ở cuối,
    // cộng với \n trong printf nên mỗi dòng bị thừa một dòng trống.
    printf("Last status change:     %s\n", ctime(&sb.st_ctim.tv_sec));
    printf("Last file access:       %s\n", ctime(&sb.st_atim.tv_sec));
    printf("Last file modification: %s\n", ctime(&sb.st_mtim.tv_sec));

    
    return 0;
}