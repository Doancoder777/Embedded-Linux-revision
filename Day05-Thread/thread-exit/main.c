#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

pthread_t thread_id1, thread_id2;

typedef struct {
    char name[30];
    int number;
} thr_data_t;

/* Thread 1: tu ket thuc bang pthread_exit(), tra ve gia tri TUY CHINH */
static void *thr_exit_handle(void *args)
{
    thr_data_t *data = (thr_data_t *)args;

    printf("[thread 1] Hello %s, number = %d\n", data->name, data->number);
    sleep(1);

    /* Phai dung heap (malloc), khong dung bien cuc bo,
       vi stack cua thread bi thu hoi khi thread ket thuc */
    int *result = malloc(sizeof(int));
    *result = data->number * 2;

    printf("[thread 1] goi pthread_exit(), retval = %d\n", *result);
    pthread_exit(result);
}

/* Thread 2: chay vong lap vo han, cho main goi pthread_cancel() */
static void *thr_cancel_handle(void *args)
{
    (void)args;
    int count = 0;

    while (1) {
        printf("[thread 2] dang chay... %d\n", count++);
        sleep(1);   /* sleep() la cancellation point: cho phep thread bi huy */
    }

    return NULL;    /* khong bao gio toi day, nhung de het warning va dung kieu */
}

int main(int argc, char const *argv[])
{
    (void)argc;
    (void)argv;

    int ret;
    void *ketqua1, *ketqua2;
    thr_data_t data = {0};

    strncpy(data.name, "sigma", sizeof(data.name) - 1);
    data.number = 21;

    if ((ret = pthread_create(&thread_id1, NULL, &thr_exit_handle, &data)) != 0) {
        printf("pthread_create() error number=%d\n", ret);
        return -1;
    }

    if ((ret = pthread_create(&thread_id2, NULL, &thr_cancel_handle, NULL)) != 0) {
        printf("pthread_create() error number=%d\n", ret);
        return -1;
    }

    /* Cho thread 2 chay vai giay roi gui yeu cau huy */
    sleep(3);
    printf("[main] goi pthread_cancel(thread_id2)\n");
    pthread_cancel(thread_id2);

    /* Join thread 1: nhan retval tuy chinh */
    pthread_join(thread_id1, &ketqua1);
    printf("[main] thread 1 ket thuc, ketqua1 = %d\n", *(int *)ketqua1);
    free(ketqua1);

    /* Join thread 2: nhan PTHREAD_CANCELED do he thong dat */
    pthread_join(thread_id2, &ketqua2);
    if (ketqua2 == PTHREAD_CANCELED) {
        printf("[main] thread 2 bi cancel (ketqua2 == PTHREAD_CANCELED)\n");
    } else {
        printf("[main] thread 2 tu ket thuc\n");
    }

    return 0;
}
