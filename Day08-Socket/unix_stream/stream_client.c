#include <sys/un.h>
#include <sys/socket.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>     // exit()
#include <string.h>     // memset, strncpy
#include <unistd.h>     // read, write, close

#define BUF_SIZE 10 
#define SOCK_PATH "./sock_stream"

int main(int argc, char *argv[])
{    
    struct sockaddr_un svaddr;
    int fd;
    char buff[BUF_SIZE] = {0};
    
    // Dùng argc để kiểm tra tham số, vừa hết warning unused parameter, vừa tránh crash
    if (argc < 2) {
        printf("Usage: %s <message>\n", argv[0]);
        return 1;
    }
    
    fd = socket(AF_UNIX, SOCK_STREAM, 0);      
    if (fd == -1)
        return 1;
    
    memset(&svaddr, 0, sizeof(struct sockaddr_un));
    svaddr.sun_family = AF_UNIX;
    strncpy(svaddr.sun_path, SOCK_PATH, sizeof(svaddr.sun_path) - 1);
    
    //sửa connect() để kiểm tra lỗi và in ra thông báo nếu kết nối thất bại
    if (connect(fd, (struct sockaddr*)&svaddr, sizeof(struct sockaddr_un)) != 0) 
    { 
        printf("connection with the server failed...\n"); 
        close(fd);
        return 1; 
    } 
    else
    {
        printf("connected to the server.\n"); 
    }

    printf("send : %s\n", argv[1]); 
    write(fd, argv[1], BUF_SIZE);
    
    while(1)
    {      
        memset(buff, 0, sizeof(buff));
        if(read(fd, buff, sizeof(buff)) > 0) 
            printf("From Server : %s\n", buff); 
        else
            break; // Thoát khi server đóng kết nối
    } 
    
    close(fd);
    return 0;
}