#include <string.h>
#include <syscall.h>
#include <uname.h>

#define LEN 1024
char    buf[LEN];
char*   argv[LEN];
int     argc;

int parse() {
    int i;
    int len;
    argc = 0;
    len = strlen(buf);
    for(i=0; i<len; ){
        if(buf[i] == ' ') {
            buf[i++] = '\0';
            continue;
        }
        else {
            argv[argc++] = &buf[i];
            while(i < len && buf[i] != ' ') i++;
        }
    }
    return argc >= 1;
}

int main() {
    int child;
    int ret;
    int i;
    struct utsname name;
    uname(&name);
    printf("%s %s\n", name.sysname, name.version);
    while(1) {
        printf("[PandaOS]$ ");
        memset(buf, 0, sizeof(buf));
        memset(argv, 0, sizeof(argv));
        if(read(0, buf, LEN) > 0) {
            if(parse() != 1) continue;
            if(fork() == 0) {
                char cmd[LEN];
                memset(cmd, 0, LEN);
                cmd[0]='/';
                strcat(cmd, argv[0]);
                exec(cmd, &argv[1]);
                exit(1);
            }
            wait(-1, &ret);
        }
    }
    return 0;
}
