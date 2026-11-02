/* Thử 3 cách thread kết thúc: exit, cancel/join, detach */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

pthread_t thread_id1, thread_id2, thread_id3;

typedef struct {
    char name[30];
    char msg[30];
} thr_data_t;

/* Thread 1: tự kết thúc bằng pthread_exit */
static void *thr_handle1(void *args)
{
    thr_data_t *thr = (thr_data_t *)args;   /* ép void * về kiểu thật */
    sleep(1);
    printf("hello %s !\n", thr->name);
    printf("thread1 handler\n");

    pthread_exit(NULL); // exit              /* tự dừng, không trả giá trị */

}

/* Thread 2: chạy vô hạn, chỉ dừng khi bị cancel */
static void *thr_handle2(void *args)
{
    // sleep(5);
    // pthread_exit(NULL); // exit
    // exit(1);                             /* exit() giết cả process */
    while (1) {
        printf("thread2 handler\n"); 
        sleep(1);                           /* cancellation point */
    };
}

/* Thread 3: tự detach, kết thúc xong hệ thống tự dọn */
static void *thr_handle3(void *args)
{
    // pthread_detach(pthread_self());
    //sleep(2);
    pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{
    /* code */
    int ret, counter = 0;                   /* counter: tổng số thread 3 đã tạo */
    int retval;
    thr_data_t data = {0};

    strncpy(data.name, "sigma", sizeof(data.name));                 
    strncpy(data.msg, "learning thread programming\n", sizeof(data.msg));

    /* ret khác 0 = tạo thread lỗi */
    if (ret = pthread_create(&thread_id1, NULL, &thr_handle1, &data)) {
        printf("pthread_create() error number=%d\n", ret);
        return -1;
    }

    if (ret = pthread_create(&thread_id2, NULL, &thr_handle2, NULL)) {
        printf("pthread_create() error number=%d\n", ret);
        return -1;
    }
    sleep(2);

    // pthread_cancel(thread_id2);          /* cancel: gửi yêu cầu dừng, không chờ */
    // pthread_join(thread_id2, NULL);         /* join: đợi thread 2 dừng hẳn */
                                            /* ! cancel đang comment -> đợi mãi,
                                                 không tới được vòng while */
    // printf("thread_id2 termination\n"); 

    /* Tạo thread 3 liên tục để thử detach */
    while (1) {
        if (ret = pthread_create(&thread_id3, NULL, &thr_handle3, NULL)) {
            printf("pthread_create() error number=%d\n", ret);
            break;                          /* hết tài nguyên thì thoát */
        }
        counter++;
        //pthread_join(thread_id3, NULL);   /* dùng join thì không detach nữa */

        /*
        Nhược điểm của pthread_join là nó BLOCK (chặn) main: phải đợi thread
        kết thúc xong mới chạy tiếp. Join ngay sau mỗi lần tạo thì các thread
        chạy lần lượt từng cái một, mất tính song song và chạy chậm. Thread
        không bao giờ kết thúc thì main đợi mãi. Mỗi thread cũng phải join
        đúng 1 lần.
        => Nếu không cần lấy giá trị trả về hay chờ thread xong, nên dùng
        pthread_detach để hệ thống tự dọn thay vì join.
        */
        if (counter%1000 == 0) {
            printf("Thread created: %d\n", counter);
            // sleep(1);

        }
    }
    printf("Thread created: %d\n", counter);
    sleep(5);

    return 0;
}