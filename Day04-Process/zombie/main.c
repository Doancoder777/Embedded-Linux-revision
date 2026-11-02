#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char const *argv[])   /* Cấp phát stack frame cho hàm main() */
{
    pid_t child_pid;                /* Lưu trong stack frame của main() */
    int status;
    int counter = 0;
 
    child_pid = fork();         
    if (child_pid >= 0) {
        if (0 == child_pid) {       /* Process con */
            printf("Child process termination\n");
            
            /* 
             * NGUYÊN NHÂN 1 TẠO ZOMBIE:
             * Tiến trình con không làm gì cả, đi tới return 0 và CHẾT NGAY LẬP TỨC.
             * Dù con đã chết, Kernel vẫn phải giữ lại PID và mã thoát (exit status) 
             * trong Process Table để chờ tiến trình cha đến lấy.
             */
            return 0; 
            
        } else {                    /* Process cha */
            /* 
             * NGUYÊN NHÂN 2 TẠO ZOMBIE:
             * Thằng cha bị kẹt ở vòng lặp vô hạn while(1), không bao giờ thoát ra.
             * Lệnh wait(&status) bị comment lại, nghĩa là cha KHÔNG thèm đọc giấy 
             * chứng tử và KHÔNG dọn dẹp xác cho con.
             * => Đứa con chính thức biến thành ZOMBIE (defunct).
             */
            while(1);  
            
            // wait(&status); /* Nếu mở khóa dòng này, cha sẽ dọn xác con và hết Zombie */
        }
    } else {                        /* fork() return -1 nếu lỗi. */
        printf("fork() unsuccessfully\n"); 
    }

    return 0;
}