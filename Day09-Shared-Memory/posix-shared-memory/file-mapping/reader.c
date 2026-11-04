#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

/* ============================================================================
 * [NOTE THỰC NGHIỆM KIỂM CHỨNG CƠ CHẾ COPY-ON-WRITE (CoW) VỚI mmap]
 *
 * MỤC ĐÍCH: 
 *   Chứng minh sự khác nhau giữa MAP_SHARED và MAP_PRIVATE.
 *
 * CÁC BƯỚC THỰC HIỆN:
 *   Bước 1: Để writer dùng MAP_SHARED, ghi chuỗi A vào memory rồi chạy writer.
 *           -> Dữ liệu A được ghi thẳng vào shared memory (/dev/shm/my_mmap).
 *
 *   Bước 2: Chạy reader (nhưng comment dòng shm_unlink lại để KHÔNG xóa file).
 *           -> reader đọc được chuỗi A thành công. File gốc vẫn còn trên RAM.
 *
 *   Bước 3: Đổi cờ ở writer sang MAP_PRIVATE, sửa code ghi sang chuỗi B mới, 
 *           chạy lại writer.
 *           -> writer in ra chuỗi B trên terminal của nó.
 *
 *   Bước 4: Chạy lại reader.
 *           -> KẾT QUẢ: reader VẪN ĐỌC ĐƯỢC CHUỖI A CŨ, hoàn toàn không thấy chuỗi B!
 *
 * BẢN CHẤT:
 *   - Với MAP_SHARED: Mọi thay đổi được ghi thẳng vào kernel shared object.
 *   - Với MAP_PRIVATE: Khi writer cố tình ghi đè (write attempt), cơ chế CoW 
 *     lập tức sao chép ra trang nhớ riêng cho writer. Writer chỉ sửa trên bản 
 *     sao tạm đó, file gốc /dev/shm/my_mmap không hề bị thay đổi.
 * ============================================================================
 */

#define SHARED_MEM_SIZE     100             /* the size (in bytes) of shared memory object */
#define FILE_NAME           "my_mmap"     /* name of the shared memory object */

/* 4 bước: 1. Tạo fd  2. Set size  3. map/unmap  4. Xóa shared memory segment */

int main() 
{ 
    // shm_open: mở object "my_mmap" đã được writer tạo (không có O_CREAT)
    int fd = shm_open(FILE_NAME, O_RDWR, 0);
    if (fd < 0) {
        printf("shm_open() is failed, %s.\n", strerror(errno));
        return -1;
    }

    // ftruncate: đặt kích thước 100 byte
    ftruncate(fd, SHARED_MEM_SIZE);

    // mmap: ánh xạ object vào vùng nhớ của process, trả về con trỏ
    char *data = (char *)mmap(0, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

    // đọc dữ liệu từ vùng nhớ
    printf("%s: Read data: %s\n", __FILE__, data);

    // munmap: gỡ ánh xạ khỏi process
    // munmap(data, SHARED_MEM_SIZE);

    // close: đóng fd
    close(fd);

    // shm_unlink(FILE_NAME);
  
    return 0; 
}

/*
 * ===== CHI TIẾT CÁC HÀM =====
 *
 * shm_open, ftruncate, mmap, munmap: giống hệt phần chi tiết ở writer.c bên trên.
 *   (ở reader không có O_CREAT, nên object phải do writer tạo trước.)
 *
 * close: int close(int fd)
 *   Đóng fd. Sau munmap vẫn nên close fd.
 *
 * shm_unlink: int shm_unlink(const char *name)
 *   Xóa shared memory object đã tạo. name: tên (đường dẫn) object.
 *   Object chỉ bị xóa thật sau khi process cuối cùng đã unmap.
 *   Nếu không gọi thì file /dev/shm/my_mmap vẫn còn tới khi reboot.
 */