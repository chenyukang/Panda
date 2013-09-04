#include <string.h>
#include <stdlib.h>
#include <syscall.h>

int main() {
    int pid = getpid();
    int ppid = getppid();

    printf("current pid: %d \nparent  pid: %d\n", pid, ppid);
    return 0;
}
