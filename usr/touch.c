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
    int k, fd;
    int cnt;
    char path[1024];
    char buf[1024];
    for(k=1; k<argc; k++) {
        memset(path, 0, sizeof(path));
        memset(buf, 0, sizeof(buf));
        strcpy(path, argv[k]);
        strcpy(buf, "hello");
        fd = open(path, O_CREATE|O_RDWR, 0);
        write(fd, buf, strlen(buf));
        close(fd);

        memset(buf, 0, sizeof(buf));
        fd = open(path, O_RDONLY, 0);
        if(fd > 0) {
            while((cnt = read(fd, buf, sizeof(buf))) > 0) {
                printf("content:%s\n", buf);
            }
        }
    }
    return 0;
}
