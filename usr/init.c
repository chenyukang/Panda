#include <string.h>
#include <syscall.h>

int main(int argc, char **argv) {
    printf("\n====== ENTER USER SPACE =======\n\n");
    if(fork() == 0) {
        exec("/sh", NULL);
    } else {
        while(1) {
            ;
        }
    }
    return 0;
}
