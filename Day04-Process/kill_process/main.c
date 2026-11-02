#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) 
{    
    printf("Run command <ls -lah> after 1 seconds\n");
    sleep(1);
    while (1) {
       printf("hello\n");
       sleep(1);
    }
    return 0;   
}
