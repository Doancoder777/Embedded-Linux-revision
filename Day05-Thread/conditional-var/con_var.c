/*
=====
LUỒNG pthread_cond_wait (CHỐT)
=====

1. main khóa mutex trước.

2. main gọi pthread_cond_wait: wait TỰ mở khóa (unlock) rồi đưa main đi ngủ.

3. Khóa đã được mở nên thread 1 khóa được mutex, bắt đầu đếm lên 5.

4. Đếm đủ thì thread 1 gọi pthread_cond_signal để báo cho main.
   Lúc này main đã biết, nhưng chưa khóa lại được vì thread 1 vẫn đang giữ khóa.

5. thread 1 chạy tới pthread_mutex_unlock, trả khóa.

6. main được khóa: wait giúp main KHÓA LẠI mutex rồi mới kết thúc wait.

7. main kiểm tra lại điều kiện (counter != THRESHOLD),
   đủ rồi thì thoát vòng lặp, in ra, rồi unlock.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define THRESHOLD   5

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;   /* khóa bảo vệ counter */
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;     /* dùng để báo "counter đã đủ" */
int counter; // critical section <=> global resource

typedef struct {
    char name[30];
    char msg[30];
} thread_args_t;

static void *handle_th1(void *args) 
{
    thread_args_t *thr = (thread_args_t *)args;

    pthread_mutex_lock(&lock);      /* bước 3 */
    printf("Hello %s !\n", thr->name);

    while (counter < THRESHOLD) {
        counter += 1;
        printf("Counter = %d\n", counter);
        sleep(1);
    }

    pthread_cond_signal(&cond);     /* bước 4 */
    printf("thread1 handler, counter = %d\n", counter);
    pthread_mutex_unlock(&lock);    /* bước 5 */

    pthread_exit(NULL); // exit or return;

}

int main(int argc, char const *argv[])
{
    /* code */
    int ret;
    thread_args_t thr;
    pthread_t thread_id1;

    memset(&thr, 0x0, sizeof(thread_args_t));
    strncpy(thr.name, "sigma", sizeof(thr.name));

    if (ret = pthread_create(&thread_id1, NULL, &handle_th1, &thr)) {
        printf("pthread_create() error number=%d\n", ret);
        return -1;
    }

    pthread_mutex_lock(&lock);              /* bước 1 */

    /*    
    Kiểm tra điều kiện trước, chưa đủ mới wait.
    trong code có thể bỏ vòng while, nhưng để an toàn thì nên có vòng while.
    tránh đánh bị bug thức giả khi đi làm
       Thread 1 chạy xong trước thì counter đã đủ, main không cần wait,
       nên signal tới sớm cũng không làm treo. */
    while (counter != THRESHOLD) {
        pthread_cond_wait(&cond, &lock);    /* bước 2 và bước 6 */
    }

    printf("Global variable counter = %d.\n", counter);    /* bước 7 */
    pthread_mutex_unlock(&lock);
    
    // used to block for the end of a thread and release
    pthread_join(thread_id1,NULL);          /* đợi thread 1 kết thúc hẳn */

    return 0;
}