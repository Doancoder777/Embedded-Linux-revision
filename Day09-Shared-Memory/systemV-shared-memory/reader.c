#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_KEY 0x1234

int main()
{
    // ftok: phải cho ra cùng key với writer thì mới thấy cùng segment
    key_t key = ftok("./shmfile", 65);

    // shmget: mở segment của writer (IPC_CREAT: nếu chưa có thì tạo mới rỗng)
    int shmid = shmget(key, 1024, 0666 | IPC_CREAT);

    // shmat: gắn segment vào process, trả về con trỏ tới vùng nhớ chung
    char *shmaddr = (char*)shmat(shmid, (void*)0, 0);

    // đọc dữ liệu writer đã ghi
    printf("Data read from memory: %s\n", shmaddr);

    // shmdt: gỡ segment khỏi process này
    shmdt(shmaddr);

    // shmctl + IPC_RMID: đánh dấu xóa segment (xóa thật khi không còn process nào gắn)
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}

/*
 * ===== CHI TIẾT CÁC HÀM =====
 *
 * ftok: key_t ftok(const char *path, int id)
 *   Tạo key để các process tìm đúng segment. Cách tạo key:
 *     - Số tùy ý (ví dụ SHM_KEY 0x1234)
 *     - IPC_PRIVATE: cho process cha - con
 *     - ftok(path, id)
 *   path: đường dẫn file (phải tồn tại). id: project ID, chỉ dùng 8 bit thấp.
 *   Trả về key nếu OK, -1 nếu lỗi.
 *
 * shmget: int shmget(key_t key, size_t size, int shmflg)
 *   Tạo shared memory segment.
 *   size: làm tròn lên bội số của PAGE_SIZE.
 *   shmflg (kèm quyền 0666):
 *     IPC_CREAT: tạo mới; nếu key đã có segment thì dùng lại.
 *     IPC_EXCL : tạo mới; nếu đã có segment thì THẤT BẠI (dùng cùng IPC_CREAT).
 *   Trả về shmid nếu OK, -1 nếu lỗi.
 *
 * shmat: void *shmat(int shmid, const void *shmaddr, int shmflg)
 *   Gắn (attach) segment vào không gian địa chỉ của process.
 *   shmaddr: địa chỉ muốn gắn; 0 (NULL) = kernel tự chọn.
 *   shmflg (mặc định 0 = đọc và ghi):
 *     SHM_RDONLY: gắn chỉ để đọc.
 *     SHM_REMAP : thay vùng ánh xạ đang có, từ shmaddr tới hết segment.
 *     SHM_RND   : làm tròn shmaddr xuống bội số của SHMLBA.
 *   shmaddr khác 0 và không có SHM_RND: gắn đúng địa chỉ đó.
 *   Trả về con trỏ nếu OK, -1 nếu lỗi.
 *
 * shmdt: int shmdt(const void *shmaddr)
 *   Gỡ (detach) segment khỏi process; shmaddr do shmat trả về.
 *   KHÔNG xóa segment: shmid còn tồn tại cho tới khi có process gọi shmctl IPC_RMID.
 *   Trả về 0 nếu OK, -1 nếu lỗi.
 *
 * shmctl: int shmctl(int shmid, int cmd, struct shmid_ds *buf)
 *   Điều khiển segment.
 *   cmd:
 *     IPC_STAT: chép thông tin hiện tại của segment vào buf (cần quyền đọc).
 *     IPC_SET : đặt user ID, group ID của chủ sở hữu, quyền... theo buf.
 *     IPC_RMID: đánh dấu xóa; chỉ xóa thật khi process cuối cùng đã detach.
 *     IPC_INFO: trả về thông tin giới hạn và tham số của shared memory.
 *     SHM_INFO: trả về struct shm_info (tài nguyên hệ thống shm đang dùng).
 *   buf: con trỏ tới struct shmid_ds, dùng để lấy (get) hoặc đặt (set) tùy cmd.
 *        Với IPC_RMID truyền NULL.
 */