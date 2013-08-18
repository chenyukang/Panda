/*******************************************************************************
 *
 *      vi.c
 *
 *      @brief
 *
 *      @author   Yukang Chen  @date  2013-08-04 10:50:42
 *
 *******************************************************************************/

#include <string.h>
#include <stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <syscall.h>


int main(int argc, char* argv[]) {
    char buf[1024];
    char filename[32];
    char c;
    struct stat s;
    int fd, idx = 0;

    if( argc != 2) {
        printf("usage: vi <filename>\n");
        return -1;
    }
    strcpy(filename, argv[1]);
    memset(&s, 0, sizeof(s));
    if(stat(filename, &s) < 0) {
        fd = open(filename, O_CREATE|O_RDWR, 0);
    } else {
        fd = open(filename, O_RDWR, 0);
    }
    if(fd < 0) {
        printf("create file: %s error\n", filename);
        return -1;
    }

    memset(buf, 0, sizeof(buf));
    while((c = getchar()) != 27) { //ESC to exit
        buf[idx++] = c;
    }
    write(fd, buf, strlen(buf));
    close(fd);

    getchar();//eat the \n
    printf("exiting vi ...\n");
    return 0;
}
