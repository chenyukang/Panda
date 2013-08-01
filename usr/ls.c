#include <fcntl.h>
#include <string.h>
#include <syscall.h>
#include <stat.h>
#include <dirent.h>
#define MAX 14

void fmt(char* buf) {
    int len = strlen(buf);
    int i;
    for(i=0; i<len-1; i++) {
        buf[i] = buf[i+1];
    }
    for(i=len-1; i<MAX; i++) {
        buf[i] = ' ';
    }
    buf[MAX] = 0;
}

int ls(char* path) {
    struct stat s;
    struct dirent dire;
    char buf[512];
    int fd;
    
    memset(&dire, 0, sizeof(dire));
    if((fd = open(path, O_RDONLY, 0)) < 0) {
        printf("open failed: %s\n", path);
        return -1;
    }
    if(stat(path, &s) < 0)  {
        printf("stat error: %s\n", path);
        close(fd);
        return -1;
    }
    
    if(S_ISREG(s.st_mode)) {
        strcat(buf, path);
        if(stat(buf, &s) < 0) {
            printf("stat error: %s\n", path);
            close(fd);
            return -1;
        }
        fmt(buf);
        printf("name:%s size:%d\n", buf, s.st_size);
    }
    else if(S_ISDIR(s.st_mode)) {
        while(read(fd, &dire, sizeof(struct dirent)) == sizeof(struct dirent)) {
            if(dire.d_ino == 0) continue;
            memset(buf, 0, sizeof(buf));
            memset(&s, 0, sizeof(s));
            buf[0] = '/';
            strcat(buf, dire.d_name);
            if(stat(buf, &s) < 0) {
                printf("stat error: %s\n", path);
                close(fd);
                return -1;
            }
            fmt(buf);
            printf("name: %s size: %d\n", buf,  s.st_size);
        }
    }
    close(fd);
    return 0;
}

int main(int argc, char* argv[]) {
    int k, i;
    char path[1024];
    
    memset(path, 0, sizeof(path));
    if(argc == 1) {
        strcpy(path, "/");
    } else {
        path[0] = '/';
        strcat(path, argv[1]);
    }
    return ls(path);
}
