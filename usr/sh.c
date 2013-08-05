#include <string.h>
#include <syscall.h>
#include <uname.h>

#define LEN 1024

char    buf[LEN];
char*   argv[LEN];
int     argc;

int parse() {
    int i, len;
    argc = 0;
    len = strlen(buf);
    for(i=0; i<len; ) {
        if(buf[i] == ' ' || buf[i] == '\n') {
            buf[i++] = '\0';
            continue;
        }
        else {
            argv[argc++] = &buf[i];
            while(i < len &&
                  (buf[i] != ' ' && buf[i] != '\n'))
                i++;
        }
    }
    return argc >= 1;
}

int main() {
    int i, ret, child;
    struct utsname name;
    char cd[3];
    strcpy(cd, "cd");
    uname(&name);
    printf("%s %s\n", name.sysname, name.version);
    while(1) {
        printf("[PandaOS]$ ");
        memset(buf, 0, sizeof(buf));
        memset(argv, 0, sizeof(argv));
        if(read(0, buf, LEN) > 0) {
            if(!parse()) continue;
            if(argv[0][0] == 'c' && argv[0][1] == 'd') {
                if(argc == 2) {
                    chdir(argv[1]);
                }
                continue;
            }
            
            if(fork() == 0) {
                char cmd[LEN];
                memset(cmd, 0, LEN);
                strcpy(cmd, "/home/");
                strcat(cmd, argv[0]);
                exec(cmd, &argv[1]);
            }
            wait(-1, &ret);
            if(ret == 1) {
                printf("no such command: %s\n", argv[0]);
            } else {
                //executed succ
                //executed error
            }
        }
    }
    return 0;
}










