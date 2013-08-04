
/*******************************************************************************
 *
 *      vi.c
 *
 *      @brief
 *
 *      @author   Yukang Chen  @date  2013-08-04 10:50:42
 *
 *      COPYRIGHT (C) 2006~2012, Nextop INC., ALL RIGHTS RESERVED.
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
    int fd;
    int idx = 0;
    
    if( argc != 2) {
        printf("usage: vi <filename>\n");
        return -1;
    }
    strcpy(filename, argv[1]);
    fd = open(filename, O_CREATE|O_RDWR, 0);
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
    
    printf("exiting vi ...\n");
    return 0;
}

















