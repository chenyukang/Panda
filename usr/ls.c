#include <fcntl.h>
#include <string.h>
#include <syscall.h>
#include <stat.h>
#include <dirent.h>


int main(int argc, char* argv[]) {
    struct stat s;
    int fd,  k, i;
    char path[1024];
    struct dirent dire;
    
    memset(path, 0, sizeof(path));
    memset(&dire, 0, sizeof(dire));
    if(argc == 1) {
        strcpy(path, "/");
    } else {
        path[0] = '/';
        strcat(path, argv[1]);
    }
    if((fd = open(path, O_RDONLY, 0)) < 0) {
        printf("open failed: %s\n", path);
        return -1;
    }
    if(stat(path, &s) < 0)  {
        printf("stat error: %s\n", path);
        return -1;
    }
    
    if(S_ISREG(s.st_mode)) {
        printf("regular file\n");
    }
    else if(S_ISDIR(s.st_mode)) {
        printf("dir:%s\n", path);
        while(read(fd, &dire, sizeof(struct dirent)) == sizeof(struct dirent)) {
                if(dire.d_ino == 0) continue;
                printf("%s\n", dire.d_name);
            }
    }
    
    return 0;
}
