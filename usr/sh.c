#include <string.h>
#include <syscall.h>

#define LEN 1024

char buf[LEN];

int main() {
    int child;
    while(1) {
        printf("[PandaOS]$ ");
        memset(buf, 0, LEN);
        if(read(0, buf, LEN) > 0) {
            //printf("line: %s", buf);
            //if(do_exec(buf) == -1) {
            //printf("Error: no such command [%s]\n", buf);
            //}
            int ret;
            if(fork() == 0) {
                char cmd[LEN];
                memset(cmd, 0, LEN);
                cmd[0]='/';
                strcat(cmd, buf);
                exec(cmd, NULL);
                exit(1);
                //exit(1);
            } else {
                wait(4, &ret);
            }
        }
    }
    return 0;
}
