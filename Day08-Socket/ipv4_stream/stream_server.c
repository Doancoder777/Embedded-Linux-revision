// ============================================================================
// server.c: server chat TCP đơn giản, chỉ phục vụ MỘT client tại một thời điểm.
// Cách chạy: ./server <số cổng>          ví dụ: ./server 5555
// Luồng: socket -> setsockopt -> bind -> listen -> accept -> chat (read/write)
// ============================================================================

#include <stdio.h>          // printf, fgets, perror, stdin
#include <stdlib.h>         // exit, EXIT_FAILURE, atoi, system
#include <string.h>         // memset, strncmp
#include <errno.h>          // errno (chưa dùng, chỉ xuất hiện trong dòng đang bị comment)
#include <sys/socket.h>     // Chứa cấu trúc cần thiết cho socket. 
#include <netinet/in.h>     // Thư viện chứa các hằng số, cấu trúc khi sử dụng địa chỉ trên internet
#include <arpa/inet.h>      // (chưa dùng trong server) inet_pton, inet_ntop...
#include <unistd.h>         // read, write, close, sleep

#define LISTEN_BACKLOG 50   // số kết nối tối đa được xếp hàng chờ accept()
#define BUFF_SIZE 256       // kích thước vùng đệm gửi/nhận (byte)

// handle_error(msg): in thông báo lỗi hệ thống rồi thoát chương trình.
//   perror(msg) in ra dạng "msg: <lý do lỗi>" (lý do lấy từ biến errno)
//   do { ... } while (0): gom nhiều câu lệnh thành một khối, dùng được như một câu lệnh
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)


/* Chức năng chat */
// new_socket_fd: file descriptor của socket nói chuyện với MỘT client cụ thể
//                (do accept() trả về, khác với server_fd dùng để lắng nghe)
void chat_func(int new_socket_fd)
{       
    int numb_read, numb_write;   // số byte mà read()/write() trả về (-1 nếu lỗi)
    char sendbuff[BUFF_SIZE];    // vùng đệm chứa tin nhắn sẽ gửi cho client
    char recvbuff[BUFF_SIZE];    // vùng đệm chứa tin nhắn nhận từ client
    
    while (1) {        
        // Điền giá trị 0 (ký tự NUL '\0') vào cả hai vùng đệm trước mỗi lượt chat.
        // Lưu ý: khác với ký tự '0' (mã ASCII 48).
        memset(sendbuff, 0, BUFF_SIZE);
        memset(recvbuff, 0, BUFF_SIZE);

        /* Đọc dữ liệu từ socket */
        /* Hàm read sẽ block cho đến khi đọc được dữ liệu từ client */
        // read trả về số byte đọc được; trả về 0 nếu client đã đóng kết nối, trả về -1 nếu lỗi
        numb_read = read(new_socket_fd, recvbuff, BUFF_SIZE);
        if(numb_read == -1)
            handle_error("read()");
        if (numb_read == 0) {                    // client đã ngắt kết nối (hoặc tắt terminal)
            printf("Client exit ...\n");
            break;                               // thoát vòng lặp chat, kết thúc hàm để server quay lại accept() đón client khác
        }
        // So sánh 4 ký tự đầu của tin nhận được với "exit": trùng thì kết thúc cuộc chat
        if (strncmp("exit", recvbuff, 4) == 0) {
            system("clear");     // chạy lệnh shell "clear" để xóa màn hình terminal
            break;
        }
        printf("\nMessage from Client: %s\n", recvbuff);

        /* Nhập phản hồi từ bàn phím */
        printf("Please respond the message : ");
        // fgets: đọc một dòng từ bàn phím vào sendbuff, tối đa BUFF_SIZE-1 ký tự,
        // giữ lại ký tự '\n' và tự động thêm ký tự kết thúc chuỗi '\0' ngay sau đó
        fgets(sendbuff, BUFF_SIZE, stdin);

        /* Ghi dữ liệu tới client thông qua hàm write */
        // Gửi toàn bộ 256 byte của sendbuff sang client qua kết nối socket
        numb_write = write(new_socket_fd, sendbuff, sizeof(sendbuff));
        if (numb_write == -1)
            handle_error("write()");
        
        // Tin server vừa gõ bắt đầu bằng "exit" thì server cũng chủ động kết thúc cuộc chat
        if (strncmp("exit", sendbuff, 4) == 0) {
            system("clear");
            break;
        }

        sleep(1);                // nghỉ 1 giây trước lượt tiếp theo
    }
    close(new_socket_fd);        // đóng socket kết nối riêng với client này khi cuộc chat kết thúc
}

int main(int argc, char *argv[])
{
    // port_no       : số cổng server sẽ lắng nghe (lấy từ tham số dòng lệnh)
    // len           : kích thước của client_addr (accept() đọc để biết dung lượng và ghi đè độ dài thực tế vào đây)
    // opt           : biến truyền địa chỉ vào setsockopt
    int port_no, len, opt; 
    // server_fd     : socket LẮNG NGHE, mở cổng và duy trì hàng đợi chờ client
    // new_socket_fd : socket KẾT NỐI RIÊNG 1-1 với một client cụ thể, do accept() sinh ra
    int server_fd, new_socket_fd;
    // serv_addr     : struct lưu cấu hình địa chỉ IP + cổng của server
    // client_addr   : struct để Kernel tự động điền IP + cổng của client khi kết nối tới
    struct sockaddr_in serv_addr, client_addr;

    /* Đọc portnumber trên command line */
    // argc = số tham số dòng lệnh (tính cả tên chương trình), argv = mảng các tham số.
    // Cần ít nhất 2: argv[0] = ./server, argv[1] = số cổng
    if (argc < 2) {
        printf("No port provided\ncommand: ./server <port number>\n");
        exit(EXIT_FAILURE);
    } else
        port_no = atoi(argv[1]);    // atoi: đổi chuỗi "5555" thành số nguyên 5555

    /*
    câu lệnh khi chạy gồm ./server <port number>
    trong đó argv[0] là ./server, argv[1] là <port number>
    */

    // Xóa toàn bộ hai struct về 0 (số 0 thật) để xóa sạch dữ liệu rác trên RAM
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    
    /* Tạo socket */
    // AF_INET     : họ địa chỉ IPv4
    // SOCK_STREAM : kiểu truyền luồng byte liên tục, tin cậy (giao thức TCP)
    // 0           : chọn giao thức mặc định tương ứng (TCP)
    // Trả về file descriptor của socket (số nguyên không âm), hoặc -1 nếu lỗi
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
        handle_error("socket()");
    // fprintf(stderr, "ERROR on socket() : %s\n", strerror(errno));

    /* Ngăn lỗi : “address already in use” */
    // Tránh lỗi chiếm dụng cổng khi vừa tắt server cũ mà bật lại ngay (do trạng thái TIME_WAIT của TCP)
    // setsockopt: thiết lập tùy chọn cấu hình cho socket
    //   SOL_SOCKET                  : mức cấu hình ở tầng socket
    //   SO_REUSEADDR | SO_REUSEPORT : tùy chọn cho phép dùng lại địa chỉ/cổng ngay lập tức
    //   &opt, sizeof(opt)           : địa chỉ biến lưu giá trị cấu hình và kích thước của nó
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
        handle_error("setsockopt()");  

    /* Khởi tạo địa chỉ cho server */
    serv_addr.sin_family = AF_INET;      // Họ địa chỉ IPv4
    serv_addr.sin_port = htons(port_no); // Chuyển số cổng từ Host Byte Order (thường là Little-Endian) sang Network Byte Order (Big-Endian)
    // INADDR_ANY (0.0.0.0): Chấp nhận kết nối gửi tới bất kỳ địa chỉ IP/card mạng nào của máy này
    serv_addr.sin_addr.s_addr =  INADDR_ANY; //inet_addr("192.168.5.128"); //INADDR_ANY

    /* Gắn socket với địa chỉ server */
    // bind: ràng buộc socket server_fd với thông tin IP và Cổng được thiết lập trong serv_addr.
    // Ép kiểu con trỏ sang (struct sockaddr*) để tương thích với giao diện hàm tổng quát của hệ thống
    if (bind(server_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        handle_error("bind()");

    /* Nghe tối đa LISTEN_BACKLOG (50) kết nối trong hàng đợi */
    // listen(): CHỈ CẦN GỌI ĐÚNG 1 LẦN để chuyển server_fd sang trạng thái bị động (chờ kết nối).
    // LISTEN_BACKLOG (50): Kích thước hàng đợi ở tầng Kernel, cho phép tối đa 50 client chờ cùng lúc.
    // Sau lệnh này, cổng mở liên tục và Kernel tự động xếp client vào hàng đợi mà không cần gọi lại listen().
    if (listen(server_fd, LISTEN_BACKLOG) == -1)
        handle_error("listen()");

    /* Dùng để lấy thông tin client */
    // Khởi tạo len bằng dung lượng vùng nhớ client_addr (16 byte với IPv4) trước khi đưa vào accept()
    len = sizeof(client_addr);

    // Vòng lặp vô hạn: Phục vụ xong một client thì quay lại bốc client tiếp theo từ hàng đợi
    while (1) {
        printf("Server is listening at port : %d \n....\n",port_no);
        // accept(): Lệnh trực tiếp đón từng client ra khỏi hàng đợi của listen():
        // - Nếu hàng đợi rỗng: Hàm sẽ block (treo) chờ cho tới khi có client mới kết nối.
        // - Khi có client: Kernel điền IP, port của client vào client_addr, cập nhật kích thước vào len,
        //   và trả về new_socket_fd (fd mới độc lập dùng riêng để chat với client này).
        new_socket_fd  = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t *)&len); 
        if (new_socket_fd == -1)
            handle_error("accept()");

        system("clear");
        
        //char temp[BUFF_SIZE];
        //inet_ntop(client_addr.sin_family, (struct sockaddr*)&client_addr, temp, sizeof(temp));
        printf("Server : got connection \n");
        // Gọi hàm chat 1-1 với client vừa kết nối. Khi chat_func kết thúc (client thoát),
        // vòng while (1) sẽ quay lên đầu và gọi lại accept() để đón client tiếp theo đang chờ.
                chat_func(new_socket_fd);
    }
    // Hai dòng dưới không bao giờ chạy tới, vì vòng while (1) ở trên là vô hạn
    close(server_fd);
    return 0;
}