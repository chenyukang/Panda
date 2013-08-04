#include <string.h>
#include <syscall.h>

int main(int argc, char **argv) {
    printf("\n====== ENTER USER SPACE =======\n\n");
    if(fork() == 0) {
        exec("/home/sh", NULL);
        while(1) {
            ;
        }
    } else {
        //printf("parent\n");
        while(1) {
            ;
        }
    }
    return 0;
}
