#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/* =========================================================================
 * OPTION 1: CODE GỐC (Dùng wait)
 * ========================================================================= */
void func_option1_wait(int signum)
{
    printf("\n>>> [Option 1] Nhan duoc SIGCHLD (%d)\n", signum);

    /* 
     * wait(NULL):
     * - Chỉ dọn được ĐÚNG 1 tiến trình con vừa kết thúc.
     * - Nhược điểm: Tín hiệu cùng loại không xếp hàng (queue). Nếu có nhiều con 
     *   chết cùng lúc, Kernel chỉ gửi duy nhất 1 tín hiệu SIGCHLD đến cha. 
     *   Lệnh wait() này chỉ dọn được 1 đứa con, những đứa con còn lại bị sót 
     *   và biến thành ZOMBIE vĩnh viễn.
     */
    wait(NULL);

    printf(">>> [Option 1] Da don 1 tien trinh con.\n\n");
}

/* =========================================================================
 * OPTION 2: NÂNG CẤP CHUẨN ĐI LÀM (Dùng waitpid kết hợp WNOHANG)
 * ========================================================================= */
void func_option2_waitpid(int signum)
{
    printf("\n>>> [Option 2] Nhan duoc SIGCHLD (%d)\n", signum);

    pid_t pid;

    /* 
     * =========================================================================
     * CÚ PHÁP GỐC: pid_t waitpid(pid_t pid, int *wstatus, int options);
     * =========================================================================
     * 
     * GIẢI THÍCH CHI TIẾT TỪNG THAM SỐ TRUYỀN VÀO:
     * 
     * 1. THAM SỐ 1: pid_t pid (Xác định muốn dọn tiến trình con nào)
     *    - pid > 0 (Số dương, ví dụ 1234): Chỉ chờ đích danh con có PID = 1234.
     *    - pid == -1: Chờ BẤT KỲ tiến trình con nào (tương đương hành vi của wait).
     *    - pid == 0: Chờ bất kỳ con nào có cùng Process Group ID (PGID) với cha.
     *    - pid < -1 (Số âm, ví dụ -1500): Chờ bất kỳ con nào thuộc nhóm PGID = |pid|.
     *    -> Chọn giá trị -1: Để gom bất kỳ đứa con nào đã chết.
     * 
     * 2. THAM SỐ 2: int *wstatus (Thông tin nguyên nhân kết thúc)
     *    - Truyền &status: Nhận mã trạng thái để phân tích qua macro (WIFEXITED, WTERMSIG,...).
     *    - Truyền NULL: Không cần lấy mã trạng thái, chỉ dọn xác khỏi bảng tiến trình.
     *    -> Chọn NULL: Mục đích chính là dọn dẹp sạch sẽ để tránh Zombie.
     * 
     * 3. THAM SỐ 3: int options (Cơ chế chờ)
     *    - options = 0 (Mặc định): Chặn luồng (blocking), treo cha đợi con chết.
     *    - options = WNOHANG (Wait No Hang): Không chặn (non-blocking). 
     *        + Nếu có con chết: Thu dọn và trả về ngay PID của con đó (> 0).
     *        + Nếu không có con chết: Lập tức trả về số 0, không làm đứng chương trình.
     *    - options = WUNTRACED: Báo cáo thêm nếu con bị tạm dừng (STOP).
     *    - options = WCONTINUED: Báo cáo thêm nếu con vừa được chạy lại (CONT).
     *    -> Chọn WNOHANG: Đảm bảo handler chạy xong là thoát ngay, cha không bị treo.
     * 
     * -------------------------------------------------------------------------
     * Ý NGHĨA GIÁ TRỊ TRẢ VỀ VÀ CƠ CHẾ VÒNG LẶP while ((pid = ...) > 0):
     * 
     * Các trường hợp trả về của waitpid(-1, NULL, WNOHANG):
     *   - Trả về > 0 : Dọn thành công 1 con, giá trị trả về là PID của con vừa dọn.
     *   - Trả về == 0: HẾT CON CHẾT (vẫn còn con khác đang sống, nhưng hiện tại 
     *                  không có đứa nào vừa chết để dọn).
     *   - Trả về == -1: HẾT CON ĐỂ CHẾT (tất cả các con đều đã dọn xong từ trước, 
     *                   hoặc gặp lỗi ECHILD).
     * 
     * Cơ chế quét sạch Zombie bằng vòng lặp:
     * - Khi nhiều con cùng kết thúc, chỉ có 1 tín hiệu SIGCHLD kích hoạt hàm.
     * - Vòng lặp while tiếp tục chạy chừng nào hàm còn trả về PID > 0:
     *     + Lần 1: Dọn con thứ 1 -> trả về PID con 1 (> 0) -> Lặp tiếp.
     *     + Lần 2: Dọn con thứ 2 -> trả về PID con 2 (> 0) -> Lặp tiếp.
     *     + Hết con chết: Trả về 0 (hoặc -1) -> Điều kiện (> 0) SAI -> Dừng lặp ngay.
     * =========================================================================
     */
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        printf(">>> [Option 2] Da don sach xac con co PID: %d\n", pid);
    }

    printf(">>> [Option 2] Hoan tat thu don.\n\n");
}

int main()
{
    /* fork() tạo tiến trình con: trả về 0 cho con, trả về PID con cho cha */
    pid_t child_pid = fork();
    
    if (child_pid >= 0) {
        if (child_pid == 0) {       /* --- TIẾN TRÌNH CON --- */
            printf("I am Child, myPID: %d\n", getpid());

            /* Con thực hiện công việc trong 3 giây rồi thoát */
            sleep(3);
            printf("Child lam xong viec va tu ket thuc!\n");
            
            /* Con thoát -> Kernel tự động bắn tín hiệu SIGCHLD (số 17) tới cha */
            exit(0);  
            
        } else {                    /* --- TIẾN TRÌNH CHA --- */
            /* 
             * ĐĂNG KÝ HÀM XỬ LÝ TÍN HIỆU (Signal Registration):
             * - Muốn dùng code gốc:    signal(SIGCHLD, func_option1_wait);
             * - Muốn dùng code chuẩn:  signal(SIGCHLD, func_option2_waitpid);
             */
            signal(SIGCHLD, func_option2_waitpid);
            
            printf("I am Parent, dang lam viec rieng...\n");
            
            /* Cha chạy liên tục việc của mình, không bị treo để chờ con */
            while (1) {
                printf("Parent van dang chay binh thuong...\n");
                sleep(1);
            }
        }
    } else {
        printf("fork() unsuccessfully\n");
    }

    return 0;
}