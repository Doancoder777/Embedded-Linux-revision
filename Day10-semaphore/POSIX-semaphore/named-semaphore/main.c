#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>

#define     POSIX_SEM_NAMED       "/named_app"
#define     R_W_PERM              0666
#define     SEM_WAITING_TIMEOUT   10000 // mili-giây (10 giây)
#define     MSECS_IN_SEC          1000

int main(int argc, char *argv[])
{   
    char c;
    int ret = -1;
    int current_value;
    sem_t *sem; // Con trỏ đến semaphore có tên 
    struct timespec timeout;

    // --- 1. KHỞI TẠO HOẶC MỞ SEMAPHORE CÓ TÊN ---
    sem = sem_open(POSIX_SEM_NAMED, O_CREAT | O_EXCL, R_W_PERM, 1);
    if (sem == SEM_FAILED) {
        if (errno != EEXIST) {
            printf("Failed to open semaphore error: %s\n", strerror(errno));
            return -1;
        }

        // Đã tồn tại thì chỉ mở ra để dùng chung
        printf("%s, Reading available semaphore.\n", argv[0]);
        sem = sem_open(POSIX_SEM_NAMED, 0);
        if (sem == SEM_FAILED) {
            printf("Failed to open semaphore error: %s\n", strerror(errno));
            return -1;
        }
    }

    // Đọc giá trị bộ đếm hiện tại
    sem_getvalue(sem, &current_value);
    printf("Current semaphore value = %d.\n", current_value);

    // --- 2. XIN KHÓA CÓ GIỚI HẠN THỜI GIAN (TIMEOUT 10 GIÂY) ---
    if (clock_gettime(CLOCK_REALTIME, &timeout) == -1) {
        printf("Failed to get current time error: %s\n", strerror(errno));
        return -1;
    }
    
    timeout.tv_sec += SEM_WAITING_TIMEOUT / MSECS_IN_SEC; // Thiết lập mốc giờ tương lai
    ret = sem_timedwait(sem, &timeout);                   // Giảm bộ đếm đi 1 (chờ tối đa 10s)
    if (ret == -1) {
        printf("Failed to wait semaphore error: %s\n", strerror(errno));
        return -1;
    }

    // --- 3. VÙNG NGUY HIỂM / CRITICAL SECTION (GIỮ KHÓA) ---
    printf("%s, Please type any character: ", argv[0]);
    c = getchar();

    // --- 4. NHẢ KHÓA ---
    ret = sem_post(sem); // Tăng bộ đếm lên 1 để nhường quyền
    if (ret == -1) {
        printf("Failed to release semaphore error: %s\n", strerror(errno));
        return -1;
    }

    // Dừng lại chờ gõ phím trước khi thoát
    printf("\n%s, Please type any character to exit ...\n", argv[0]);
    getchar();

    // --- 5. ĐÓNG VÀ XÓA SEMAPHORE KHỎI HỆ THỐNG ---
    ret = sem_close(sem);
    if (ret == -1) {
        printf("%s, Failed to close semaphore error: %s\n", argv[0], strerror(errno));
        return -1;
    }

    // Xóa tên khỏi /dev/shm/ (lưu ý: truyền tên chuỗi, không truyền &sem)
    sem_unlink(POSIX_SEM_NAMED);

    return ret;
}