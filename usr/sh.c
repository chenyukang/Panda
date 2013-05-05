#include <string.h>
#include <syscall.h>

#define LEN 1024

char buf[LEN];
char cmd[LEN];

int do_exec(char* buf) {
    if(fork() == 0) {
        printf("buf:%s\n", buf);
        exec(buf, NULL);
    }
    return 0;
}

int main() {
    int child;
    while(1) {
        printf("[PandaOS]$ ");
        memset(buf, 0, LEN);
        while(read(0, buf, LEN) > 0) {
            //printf("line: %s", buf);
            //if(do_exec(buf) == -1) {
            //printf("Error: no such command [%s]\n", buf);
            //}
            if(fork() == 0) {
                memset(cmd, 0, LEN);
                cmd[0]='/';
                strcat(cmd, buf);
                exec(cmd, NULL);
                while(1) {
                    ;
                }
            } else {
                while(1) {
                    ;
                }
                printf("[PandaOS]$ ");
                memset(buf, 0, LEN);
            }
        }
    }
    return 0;
}
