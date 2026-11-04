#include <sys/un.h>
#include <sys/socket.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 10 
#define SOCK_PATH "./sock_dgram"

int main(int argc, char *argv[])
{    
	/*
     * optval (Option Value): Cờ bật/tắt giá trị cấu hình socket (1 = bật, 0 = tắt).
     * Dùng cho setsockopt() bật SO_PASSCRED: cho phép gửi kèm thông tin định danh
     * tiến trình (PID, UID, GID) sang bên nhận để phục vụ xác thực bảo mật.
     */
    int fd, optval;
    struct sockaddr_un svaddr;
    int fd, optval;	
    ssize_t msgLen;
    ssize_t numBytes;
    char resp[BUF_SIZE];
    
    // Kiểm tra tham số dòng lệnh để tránh lỗi Segmentation fault và hết cảnh báo argc
    if (argc < 2) {
        printf("Usage: %s <message>\n", argv[0]);
        return 1;
    }

    // Tạo socket Unix Domain kiểu Datagram (SOCK_DGRAM)
    fd = socket(AF_UNIX, SOCK_DGRAM, 0);      
    if (fd == -1)
        return 1;
    
    // Khởi tạo thông tin địa chỉ socket server đích cần gửi đến
    memset(&svaddr, 0, sizeof(struct sockaddr_un)); 
    svaddr.sun_family = AF_UNIX;
    strncpy(svaddr.sun_path, SOCK_PATH, sizeof(svaddr.sun_path)-1);
    
    // Bật tùy chọn SO_PASSCRED để nhận/truyền thông tin định danh tiến trình
    optval = 1;
    setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &optval, sizeof(optval));
    
	// Dùng kiểu ssize_t để khớp hoàn toàn với kiểu trả về của sendto.
    // ssize_t (signed size) là số nguyên có dấu, dùng cho các hàm I/O hệ thống
    // để vừa biểu diễn được số byte truyền nhận (>= 0), vừa trả về -1 khi lỗi;
    // tránh cảnh báo -Wsign-compare khi so sánh trực tiếp với size_t (không dấu).
    msgLen = (ssize_t)strlen(argv[1]);
    /*
     * sendto(): Gửi trực tiếp chuỗi argv[1] đến server tại svaddr.
     * Do là datagram nên không cần gọi connect() trước.
     */
    if (sendto(fd, argv[1], msgLen, 0, (struct sockaddr *)&svaddr, sizeof(struct sockaddr_un)) != msgLen)
        return 1;

    /*
     * recvfrom(): Chặn chờ nhận gói tin phản hồi từ server vào mảng resp.
     * NULL, NULL nghĩa là không cần lấy thông tin địa chỉ bên phản hồi.
     */
	numBytes = recvfrom(fd, resp, BUF_SIZE - 1, 0, NULL, NULL);
    if (numBytes == -1) {
        return 1;
    } else {
        resp[numBytes] = '\0'; // Đặt ký tự kết thúc chuỗi tại vị trí kết thúc dữ liệu
        printf("Response : %s\n", resp);
    }
    
    return 0;
}