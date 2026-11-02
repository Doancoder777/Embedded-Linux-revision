#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) 
{    
    printf("Run command <ls -lah> after 2 seconds\n");
    sleep(2);
    execl("/bin/ls", "ls", "-l", "-a", "-h", NULL);
    /*
     * execl() sẽ thay thế tiến trình hiện tại bằng lệnh ls -lah
     * Nếu execl() thành công, dòng code phía dưới sẽ không bao giờ được thực thi
     * 
     * cac tham so truyen vao execl() la:
     * 1. Duong dan den chuong trinh can chay ("/bin/ls")
     * 2. Ten chuong trinh (argv[0]) khi chuong trinh duoc goi (trong truong hop nay la "ls")   
     * 3. cac tham so truyen vao chuong trinh (argv[1], argv[2], ...) (trong truong hop nay la "-l", "-a", "-h")
     * 
     * execl("/bin/ls", "ls", "-l", "-a", "-h", NULL);
     * 1. Duong dan den chuong trinh can chay ("/bin/ls")
     * 2. Ten chuong trinh (argv[0]) la "ls"
     * 3. cac tham so truyen vao chuong trinh (argv[1], argv[2], ...) la "-l", "-a", "-h"
     * 4. NULL la ket thuc danh sach tham so
     */

    return 0;   
}
