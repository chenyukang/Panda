#include <string.h>
#include <syscall.h>
#include <stat.h>

static void run(char* path) {
    if(fork() == 0) {
        exec(path, NULL);
        while(1) {
            ;
        }
    }
    while(1) {
        ;
    }
}

int main(int argc, char **argv) {
    struct stat s;

    printf("\n====== ENTER USER SPACE =======\n\n");
    if(stat("/home/.ostest", &s) == 0)
        run("/home/ostest");

    run("/home/sh");
    return 0;
}
