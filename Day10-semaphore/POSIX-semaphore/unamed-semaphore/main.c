#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>

#define SEM_WAITING_TIMEOUT_SEC  3   // Chờ tối đa 3 giây

// Cấu trúc gom DỮ LIỆU THỰC TẾ và KHÓA BẢO VỆ vào chung một vùng nhớ
struct SharedData {
    char message[64];  // Dữ liệu trao đổi thực tế giữa 2 tiến trình
    sem_t sem;         // Unnamed semaphore dùng để đồng bộ
};

int main()
{
    // =========================================================================
    // 1. CẤP PHÁT BỘ NHỚ CHIA SẺ (SHARED MEMORY) BẰNG MMAP
    // =========================================================================
    /*
- VẤN ĐỀ VỀ BỘ NHỚ VÀ TIẾN TRÌNH:
- Khi gọi fork(), Linux nhân bản không gian địa chỉ ảo và dùng cơ chế Copy-on-Write (CoW).
- Nếu bạn dùng biến thường (sem_t sem) hoặc malloc(), Cha và Con sẽ có 2 bản sao độc lập.
-   -> Cha sem_post() thì chỉ tăng bản sao của Cha, Con đang đợi ở bản sao của Con sẽ bị treo mãi mãi.
-   -> Tương tự, Con ghi dữ liệu vào biến thường thì Cha cũng không bao giờ nhìn thấy.
- 
- GIẢI PHÁP VỚI MMAP (shared memory - posix):
- Yêu cầu Kernel cấp phát 1 vùng nhớ vật lý duy nhất trên RAM.
- Khi fork() xảy ra, cả Cha và Con đều cùng trỏ vào ĐÚNG ô nhớ vật lý này (không bị sao chép riêng).
- 
- CHI TIẾT TỪNG THAM SỐ:
- NULL: Để Kernel tự chọn địa chỉ ảo thích hợp trên không gian nhớ.
- sizeof(struct SharedData): Kích thước vùng nhớ, đủ chứa cả chuỗi message và struct semaphore.
- PROT_READ | PROT_WRITE: Cấp quyền đọc và ghi vào ô nhớ này.
- MAP_SHARED: Mọi sửa đổi trên vùng nhớ đều được chia sẻ tức thì giữa các tiến trình.
- MAP_ANONYMOUS: Bộ nhớ ẩn danh nằm trên RAM, không liên kết với file nào trên ổ cứng.
- -1, 0: File descriptor (-1) và Offset (0), bắt buộc để như vậy khi dùng MAP_ANONYMOUS.
     */
    struct SharedData *shm = mmap(NULL, sizeof(struct SharedData), 
                                  PROT_READ | PROT_WRITE, 
                                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (shm == MAP_FAILED) {
        perror("mmap thất bại");
        return -1;
    }

    // =========================================================================
    // 2. KHỞI TẠO UNNAMED SEMAPHORE (sem_init)
    // =========================================================================
    /*
- &(shm->sem): Con trỏ trỏ tới biến semaphore nằm ngay trong vùng Shared Memory vừa tạo.
- pshared = 1: Bật cờ chia sẻ giữa CÁC TIẾN TRÌNH (Process).
-   (Nếu pshared = 0: chỉ dùng cho các Luồng/Thread trong cùng 1 tiến trình).
- value = 1: Giá trị ban đầu của bộ đếm (Binary Semaphore: 1 lượt truy cập).
     */
    if (sem_init(&(shm->sem), 1, 1) == -1) {
        perror("sem_init thất bại");
        munmap(shm, sizeof(struct SharedData));
        return -1;
    }

    // =========================================================================
    // 3. TẠO TIẾN TRÌNH CON (FORK)
    // =========================================================================
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork thất bại");
        sem_destroy(&(shm->sem));
        munmap(shm, sizeof(struct SharedData));
        return -1;
    }

    if (pid == 0) {
        // ---------------- TIẾN TRÌNH CON (CHILD) ----------------
        printf("[Child %d] Bắt đầu xin khóa...\n", getpid());

        // Lấy thời gian thực hiện tại và thiết lập mốc hẹn giờ tuyệt đối (+3s)
        struct timespec timeout;
        if (clock_gettime(CLOCK_REALTIME, &timeout) == -1) {
            perror("[Child] Lỗi lấy thời gian");
            exit(EXIT_FAILURE);
        }
        timeout.tv_sec += SEM_WAITING_TIMEOUT_SEC;

        // Xin khóa có timeout (giảm value đi 1)
        if (sem_timedwait(&(shm->sem), &timeout) == -1) {
            perror("[Child] Không lấy được khóa (Timeout)");
            exit(EXIT_FAILURE);
        }

        // --- Vùng giữ khóa (Critical Section) ---
        printf("[Child %d] ĐÃ LẤY ĐƯỢC KHÓA! Đang ghi dữ liệu...\n", getpid());
        
        // Ghi dữ liệu thực tế vào ô nhớ dùng chung
        strcpy(shm->message, "Hello tu tien trinh Con!");
        sleep(2); // Giữ khóa 2 giây để chứng minh Cha phải đứng chờ

        // Nhả khóa (tăng value lên 1 để nhường quyền)
        printf("[Child %d] Nhả khóa cho tiến trình khác.\n", getpid());
        sem_post(&(shm->sem));

        exit(EXIT_SUCCESS);
    } 
    else {
        // ---------------- TIẾN TRÌNH CHA (PARENT) ----------------
        // Cha ngủ 1 giây để Con có thời gian vào chiếm khóa trước
        sleep(1); 

        printf("[Parent %d] Đang cố xin khóa bằng sem_timedwait (chờ tối đa 3s)...\n", getpid());

        struct timespec timeout;
        if (clock_gettime(CLOCK_REALTIME, &timeout) == -1) {
            perror("[Parent] Lỗi lấy thời gian");
        } else {
            timeout.tv_sec += SEM_WAITING_TIMEOUT_SEC;

            // Chờ đến khi Con gọi sem_post() hoặc hết 3 giây
            if (sem_timedwait(&(shm->sem), &timeout) == -1) {
                if (errno == ETIMEDOUT) {
                    printf("[Parent %d] Hết thời gian chờ (ETIMEDOUT)!\n", getpid());
                } else {
                    perror("[Parent] sem_timedwait lỗi");
                }
            } else {
                // --- Vùng giữ khóa của Cha ---
                printf("[Parent %d] ĐÃ LẤY ĐƯỢC KHÓA (sau khi Con nhả)!\n", getpid());
                
                // Đọc dữ liệu Con đã ghi từ Shared Memory ra màn hình
                printf("[Parent %d] Dữ liệu đọc được từ RAM chung: \"%s\"\n", getpid(), shm->message);
                
                sem_post(&(shm->sem)); // Dùng xong thì nhả khóa
            }
        }

        // Chờ Con kết thúc hoàn toàn trước khi thu hồi tài nguyên
        wait(NULL);

        // =========================================================================
        // 4. HỦY SEMAPHORE VÀ GIẢI PHÓNG BỘ NHỚ SHARED MEMORY
        // =========================================================================
        /*
- Unnamed semaphore không tạo file ảo trong /dev/shm nên:
- KHÔNG dùng: sem_close() hay sem_unlink().
- BẮT BUỘC dùng: sem_destroy() để giải phóng cấu trúc semaphore,
-   sau đó munmap() để trả vùng RAM lại cho hệ điều hành.
         */
        sem_destroy(&(shm->sem));
        munmap(shm, sizeof(struct SharedData));
        printf("[Parent %d] Đã hủy semaphore và giải phóng bộ nhớ mmap thành công.\n", getpid());
    }

    return 0;
}