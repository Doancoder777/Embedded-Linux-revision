#include <sys/un.h>
#include <sys/socket.h>
#include <stddef.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 10
#define SOCK_PATH "./sock_dgram"

int main(void)
{
    // svaddr: Lưu địa chỉ file socket của server
    // claddr: Dùng để nhận địa chỉ file socket của client gửi tới
    struct sockaddr_un svaddr, claddr;
    int fd, j;
    ssize_t numBytes;
    socklen_t len;
    char buf[BUF_SIZE];

    // Xóa file socket cũ nếu còn tồn tại từ lần chạy trước để tránh lỗi bind
    unlink(SOCK_PATH);

    // Tạo socket Unix Domain kiểu Datagram (SOCK_DGRAM)
    fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd == -1) 
        return 1;

    // Thiết lập cấu trúc địa chỉ cho server với đường dẫn file SOCK_PATH
    memset(&svaddr, 0, sizeof(struct sockaddr_un)); 
    svaddr.sun_family = AF_UNIX;
    strncpy(svaddr.sun_path, SOCK_PATH, sizeof(svaddr.sun_path)-1);

    // Gắn socket fd với file đường dẫn socket trên hệ thống file
    if (bind(fd, (struct sockaddr *) &svaddr, sizeof(struct sockaddr_un)) == -1) 
        return 1; 
    printf("start listening on server\n");
    
    // Vòng lặp vô hạn xử lý nhận và phản hồi từng gói tin
    for (;;) {
        len = sizeof(struct sockaddr_un);

        /*
         * recvfrom(): Chặn chờ nhận gói tin từ client.
         * Tự động lưu địa chỉ socket của bên gửi vào biến claddr
         * và trả về số byte thực tế đọc được vào buf.
         */
        numBytes = recvfrom(fd, buf, BUF_SIZE, 0, (struct sockaddr *) &claddr, &len);
        if (numBytes == -1)
            return 1;
            
        printf("Server received %ld bytes from %s \n", (long) numBytes, claddr.sun_path); 

        // Chuyển toàn bộ các ký tự nhận được thành chữ in hoa
        for (j = 0; j < numBytes; j++) 
            buf[j] = toupper((unsigned char) buf[j]);
            
        /*
         * sendto(): Gửi gói tin đã xử lý ngược lại cho client
         * dựa trên thông tin địa chỉ claddr thu được từ recvfrom().
         */
        if (sendto(fd, buf, numBytes, 0, (struct sockaddr *) &claddr, len) != numBytes) 
            printf("sendto error\n");
    }
}