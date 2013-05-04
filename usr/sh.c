#include <string.h>
#include <syscall.h>

#define LEN 1024

char buf[LEN];

int do_exec() {
}

int main() {
    printf("in sh\n");
    while(1) {
        printf("$ ");
        memset(buf, 0, LEN);
        if(read(0, buf, LEN)) {
            do_exec();
        }
    }
    return 0;
}
