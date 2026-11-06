#include <stdio.h>
#include <string.h>
#include <mqueue.h>
#include <errno.h>

#define MQ_MODE (S_IRUSR | S_IWUSR) // MQ_MODE: quyền truy cập (mode); S_IRUSR | S_IWUSR = chủ sở hữu được đọc/ghi (0600)
int main(int argc, char *argv[])
{
    struct mq_attr attr, attrp; // mq_attr: struct thuộc tính queue; attr để đặt, attrp để đọc lại

    attr.mq_maxmsg = 5;
    attr.mq_msgsize = 2048;
    printf("Create mqueu\n");
    mqd_t mqid = mq_open("/mqueue", O_RDWR | O_CREAT | O_NONBLOCK, MQ_MODE, NULL); // mq_open: tạo/mở queue; tên phải bắt đầu bằng "/"; O_NONBLOCK: không chờ; NULL: attr mặc định
    if (mqid == -1)
    {
        printf("mq_open() error %d: %s\n", errno, strerror(errno)); // strerror: đổi mã lỗi errno thành chuỗi mô tả
        return -2;
    }

    if (mq_getattr(mqid, &attrp) != 0)
    { // mq_getattr: đọc thuộc tính hiện tại của queue vào attrp
        printf("mq_open() error %d: %s\n", errno, strerror(errno));
        return -3;
    }

    if (attrp.mq_flags == 0) // mq_flags: 0 là blocking, khác 0 là O_NONBLOCK
        printf("mq_flags = 0\n");
    else
        printf("mq_flags = O_NONBLOCK\n");

    printf("mq_maxmsg = %ld,\n", attrp.mq_maxmsg);  // mq_maxmsg: số message tối đa trong queue
    printf("mq_msgsize = %ld\n", attrp.mq_msgsize); // mq_msgsize: kích thước tối đa 1 message (byte)
    printf("mq_curmsgs = %ld\n", attrp.mq_curmsgs); // mq_curmsgs: số message hiện có trong queue

    mq_unlink("/mqueue"); // mq_unlink: xóa queue khỏi hệ thống
    return 0;
}

/*
 *
 * Đã dùng trong file này:
 *   mq_open()    : tạo/mở queue, trả về mqd_t (message queue descriptor)
 *   mq_getattr() : đọc thuộc tính của queue (mq_attr)
 *   mq_unlink()  : xóa tên queue khỏi hệ thống
 *
 *   mq_send(mqid, msg, len, prio)                  : gửi message kèm priority
 *   mq_receive(mqid, buf, size, &prio)             : nhận message có priority cao nhất; buf >= mq_msgsize
 *   mq_close(mqid)                                 : đóng descriptor (không xóa queue)
 *   mq_setattr(mqid, &newattr, &oldattr)           : đổi thuộc tính (chỉ đổi được mq_flags, ví dụ bật/tắt O_NONBLOCK)
 *   mq_notify(mqid, &sigevent)                     : nhờ hệ thống báo (signal/thread) khi queue có message mới
 *   mq_timedsend(mqid, msg, len, prio, &abs_time)  : như mq_send nhưng chờ tối đa đến thời điểm abs_time (timeout)
 *   mq_timedreceive(mqid, buf, size, &prio, &abs_time) : như mq_receive nhưng có timeout
 */