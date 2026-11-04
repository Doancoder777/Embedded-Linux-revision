// ============================================================================
// client.c: client chat TCP đơn giản, kết nối tới server rồi gửi/nhận tin nhắn.
// Cách chạy: ./client <địa chỉ server> <số cổng>    ví dụ: ./client 127.0.0.1 5555
// Luồng: socket -> connect -> chat (write rồi read, lặp lại)
// ============================================================================

#include <stdio.h>          // printf, fgets, perror, stdin
#include <stdlib.h>         // exit, EXIT_FAILURE, atoi
#include <string.h>         // memset, strncmp
#include <errno.h>          // errno (chưa dùng trực tiếp trong file này)
#include <sys/socket.h>     //  Chứa cấu trúc cần thiết cho socket. 
#include <netinet/in.h>     //  Thư viện chứa các hằng số, cấu trúc khi sử dụng địa chỉ trên internet
#include <arpa/inet.h>      // inet_pton (đổi chuỗi IP thành số nhị phân)
#include <unistd.h>         // read, write, close

#define BUFF_SIZE 256       // kích thước vùng đệm gửi/nhận (byte)

// handle_error(msg): in thông báo lỗi hệ thống rồi thoát chương trình.
//   perror(msg) in ra dạng "msg: <lý do lỗi>" (lý do lấy từ biến errno)
#define handle_error(msg) \
do { perror(msg); exit(EXIT_FAILURE); } while (0)
/* Chức năng chat */
// server_fd: file descriptor của socket đã kết nối tới server
void chat_func(int server_fd)
{
    int numb_write, numb_read;   // số byte mà write()/read() trả về (-1 nếu lỗi)
    char recvbuff[BUFF_SIZE];    // vùng đệm chứa tin nhắn nhận từ server
    char sendbuff[BUFF_SIZE];    // vùng đệm chứa tin nhắn sẽ gửi cho server
    while (1) {
        // Điền ký tự '0' (mã ASCII 48, KHÔNG phải số 0) vào cả hai vùng đệm
        memset(sendbuff, '0', BUFF_SIZE);
        memset(recvbuff, '0', BUFF_SIZE);
        printf("Please enter the message : ");
        // fgets: đọc một dòng từ bàn phím vào sendbuff, tối đa BUFF_SIZE-1 ký tự,
        // giữ lại ký tự '\n' và thêm '\0' ngay sau đó
        fgets(sendbuff, BUFF_SIZE, stdin);

        /* Gửi thông điệp tới server bằng hàm write */
        // Gửi sizeof(sendbuff) = 256 byte (gồm cả phần '0' đệm phía sau ký tự kết thúc chuỗi)
        numb_write = write(server_fd, sendbuff, sizeof(sendbuff));
        if (numb_write == -1)  handle_error("write()"); // if này dùng để kiểm tra xem hàm write có bị lỗi hay không, nếu có thì in ra thông báo lỗi và thoát chương trình.
        // So sánh 4 ký tự đầu của tin vừa gửi với "exit": trùng thì client thoát vòng chat
        if (strncmp("exit", sendbuff, 4) == 0) {
            printf("Client exit ...\n");
            break;
        }
        /* Nhận thông điệp từ server bằng hàm read */
        // read block cho đến khi server gửi dữ liệu về; trả về số byte đọc được
        // Nếu server đã đóng kết nối thì read trả về 0, nếu lỗi thì trả về -1
        numb_read = read(server_fd, recvbuff, sizeof(recvbuff));
        if (numb_read < 0)  handle_error("read()");
        if (numb_read == 0) {                    // server đã đóng kết nối
            printf("Server exit ...\n");
            break;
        }
        if (strncmp("exit", recvbuff, 4) == 0) {
            printf("Server exit ...\n");
            break;
        }
        printf("\nMessage from Server: %s\n",recvbuff);
    }
    close(server_fd); /*close*/     // đóng kết nối với server
}

int main(int argc, char *argv[])
{
    int portno;                      // số cổng của server cần kết nối (lấy từ dòng lệnh)
    int server_fd;                   // fd của socket dùng để kết nối tới server
    struct sockaddr_in serv_addr;    // địa chỉ của server (họ địa chỉ + cổng + IP)
    // Điền ký tự '0' (không phải số 0) vào toàn bộ serv_addr.
    // Bên server dùng số 0 thật; các trường cần dùng bên dưới đều được gán lại.
    memset(&serv_addr, '0',sizeof(serv_addr));
    /* Đọc portnumber từ command line */
    // argc = số tham số dòng lệnh (tính cả tên chương trình), argv = mảng các tham số.
    // Cần ít nhất 3: argv[0] = ./client, argv[1] = địa chỉ server, argv[2] = số cổng
    if (argc < 3) {
        printf("command : ./client <server address> <port number>\n");
        exit(1);
    }

    /*
    câu lệnh khi chạy gồm ./client <server address> <port number>
    trong đó argv[0] là ./client, argv[1] là <server address>, argv[2] là <port number>
    */
    portno = atoi(argv[2]);          // atoi: đổi chuỗi "5555" thành số nguyên 5555
    printf("Server address: %s, Port number: %d\n", argv[1], portno);
    /* Khởi tạo địa chỉ server */
    serv_addr.sin_family = AF_INET;          // IPv4
    serv_addr.sin_port   = htons(portno);    // htons: đổi số cổng sang thứ tự byte của mạng (big-endian)
    // inet_pton: đổi chuỗi IP (ví dụ "127.0.0.1") thành số nhị phân dạng mạng, lưu vào serv_addr.sin_addr.
    // Trả về 1 nếu thành công, 0 nếu chuỗi sai định dạng, -1 nếu lỗi hệ thống.
    // Code chỉ kiểm tra -1, nên bỏ sót trường hợp 0 (ví dụ nhập "abc" sẽ trả về 0).
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) == -1) 
        handle_error("inet_pton()");
    /* Tạo socket */
    // AF_INET = IPv4, SOCK_STREAM = TCP, 0 = giao thức mặc định; trả về fd hoặc -1 nếu lỗi
    /*
    upd : socket(AF_INET, SOCK_DGRAM, 0)
    stream : socket(AF_INET, SOCK_STREAM, 0)
    */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
        handle_error("socket()");
    /* Kết nối tới server*/
    // connect: kết nối server_fd tới địa chỉ trong serv_addr (block cho tới khi xong hoặc lỗi).
    // (struct sockaddr *)&serv_addr: địa chỉ của biến, ép từ sockaddr_in sang kiểu chung sockaddr
    // vì serv_addr có dấu "&" ở trước vì connect() cần địa chỉ của biến, chứ không phải giá trị của biến.
    if (connect(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        handle_error("connect()");
    chat_func(server_fd);            // kết nối xong thì bắt đầu chat

    return 0;
}