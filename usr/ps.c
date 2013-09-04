#include <string.h>
#include <stdlib.h>
#include <syscall.h>

int main() {
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    procs(buf, 1024);
    printf("current task list:\n");
    printf("%s", buf);
    return 0;
}
