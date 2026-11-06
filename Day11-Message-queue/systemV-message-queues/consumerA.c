#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>

//cunsomer
// structure for message queue
struct mesg_buffer {
    long mesg_type;        // loại message (message type), phải > 0
    char mesg_text[100];   // nội dung message (payload)
} message;

int main()
{
    key_t key;
    int msgid;

    // ftok to generate unique key
    key = ftok("producer.c", 65);    // ftok: tạo key từ đường dẫn file bất kì + project id

    // msgget creates a message queue
    // and returns identifier
    msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {    // msgget lỗi (ví dụ queue đã tồn tại do IPC_EXCL)
        perror("msgget");    // perror: in tên hàm + lý do lỗi (từ errno)
        return 1;
    }
    /*
    IPC_CREAT: chưa có queue với key này thì tạo mới
    IPC_EXCL: đi kèm IPC_CREAT, queue đã tồn tại thì msgget thất bại (trả -1)
    IPC_RMID: xóa queue, mọi message trong queue bị hủy
    IPC_STAT: lấy thông tin trạng thái của queue
    */

    // msgrcv to receive message
    // 1: chỉ nhận message type 1; 0: blocking (chờ đến khi có message)
    if (msgrcv(msgid, &message, sizeof(message), 1, 0) == -1) {    // msgrcv lỗi trả -1
        perror("msgrcv");
        return 1;
    }

    // display the message
    printf("Data Received is : %s \n",message.mesg_text);

    // to destroy the message queue
    msgctl(msgid, IPC_RMID, NULL);    // IPC_RMID: xóa queue khỏi hệ thống
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