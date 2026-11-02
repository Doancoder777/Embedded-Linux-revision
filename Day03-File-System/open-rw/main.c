// C program to illustrate 
// I/O system Calls 
#include<stdio.h> 
#include<string.h> 
#include<unistd.h> 
#include<fcntl.h> 
int main (void) 
{ 
int fd; 
int numb_read, numb_write;
char buf1[12] = "hello world\n"; 
char buf2[32];                                   // THÊM: vùng nhớ chứa dữ liệu đọc được
// assume foobar.txt is already created 
fd = open("hello.txt", O_RDWR | O_CREAT , 0777);         
if (-1 == fd) { 
printf("open() hello.txt failed\n");
    }      

numb_write = write(fd, buf1, strlen(buf1));
printf("Write %d bytes to hello.txt\n", numb_write);
lseek(fd, 2, SEEK_SET);
write(fd, "AAAAAAAAAAAA", strlen("AAAAAAAAAAAA"));

lseek(fd, 0, SEEK_SET);                          // THÊM: về đầu file
numb_read = read(fd, buf2, sizeof(buf2) - 1);    // THÊM: đọc, chừa 1 byte cho '\0'
buf2[numb_read] = '\0';                          // THÊM: tự thêm ký tự kết thúc chuỗi
printf("Read %d bytes from hello.txt: %s\n", numb_read, buf2);   // THÊM

close(fd); 
return 0; 
}