#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>         // pipe, fork, read, write, close
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>       // wait

#define MSG_SIZE    16      // mỗi tin nhắn 16 byte (15 ký tự + '\0')

char *msg1 = "hello, world #1";
char *msg2 = "hello, world #2";
char *msg3 = "hello, world #3";

/* Chạy khi con kết thúc (cha nhận SIGCHLD): thu dọn con để không thành zombie */
static void func(int signum)
{
    wait(NULL);
    printf("child process termination\n");
}

int main(int argc, char const *argv[])
{
    /* code */
    char in_buff[MSG_SIZE];     // chỗ chứa tin đọc từ pipe
    int num_read = 0;           // số byte read trả về
    int fds[2];                 // fds[0]: đầu đọc, fds[1]: đầu ghi
    pid_t child_pid;

    // Tạo pipe TRƯỚC fork để con kế thừa cả hai đầu
    if (pipe(fds) < 0) {
        printf("pipe() unsuccessfully\n");
        exit(1);
    }

    child_pid = fork();         // sau fork: cả cha và con đều có fds[0] và fds[1]
    if (child_pid >= 0) {
        if (child_pid == 0) {
            /* ===== CON: người ĐỌC ===== */
            printf("I am Child\n");

            // Đóng đầu ghi của con. Bắt buộc, nếu không read sẽ không bao giờ
            // trả về 0 vì con vẫn còn một đầu ghi mở
            if (close(fds[1]) == -1)
                printf("close(fds[1]) failed\n");

            while (1) {
                // Pipe rỗng mà còn đầu ghi mở thì read đứng chờ (block)
                num_read = read(fds[0], in_buff, MSG_SIZE);
                if (num_read == -1) {
                    printf("read() failed\n");
                    exit(0);
                } else if (num_read == 0) {
                    // Hết dữ liệu và mọi đầu ghi đã đóng
                    printf("pipe end-of-pipe\n");
                    break;
                } else {
                    printf("msg: %s\n", in_buff);
                }
            };

        } else {
            /* ===== CHA: người GHI ===== */
            // Khi con kết thúc sẽ có SIGCHLD, hàm func gọi wait để tránh zombie
            signal(SIGCHLD, func);

            printf("I am Parent\n");

            // Cha không đọc nên đóng đầu đọc
            if (close(fds[0]) == -1)
                printf("close(fds[0]) failed\n");

            // Ghi 3 tin vào pipe (vào trước ra trước)
            // Phải GHI TRƯỚC, đóng sau: đóng rồi mới write sẽ lỗi (EBADF)
            write(fds[1], msg1, MSG_SIZE);
            write(fds[1], msg2, MSG_SIZE);
            write(fds[1], msg3, MSG_SIZE);

            // Ghi xong thì đóng đầu ghi của cha. Dữ liệu đã ghi vẫn nằm trong pipe,
            // con đọc hết rồi mới nhận 0. (Chữ trong printf bên dưới ghi nhầm fds[0],
            // thực tế đang đóng fds[1])
            if (close(fds[1]) == -1)
                printf("close(fds[0]) failed\n");

            while(1);   // cha chạy mãi để chờ nhận SIGCHLD
        }
    } else {
        printf("fork() unsuccessfully\n");
    }

    return 0;
}