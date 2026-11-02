#include <stdio.h>
#include <sys/stat.h> 
#include <sys/file.h> 
#include <unistd.h> 
#include <fcntl.h> 
#include <string.h>

int main(void) 
{ 
    int fd; 

    char text[10] = {0}; 
    struct flock fl; 

    sprintf(text, "doan1511"); 
 
    if((fd=open("./test.txt", O_RDWR)) == -1) { 
        printf("can not open file \n"); 
        return 0; 
    } else { 
        printf("open file test.txt \n"); 
    } 

    fl.l_start = 0;         /* Offset where the lock begins */
    fl.l_len = 5;           /* Number of bytes to lock; 0 means "until EOF" */
    fl.l_type = F_WRLCK;    /* Lock type: F_RDLCK, F_WRLCK, F_UNLCK */
    fl.l_whence = SEEK_SET; /* How to interpret 'l_start': SEEK_SET, SEEK_CUR, SEEK_END */

    if(fcntl(fd, F_SETLK, &fl) == -1) { 
        printf("can not set write lock byte 0-4\n"); 
    } 

    fl.l_start = 6; 
    fl.l_len = 8;     // strlen("doan1511") là 8
    fl.l_type = F_WRLCK; 
    fl.l_whence = SEEK_SET; 

    if(fcntl(fd, F_SETLK, &fl) == -1) {  //SETLK dùng để đặt lock và ghi xuống file còn GETLK dùng để lấy thông tin 
        printf("can not set write lock byte 6-13\n"); 
    } else { 
        printf("set write lock byte 6-13\n"); 
        lseek(fd, 6, SEEK_SET);

        if(write(fd, text, strlen(text)) == -1) { 
            printf("can not write file \n"); 
            return 0; 
        } else {
            printf("write file \n");
        } 
    } 

    close(fd); 
    return 0; 
} 
/* =====================================================================
 * GIẢI THÍCH LUỒNG CHẠY CỦA PROCESS A VÀ PROCESS B (RECORD LOCKING)
 * =====================================================================
 * 
 * [PROCESS A] - NGƯỜI TẠO FILE VÀ GIỮ KHÓA
 * - Ghi chuỗi gốc: "hello word\n" (11 byte)
 * - Trạng thái file (theo offset/index):
 *      h(0) e(1) l(2) l(3) o(4)  (5) w(6) o(7) r(8) d(9) \n(10)
 * - Lệnh khóa: start = 0, len = 5 
 *      => Khóa thành công vùng chữ "h e l l o" (byte 0 đến 4) trên file.
 * 
 * [PROCESS B] - NGƯỜI ĐẾN SAU THỬ XIN KHÓA
 * - Lệnh khóa 1: start = 0, len = 5 
 *      => Thử xin khóa vùng "h e l l o" trên file.
 *      => BỊ TỪ CHỐI (vì Process A đang giữ chặt vùng này).
 * 
 * - Lệnh khóa 2: start = 6, len = 8 
 *      => Đổi chiến thuật, xin khóa 8 byte trên file tính từ vị trí số 6 (byte 6 đến 13).
 *      => THÀNH CÔNG (vì vùng này không có ai khóa).
 * 
 * - Lệnh lseek: lseek(fd, 6, SEEK_SET)
 *      => Dời con trỏ ghi (chuột) đến đúng vị trí an toàn số 6 (ngay chữ 'w').
 * 
 * - Lệnh write: write(fd, "doan1511", 8)
 *      => Ghi 8 byte "doan1511" đè lên file, bắt đầu từ vị trí số 6.
 *      => Vùng chữ cũ ở vị trí 6-13 ("w o r d \n") bị đè nát hoàn toàn.
 * 
 * =====================================================================
 * KẾT QUẢ OUTPUT CUỐI CÙNG TRONG FILE (test.txt): 
 * => hello doan1511
 * ===================================================================== */