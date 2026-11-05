#include <stdio.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>

int main(void) 
{
    int fd;
    char text[16] = {0};

    sprintf(text, "hello word\n");

    // 1. Mở hoặc tạo file dùng chung (Shared Resource)
    if ((fd = open("./text.txt", O_RDWR | O_CREAT, 0666)) == -1) {
        printf("can not create file \n");
        return -1;
    }
    printf("create/open file text.txt\n");

    // ============================================================
    // [BƯỚC 1: XIN KHÓA ĐỘC QUYỀN (LOCK SECTION)]
    // Dùng LOCK_EX trước khi ghi để ngăn mọi tiến trình khác can thiệp
    // ============================================================
    if (flock(fd, LOCK_EX) == -1) {
        printf("can not set write lock (LOCK_EX)\n");
        close(fd);
        return -1;
    }
    printf("set write lock successfully\n");

    // ============================================================
    // [BƯỚC 2: CRITICAL SECTION - THAO TÁC TRÊN TÀI NGUYÊN]
    // Được bảo vệ an toàn hoàn toàn bên trong khóa LOCK_EX
    // ============================================================
    if (write(fd, text, sizeof(text) - 1) == -1) {
        printf("can not write file \n");
    } else {
        printf("write file: %s", text);
    }
    // ============================================================

    // Mô phỏng việc giữ khóa một lúc (ví dụ 5 giây) để test việc Process B bị chặn
    printf("Holding lock for 5 seconds...\n");
    sleep(5);

    // ============================================================
    // [BƯỚC 3: MỞ KHÓA (UNLOCK SECTION)]
    // Giải phóng tài nguyên cho tiến trình khác sử dụng
    // ============================================================
    if (flock(fd, LOCK_UN) == -1) {
        printf("can not unlock file\n");
    } else {
        printf("unlocked file\n");
    }

    close(fd);
    return 0;
}