#include <sys/un.h>
#include <sys/socket.h>
#include <stddef.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>     // memset, strncpy
#include <unistd.h>     // read, write, close, unlink

#define BUF_SIZE 10
#define SOCK_PATH "./sock_stream"

int main(void)
{
    struct sockaddr_un svaddr;
    int fd, datafd;
    socklen_t len;
    char buf[BUF_SIZE] = {0};

    // Xóa file socket cũ nếu còn tồn tại để không bị lỗi bind()
    unlink(SOCK_PATH);

/*
     * fd (File Descriptor): Số nguyên do Kernel cấp, đóng vai trò như ID đại diện
     * cho tài nguyên socket. Mọi hàm hệ thống (bind, listen, accept, read, write,
     * close) đều bắt buộc nhận fd làm tham số để Kernel biết chính xác tiến trình
     * đang muốn thao tác gửi, nhận hay đóng trên kênh kết nối nào.
     */
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    printf("fd is %d\n", fd);
    if (fd == -1) 
        return 1;

    memset(&svaddr, 0, sizeof(struct sockaddr_un)); 
    svaddr.sun_family = AF_UNIX;
    strncpy(svaddr.sun_path, SOCK_PATH, sizeof(svaddr.sun_path) - 1);

    if (bind(fd, (struct sockaddr *) &svaddr, sizeof(struct sockaddr_un)) == -1) {
        close(fd);
        return 1;
    }
    
    /*
     * listen() chuyển socket sang trạng thái lắng nghe để đón kết nối.
     * Tham số thứ hai (backlog = 5) là số kết nối tối đa được xếp hàng chờ.
     * Nếu vượt quá 5 kết nối đang chờ xử lý, các kết nối mới sẽ bị từ chối
     * cho đến khi server gọi accept() để lấy bớt kết nối ra khỏi hàng đợi.
     */
    if (listen(fd, 5) == -1) {
        close(fd);
        return 1; 
    } else {
        printf("start listening on server\n");
    }
    
    // Gán kích thước cho len trước khi đưa vào accept
    len = sizeof(struct sockaddr_un);
    datafd = accept(fd, (struct sockaddr*)&svaddr, &len);
    if (datafd == -1) {
        close(fd);
        return 1;
    } else {
        printf("accept connect\n");
    }
    
    while (1) {
        memset(buf, 0, BUF_SIZE);

        /*
         * Mặc định socket ở chế độ Blocking:
         * read() sẽ treo (block) tiến trình đợi cho đến khi có sự kiện:
         *   > 0 : Đọc thành công số byte dữ liệu client gửi tới.
         *   = 0 : Client đã đóng kết nối (gọi close() hoặc tắt tiến trình).
         *   < 0 : Xảy ra lỗi hệ thống (trả về -1).
         */
        if (read(datafd, buf, BUF_SIZE) > 0) {
            for (int j = 0; j < BUF_SIZE; j++) 
                buf[j] = toupper((unsigned char) buf[j]);
            printf("start response %s\n", buf);
            write(datafd, buf, BUF_SIZE - 1);
        } else {
            // Khi read() <= 0: client ngắt kết nối (= 0) hoặc bị lỗi (-1), thoát vòng lặp
            break;
        }
    }

    close(datafd);
    close(fd);
    unlink(SOCK_PATH); // Dọn dẹp file socket khi tắt server
    return 0;
}