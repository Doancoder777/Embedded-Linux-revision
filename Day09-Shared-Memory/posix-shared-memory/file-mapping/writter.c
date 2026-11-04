#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <sys/shm.h>

#define SHARED_MEM_SIZE     100             /* the size (in bytes) of shared memory object */
#define FILE_NAME           "my_mmap"     /* name of the shared memory object */

/* 4 bước: 1. Tạo fd  2. Set size  3. map/unmap  4. Xóa shared memory segment */

int main() 
{ 
    // shm_open: tạo (O_CREAT) và mở shared memory object tên "my_mmap", đọc/ghi
    // mode 0666 chỉ có tác dụng lúc TẠO MỚI; object đã có thì mode bị bỏ qua (quyền cũ giữ nguyên)
    int shm_fd = shm_open(FILE_NAME, O_CREAT | O_RDWR, 0666); 
    if (shm_fd < 0) {
        printf("shm_open() is failed, %s.\n", strerror(errno));
        return -1;
    }

    // ftruncate: đặt kích thước 100 byte (mới mở thì size = 0)
    ftruncate(shm_fd, SHARED_MEM_SIZE);
    
    // mmap: ánh xạ object vào vùng nhớ của process, trả về con trỏ
    char *data = (char *)mmap(0, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, shm_fd, 0);

    // ghi vào vùng nhớ như ghi vào mảng bình thường
    strcpy(data, "MMAP Doan Dep trai !");
    printf("%s: Write data: %s\n", __FILE__, data);

    // munmap: gỡ ánh xạ khỏi process
    munmap(data, SHARED_MEM_SIZE);

    close(shm_fd);
  
    return 0; 
}

/*
 * ===== CHI TIẾT CÁC HÀM =====
 *
 * shm_open: int shm_open(const char *name, int oflag, mode_t mode)
 *   name : tên (đường dẫn) của shared memory object.
 *   oflag:
 *     O_RDONLY: mở chỉ đọc.
 *     O_RDWR  : mở đọc/ghi.
 *     O_CREAT : chưa có object thì tạo mới; đã có thì mở cái đang có.
 *     O_EXCL  : nếu object đã tồn tại thì trả về lỗi EEXIST.
 *     O_TRUNC : nếu object đã tồn tại thì cắt (truncate) về 0.
 *   mode : quyền (permission) của object khi TẠO MỚI.
 *          Bị bỏ qua (ignored) nếu không có O_CREAT hoặc object đã tồn tại.
 *          "Bỏ qua" = lần gọi này không dùng mode; quyền của object KHÔNG mất,
 *          vẫn giữ giá trị đã gán lúc object được tạo lần đầu.
 *          (Vì vậy reader truyền mode = 0: object đã có, mode không còn tác dụng.)
 *   Trả về fd nếu OK, -1 nếu lỗi. Sau khi tạo, thấy file trong /dev/shm.
 *
 * ftruncate: int ftruncate(int shm_fd, off_t length)
 *   Đặt kích thước cho shared memory object (mới mở thì size = 0).
 *   shm_fd: fd của object. length: kích thước (byte).
 *
 * mmap: void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
 *   addr  : địa chỉ muốn map; 0 = kernel tự chọn.
 *   length: độ dài vùng map (> 0).
 *   prot  : quyền bảo vệ vùng nhớ (không được xung đột với quyền của file):
 *     PROT_EXEC : có thể thực thi.
 *     PROT_READ : có thể đọc.
 *     PROT_WRITE: có thể ghi.
 *     PROT_NONE : không được truy cập.
 *   flags : các process khác có thấy thay đổi hay không:
 *     MAP_SHARED : thay đổi được chia sẻ cho process khác.
 *     MAP_PRIVATE: dùng copy-on-write, thay đổi chỉ của riêng process,
 *                  process khác KHÔNG thấy.
 *     Dùng OR (|) để kết hợp thêm:
 *     MAP_ANONYMOUS    : map không gắn file, khởi tạo toàn 0, bỏ qua fd.
 *     MAP_UNINITIALIZED: không khởi tạo vùng anonymous.
 *   fd    : fd của shared memory object.
 *   offset: vị trí trong object bắt đầu map.
 *   Trả về con trỏ nếu OK, MAP_FAILED ((void *)-1) nếu lỗi (errno báo nguyên nhân).
 *
 * munmap: int munmap(void *addr, size_t length)
 *   Gỡ ánh xạ khỏi process. addr: con trỏ do mmap trả về. length: độ dài (> 0).
 *   Trả về 0 nếu OK, -1 nếu lỗi (thường là EINVAL).
 */