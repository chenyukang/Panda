#include <string.h>
#include <syscall.h>

int main(int argc, char **argv) {
    printf("\n====== ENTER USER SPACE =======\n\n");
    if(fork() == 0) {
        printf("child\n");
        exec("/sh", NULL);
    } else {
        printf("parent\n");
        while(1) {
            ;
        }
    }
    return 0;
}
