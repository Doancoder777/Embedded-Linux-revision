#include <stdlib.h>     // standard stuff
#include <sys/mman.h>   // mmap()
#include <stdio.h>      // io stuff
#include <unistd.h>     // sleep()
#include <semaphore.h>  // semaphore()
#include <time.h>       // time()

#define BUFFER_SIZE 10  // số chỗ của kho (buffer)

typedef struct Buffer {
    char **Tuples;
    int inSlotIndex;    // vị trí producer sẽ bỏ món vào (in)
    int outSlotIndex;   // vị trí consumer sẽ lấy món ra (out)
} Buffer;

int main()
{
    /* Create shared memory */
    // mmap: MAP_SHARED | MAP_ANONYMOUS = vùng nhớ chung KHÔNG gắn file, chỉ chia sẻ cha - con qua fork (fd = -1)
    int *buffer = (char*)mmap(NULL, sizeof(int)*BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    /* Create shared semaphores */
    // semaphore phải nằm trong vùng nhớ chung thì các process con mới dùng chung được
    sem_t *mutex = (sem_t*)mmap(NULL, sizeof(sem_t*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_t *full = (sem_t*)mmap(NULL, sizeof(sem_t*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_t *empty = (sem_t*)mmap(NULL, sizeof(sem_t*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    // sem_init(sem, pshared, value): pshared = 1 -> dùng giữa các process (0 = chỉ giữa các thread)
    sem_init(mutex, 1, 1);              // binary semaphore: "chìa khóa kho", 1 người vào tại một thời điểm
    sem_init(empty, 1, BUFFER_SIZE);    // số CHỖ TRỐNG: bắt đầu 10 (kho rỗng)
    sem_init(full, 1, 0);               // số MÓN ĐANG CÓ: bắt đầu 0

    /* Forking */
    pid_t producer;
    pid_t consumer;

    /* Child producer process */
    // fork() trả về 0 trong process con -> đây là nhánh của producer
    if ((producer = fork()) == 0)
    {
        while (1)
        {
            sem_wait(empty);            // xin 1 chỗ trống (kho đầy thì chờ ở đây)
            sem_wait(mutex);            // lấy chìa khóa vào kho (wait(empty) trước, wait(mutex) sau để tránh deadlock)
            /* YOUR TASK: IMPLEMENT IN/OUT POINTERS TO ADD/TAKE FROM THE BUFFERS IN FIFO FASHION */
            printf("Producer creates something\n");
            sem_post(full);             // báo có thêm 1 món (full +1)
            sem_post(mutex);            // trả chìa khóa

            /* Sleep between 0 and 3 seconds */
            srand(time(NULL));
            sleep(rand() % 5);          // nghỉ ngẫu nhiên 0-4 giây
        }
    }

    /* Child consumer process */
    // fork lần 2: tới đây process cha tạo thêm con consumer
    if ((consumer = fork()) == 0)
    {
        while (1)
        {
            sem_wait(full);             // xin 1 món (kho rỗng thì chờ ở đây)
            sem_wait(mutex);            // lấy chìa khóa vào kho
            /* YOUR TASK: IMPLEMENT IN/OUT POINTERS TO ADD/TAKE FROM THE BUFFERS IN FIFO FASHION */
            printf("Consumer takes something\n");

            sem_post(mutex);            // trả chìa khóa
            sem_post(empty);            // báo có thêm 1 chỗ trống (empty +1)

            /* Sleep between 3 and 8 seconds */
            srand(time(NULL));
            sleep(3 + rand() % 5);      // nghỉ ngẫu nhiên 3-7 giây
        }
    }
    /* Parent */
    else
    {
        while (1)
        {
            sleep(10);
            int takenSlots;
            sem_getvalue(full, &takenSlots);    // đọc giá trị hiện tại của full = số món đang có trong kho
            printf("Items in the buffer: %d/%d\n", takenSlots, BUFFER_SIZE);
        }
    }
}

/*
 * ===== TÓM TẮT BÀI TOÁN PRODUCER - CONSUMER (bounded buffer) =====
 *
 * Producer sinh dữ liệu bỏ vào kho (tối đa BUFFER_SIZE chỗ);
 * Consumer lấy dữ liệu ra. Kho đầy -> producer chờ; kho rỗng -> consumer chờ.
 *
 * 3 semaphore:
 *   empty = số chỗ trống   (khởi tạo BUFFER_SIZE, producer wait, consumer post)
 *   full  = số món đang có (khởi tạo 0,           consumer wait, producer post)
 *   mutex = chìa khóa kho  (khởi tạo 1, binary semaphore, ai vào kho cũng wait/post)
 *
 * Producer chạy: empty đếm ngược (10 -> 0), full đếm lên (0 -> 10).
 * Consumer chạy: ngược lại. Luôn có empty + full = BUFFER_SIZE.
 *   empty = 0 -> kho đầy, producer bị chặn.   full = 0 -> kho rỗng, consumer bị chặn.
 *
 * Quy tắc thứ tự: wait(empty / full) TRƯỚC, wait(mutex) SAU
 *   (giữ chìa khóa rồi mới chờ chỗ trống/món thì hai bên chờ nhau mãi = deadlock).
 *
 * Wait/post dùng chung vùng nhớ qua mmap(MAP_SHARED | MAP_ANONYMOUS) và fork:
 * cha tạo vùng nhớ trước, các process con kế thừa.
 */

/*
 * ===== TÓM TẮT KIỂU FEYNMAN (giải thích như cho người chưa biết gì) =====
 *
 * Tưởng tượng một quán bánh: bếp (producer) làm bánh đặt lên cái KỆ có 10 chỗ,
 * người bán (consumer) lấy bánh từ kệ ra bán cho khách.
 *
 * Quán chỉ cần nhớ 3 thứ, mỗi thứ là một semaphore (một con số):
 *   empty = "còn mấy chỗ trống trên kệ"    (đầu ngày: 10)
 *   full  = "đang có mấy cái bánh trên kệ" (đầu ngày: 0)
 *   mutex = "chìa khóa quầy kệ"            (chỉ 1 chiếc, ai cầm mới được đụng vào kệ)
 *
 * Bếp muốn đặt bánh:
 *   1. Phải có chỗ trống (empty giảm 1). Hết chỗ thì ngồi chờ, không làm gì cả.
 *   2. Cầm chìa khóa, đặt bánh lên kệ, trả chìa khóa.
 *   3. Báo "có thêm 1 bánh" (full tăng 1).
 *
 * Người bán muốn lấy bánh: làm ngược lại.
 *   1. Phải có bánh (full giảm 1). Hết bánh thì ngồi chờ.
 *   2. Cầm chìa khóa, lấy bánh, trả chìa khóa.
 *   3. Báo "có thêm 1 chỗ trống" (empty tăng 1).
 *
 * Vì sao cần chìa khóa? Để hai người không cùng thò tay vào cùng một chỗ trên kệ.
 * Vì sao phải xin chỗ/bánh TRƯỚC rồi mới cầm chìa khóa? Nếu cầm chìa khóa mà kệ
 * đã đầy, bếp đứng chờ chỗ trống trong khi cầm khóa, người bán không có khóa để
 * lấy bánh ra, hai bên chờ nhau mãi (deadlock).
 *
 * Một câu để nhớ: semaphore là cái bảng đếm "còn bao nhiêu"; hết thì chờ,
 * có thêm thì báo cho bên kia biết.
 */