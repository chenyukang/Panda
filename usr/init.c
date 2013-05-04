#include <string.h>
#include <syscall.h>

int main(int argc, char **argv) {
    printf("\n====== ENTER USER SPACE =======\n");
    if(fork() == 0) {
        exec("/sh", NULL);
    } else {
        printf("parent\n");
        while(1) {
            ;
        }
    }
    return 0;
}
