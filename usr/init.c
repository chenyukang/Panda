#include <string.h>
#include <syscall.h>

int main(int argc, char **argv) {
    printf("\n====== ENTER USER SPACE =======\n");
    if(fork() == 0) {
        printf("child\n");
        while(1) {
            ;
        }
        
    } else {
        printf("parent\n");
        while(1) {
            ;
        }
    }
    return 0;
}
