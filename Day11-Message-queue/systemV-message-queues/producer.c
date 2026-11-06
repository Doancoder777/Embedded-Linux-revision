#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define BUFFER_SIZE 100
//producer.c
// structure for message queue
struct mesg_buffer {
    long mesg_type;                  // loại message (message type), phải > 0
    char mesg_text[BUFFER_SIZE];     // nội dung message (payload)
} message;

int main()
{
    key_t key;
    int msgid;

    // ftok to generate unique key
    key = ftok("producer.c", 65);    // ftok: tạo key từ đường dẫn file bất kỳ + project id; phải giống bên consumer

    // msgget creates a message queue
    // and returns identifier
    msgid = msgget(key, 0666 | IPC_CREAT);    // IPC_CREAT: chưa có queue thì tạo, có rồi thì mở

    printf("Enter type message: ");
    scanf("%ld", &message.mesg_type);    // %ld: đọc số kiểu long vào mesg_type
    stdin = freopen(NULL, "r", stdin);   // clear input buffer: mở lại stdin để bỏ '\n' còn sót sau scanf

    printf("Enter message: ");
    fgets(message.mesg_text, BUFFER_SIZE, stdin);    // fgets: đọc cả dòng, tối đa BUFFER_SIZE - 1 ký tự

    // msgsnd to send message
    msgsnd(msgid, &message, sizeof(message), 0);    // gửi message vào queue; 0: blocking (queue đầy thì chờ)
    return 0;
}



/* 
=====================================================================
NOTE: ftok VA KEY LOI -1 (Message Queue System V)
=====================================================================

ftok(pathname, proj_id)
  key = (số định danh của file: inode + device) + (project id)
  - Chuỗi pathname KHÔNG đi vào key, chỉ dùng để TÌM file.
  - Tên file đặt tùy ý, nhưng file PHẢI tồn tại thật.

CÁCH SAI (nhìn bề ngoài vẫn chạy):
  key = ftok("hello", 65);     // không có file "hello"
  - ftok lỗi, trả -1 (không báo gì, không dừng chương trình)
  - Code không kiểm tra, nên producer và consumer đều có key = -1
  - msgget(-1, ...) vẫn tạo/mở được queue, nên gửi nhận vẫn chạy
  - Chạy được là TÌNH CỜ. Mọi chương trình ftok lỗi khác cũng chui
    vào đúng queue này, message bị lẫn.

CÁCH ĐÚNG:
  1. Tạo file thật trước:  touch sigma
  2. Kiểm tra lỗi mọi hàm:

     key = ftok("sigma", 65);
     if (key == -1) {
         perror("ftok");      // in lý do lỗi (vd: No such file or directory)
         return 1;
     }

  3. Producer và mọi consumer: cùng pathname, cùng project id,
     chạy cùng thư mục.
  4. Muốn xóa queue cũ cho sạch:  ipcs -q  rồi  ipcrm -q <msqid>

IPC_EXCL: queue đã tồn tại thì msgget trả -1 (lỗi "File exists").


*/
