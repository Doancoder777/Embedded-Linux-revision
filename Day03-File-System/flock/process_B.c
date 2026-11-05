#include <stdio.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>

int main(void) 
{
    int fd;
    char buf[16] = {0};

    // 1. Mở file dùng chung
    if ((fd = open("./text.txt", O_RDWR)) == -1) {
        printf("can not open file \n");
        return -1;
    }
    printf("open file text.txt successfully\n");

    // ============================================================
    // [BƯỚC 1: XIN KHÓA ĐỌC (LOCK SECTION)]
    // Dùng LOCK_SH để đảm bảo không ai đang ghi đè dở dang trong lúc đọc.
    // Nếu Process A đang giữ LOCK_EX, lệnh này sẽ đứng đợi (block).
    // ============================================================
    printf("Waiting for read lock (LOCK_SH)...\n");
    if (flock(fd, LOCK_SH) == -1) {
        printf("can not get read lock\n");
        close(fd);
        return -1;
    }
    printf("get read lock successfully\n");

    // ============================================================
    // [BƯỚC 2: CRITICAL SECTION - ĐỌC TÀI NGUYÊN AN TOÀN]
    // ============================================================
    if (read(fd, buf, sizeof(buf) - 1) == -1) {
        printf("can not read file \n");
    } else {
        printf("Read data: %s", buf);
    }
    // ============================================================

    // ============================================================
    // [BƯỚC 3: MỞ KHÓA (UNLOCK SECTION)]
    // ============================================================
    if (flock(fd, LOCK_UN) == -1) {
        printf("can not unlock file\n");
    } else {
        printf("unlocked file\n");
    }

    close(fd);
    return 0;
}