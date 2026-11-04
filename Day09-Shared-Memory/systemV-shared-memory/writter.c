#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main(int argc, char const *argv[])
{
    // ftok: tạo key từ file + id (65); writer và reader phải dùng CÙNG file + id
    key_t key = ftok("./shmfile", 65);

    // shmget: tạo segment 1024 byte (hoặc mở nếu đã có)
    int shmid = shmget(key, 1024, 0666 | IPC_CREAT);

    // shmat: gắn segment vào process, trả về con trỏ tới vùng nhớ chung
    char *str = (char*)shmat(shmid, (void *)0, 0);

    printf("Message to shared memory: ");

    // ghi vào vùng nhớ chung như ghi vào mảng bình thường
    fgets(str, 1024, stdin);

    // giữ process sống để reader đọc (thoát bằng Ctrl+C)
    while(1);

    // shmdt: gỡ segment khỏi process (không xóa segment)
    shmdt(str);
    return 0;
}

/*
 * ===== CHI TIẾT CÁC HÀM =====
 * ftok, shmget, shmat, shmdt: giống hệt phần chi tiết ở reader.c bên trên.
 * while(1): vòng lặp vô hạn, nên shmdt bên dưới không bao giờ chạy tới.
 */