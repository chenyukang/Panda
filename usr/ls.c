#include <fcntl.h>
#include <string.h>
#include <syscall.h>
#include <stat.h>

int main(int argc, char* argv[]) {
    struct stat s;
    int k;
    int i;
    char path[1024];
    memset(path, 0, sizeof(path));
    if(argc == 1) {
        strcpy(path, "/");
    } else {
        path[0] = '/';
        strcat(path, argv[1]);
    }
    printf("filename: %s\n", path);
    if(stat(path, &s) < 0)  {
        printf("stat error: %s\n", path);
        return -1;
    }
    
    if(S_ISREG(s.st_mode)) {
        printf("regular file\n");
    }
    else if(S_ISDIR(s.st_mode)) {
        printf("dir\n");
    }
    
    return 0;
}
