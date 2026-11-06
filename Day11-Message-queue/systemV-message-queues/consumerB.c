#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>

//consumerB.c
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
    key = ftok("producer.c", 65);   // ftok: tạo key từ  + project id; producer và consumer phải dùng cùng key
  
    // msgget creates a message queue
    // and returns identifier
    msgid = msgget(key, 0666 | IPC_CREAT);    // IPC_CREAT: chưa có queue thì tạo, có rồi thì mở (không có IPC_EXCL nên không lỗi)
  
    // msgrcv to receive message
    msgrcv(msgid, &message, sizeof(message), 2, 0);    // 2: chỉ nhận message type 2; 0: blocking (chờ đến khi có message)
  
    // display the message
    printf("Data Received is : %s \n", message.mesg_text);
  
    // to destroy the message queue
    // msgctl(msgid, IPC_RMID, NULL);    // IPC_RMID: xóa queue; đang comment lại để queue còn cho consumer khác
  
    return 0;
}