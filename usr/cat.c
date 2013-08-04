#include <fcntl.h>
#include <string.h>
#include <syscall.h>
#include <stat.h>
#include <dirent.h>

int main(int argc, char* argv[]) {
    char path[1024];
    char buf[1024];
    char pwd[1024];
    
    int fd, cnt;
    struct stat s;
    if(argc <= 1) {
        printf("cat: need a argument\n");
        return -1;
    }
    memset(path, 0, sizeof(path));
    memset(buf,  0, sizeof(buf));
    memset(pwd, 0, sizeof(pwd));
    if(argv[1][0] != '/') {
        getcwd(pwd, 1024);
        pwd[strlen(pwd)] = '/';
        strcpy(path, pwd);
    }

    strcat(path, argv[1]);
    if((fd = open(path, O_RDONLY, 0)) < 0) {
        printf("cat: open %s failed\n", path);
        return -1;
    }
    if(stat(path, &s) < 0) {
        printf("cat: stat %s failed\n", path);
        return -1;
    }
    if(!S_ISREG(s.st_mode)) {
        printf("cat: %s is not a regular file\n", path);
        return -1;
    }
    while((cnt = read(fd, buf, sizeof(buf))) > 0) {
        printf("%s", buf);
    }
    
    printf("\n");
    close(fd);
    return 0;
}
