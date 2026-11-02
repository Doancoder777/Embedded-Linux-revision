#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void sig_handler1(int signum) {
    printf("Im signal handler1\n");
    exit(EXIT_SUCCESS);
}

/* In số hiệu các signal có trong tập, giống số dùng cho lệnh kill -<số>.
   sigset_t là struct nên phải hỏi từng signal bằng sigismember. */
void print_set(const char *name, const sigset_t *set) {
    printf("%s:", name);
    for (int i = 1; i < 32; i++) {
        if (sigismember(set, i) == 1) {
            printf(" %d", i);
        }
    }
    printf("\n");
}

int main() {
    //time_t start, finish;
    sigset_t new_set, old_set, all_set;
    if (signal(SIGINT, sig_handler1) == SIG_ERR) {
        fprintf(stderr, "Cannot handle SIGINT\n");
        exit(EXIT_FAILURE);
    }

    /* ===== 5 hàm thao tác tập signal (chỉ soạn tờ phiếu, chưa chặn gì) ===== */
    sigemptyset(&new_set);                  // làm tập rỗng
    sigemptyset(&old_set);

    sigaddset(&new_set, SIGINT);            // thêm SIGINT
    sigaddset(&new_set, SIGQUIT);           // thêm SIGQUIT
    print_set("new_set", &new_set);         // in: new_set: 2 3

    sigdelset(&new_set, SIGQUIT);           // bỏ SIGQUIT
    print_set("new_set", &new_set);         // in: new_set: 2

    sigfillset(&all_set);                   // chọn TẤT CẢ signal (chỉ để xem, không áp vào process)
    print_set("all_set", &all_set);         // in: all_set: 1 2 3 4 ... 31

    if (sigismember(&new_set, SIGINT) == 1) {       // hỏi signal có trong tập không
        printf("SIGINT exist\n");
    } else if (sigismember(&new_set, SIGINT) == 0) {
        printf("SIGINT does not exist\n");
    }

    /* ===== sigprocmask: áp tờ phiếu vào process ===== */

    /* SIG_SETMASK: thay hẳn mask process bằng new_set -> SIGINT bị chặn.
       old_set cất mask TRƯỚC khi đổi (đang trống) */
    sigprocmask(SIG_SETMASK, &new_set, &old_set);
    print_set("old_set", &old_set);         // in: old_set:   (trống)

    /* SIG_UNBLOCK: gỡ new_set khỏi mask -> SIGINT hết bị chặn */
    sigprocmask(SIG_UNBLOCK, &new_set, NULL);

    /* SIG_BLOCK: cộng new_set vào mask -> SIGINT bị chặn lại */
    sigprocmask(SIG_BLOCK, &new_set, NULL);

    /* Khôi phục mask ban đầu bằng old_set -> Ctrl+C chạy được lại */
    sigprocmask(SIG_SETMASK, &old_set, NULL);

    while (1) {
        sleep(1);
    }
    return 0;
}

/* =====================================================
   CHỤP MASK HIỆN TẠI CỦA PROCESS (để xem hoặc lưu lại)

   1. Chỉ xem: newset = NULL, hàm không đổi mask,
      chỉ chép mask hiện tại vào tham số cuối.

        sigset_t cur_set;
        sigprocmask(SIG_BLOCK, NULL, &cur_set);   // cur_set = mask hiện tại
        print_set("cur_set", &cur_set);

   2. Lưu để khôi phục sau: dùng oldset khi đổi mask.

        sigprocmask(SIG_BLOCK, &new_set, &old_set);   // old_set = mask TRƯỚC khi đổi
        ...
        sigprocmask(SIG_SETMASK, &old_set, NULL);     // khôi phục

   Phân biệt: cur_set = mask lúc này, old_set = mask lúc trước khi đổi.
   ===================================================== */