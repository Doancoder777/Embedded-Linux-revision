#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>     // write, read, pipe, sleep
#include <signal.h>

#define MSG_SIZE    20  // mỗi tin nhắn đọc/ghi đúng 20 byte

char *msg1 = "hello, world #1";
char *msg2 = "hello, world #2";
char *msg3 = "hello, world #3";
char *msg4 = "hello, world #4";

char pipe_buff[MSG_SIZE];   // chỗ chứa dữ liệu đọc ra từ pipe
int fds[2];                 // fds[0]: đầu đọc, fds[1]: đầu ghi
                            // phải toàn cục để handler dùng được fds[1]

/* Chạy khi nhận SIGINT (Ctrl+C): ghi msg4 vào pipe */
void sig_handler1(int num)
{
    printf("Im signal handler1: %d\n", num);     // num = 2 (số hiệu của SIGINT)
    write(fds[1], msg4, MSG_SIZE);               // lưu ý: chuỗi chỉ 16 byte mà ghi 20,
                                                 // 4 byte cuối là dữ liệu rác
}

int main(int argc, char const *argv[])
{
    /* code */

    // Đăng ký handler cho SIGINT
    if (signal(SIGINT, sig_handler1) == SIG_ERR) {
        fprintf(stderr, "Cannot handle SIGINT\n");
        exit(EXIT_FAILURE);
    }

    // Tạo pipe: fds[0] đọc, fds[1] ghi. Trả -1 nếu lỗi
    if (pipe(fds) < 0) {
        printf("pipe() unsuccessfully\n");
        exit(1);
    }

    // Ghi 3 tin vào pipe: pipe lúc này chứa msg1, msg2, msg3 (vào trước ra trước)
    write(fds[1], msg1, MSG_SIZE);
    write(fds[1], msg2, MSG_SIZE);
    write(fds[1], msg3, MSG_SIZE); 

    /*
    khi comment 3 dong write thì pipe rỗng, read() sẽ block đứng chờ dữ liệu mới.
    */
    printf("sleep 2 seconds\n");
    sleep(2);

    // Đọc tin đầu tiên (msg1), số 1 viết cứng nên nhãn là msg[1]
    read(fds[0], pipe_buff, MSG_SIZE);
    printf("msg[%d]: %s\n\n", 1, pipe_buff);

    // Vòng for đọc tiếp 3 lần: msg2, msg3, rồi msg4
    // Lần thứ 3 pipe đã RỖNG nên read đứng chờ.
    // Bấm Ctrl+C -> handler ghi msg4 vào pipe -> read lấy được msg4 rồi chạy tiếp
    for (int i = 0; i < 3; i++) {
        read(fds[0], pipe_buff, MSG_SIZE);
        printf("msg[%d]: %s\n", i+1, pipe_buff);    // nhãn i+1 chỉ là bộ đếm,
                                                    // không khớp số trong nội dung
    }

    return 0;
}