#include <string.h>
#include <syscall.h>
#include <uname.h>

#define LEN 1024
char buf[LEN];

int main() {
    int child;
    int ret;
    struct utsname name;
    uname(&name);
    printf("%s %s\n", name.sysname, name.version);
    while(1) {
        printf("[PandaOS]$ ");
        memset(buf, 0, LEN);
        if(read(0, buf, LEN) > 0) {
            if(fork() == 0) {
                char cmd[LEN];
                memset(cmd, 0, LEN);
                cmd[0]='/';
                strcat(cmd, buf);
                exec(cmd, NULL);
                printf("hahaha now \n");
                exit(1);
            }
            wait(-1, &ret);
        }
    }
    return 0;
}
