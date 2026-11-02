
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char const *argv[])   /* Cấp phát stack frame cho hàm main() */
{
    /* code */
    pid_t child_pid;                /* Lưu trong stack frame của main() */
    int counter = 2;                /* Lưu trong frame của main() */

    printf("Gia tri khoi tao cua counter: %d\n", counter);

    /* 
     * KHOẢNH KHẮC MA THUẬT: Gọi fork() đúng 1 LẦN DUY NHẤT.
     * Từ dòng này trở đi, tiến trình Cha nhân bản thành 2 thực thể (Cha và Con).
     * Cả 2 cùng chạy song song các dòng code phía dưới với bộ nhớ độc lập.
     */
    child_pid = fork();         
    
    if (child_pid >= 0) {
        /* CẢ CHA VÀ CON đều đi qua cánh cửa này vì fork() thành công (trả về >= 0) 
        
        2 cha con chay song song, ai chay nhanh hon thi in ra truoc, ai cham hon thi in ra sau.

        */
        
        if (0 == child_pid) {       /* Process con */
            /* CHỈ CÓ CON lọt vào đây (vì hệ điều hành gán child_pid = 0 cho Con) */
            
            /* counter tăng thành 3 ở vùng nhớ riêng của Con */
            printf("\nIm the child process, counter: %d\n", ++counter);
            printf("My PID is: %d, my parent PID is: %d\n", getpid(), getppid());

            sleep(3);               /* Con ngủ 5 giây, trong khi Cha bị kẹt vô hạn ở dưới */
            
        } else {                    /* Process cha */
            /* CHỈ CÓ CHA lọt vào đây (vì hệ điều hành gán child_pid = PID của Con, luôn > 0) */
            
            /* counter tăng thành 3 ở vùng nhớ riêng của Cha (không bị cộng dồn thành 4) */
            printf("\nIm the parent process, counter: %d\n", ++counter);
            printf("My PID is: %d\n", getpid());
            
            /* Ông Cha bị kẹt vô hạn ở đây, hoàn toàn KHÔNG đẻ thêm con nào nữa */
             while (1);
        }
    } else {
        printf("fork() unsuccessfully\n");      // fork() trả về -1 nếu hệ thống lỗi/hết RAM
    }

    return 0;
}