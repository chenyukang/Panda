#include <string.h>
#include <syscall.h>

#define LEN 1024

char buf[LEN];

int main() {
    int child;
    int ret;
    while(1) {
        printf("[PandaOS]$ ");
        memset(buf, 0, LEN);
        if(read(0, buf, LEN) > 0) {
            if((child = fork()) == 0) {
                char cmd[LEN];
                memset(cmd, 0, LEN);
                cmd[0]='/';
                strcat(cmd, buf);
                exec(cmd, NULL);
                exit(1);
            }  else {
                wait(-1, &ret);
            }
        }
    }
    return 0;
}
