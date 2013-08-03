#include <fcntl.h>
#include <string.h>
#include <syscall.h>
#include <stat.h>
#include <dirent.h>

int main(int argc, char* argv[] ) {
    if(argc == 1) {
        printf("usage: touch <filename>\n");
        return -1;
    }
    int k;
    char buf[1024];
    for(k=1; k<argc; k++) {
        memset(buf, 0, sizeof(buf));
        strcpy(buf, argv[k]);
        int fd = open(buf, O_CREATE|O_RDWR, 0);
        printf("finish create in user mode\n");
        close(fd);
    }
    return 0;
}
