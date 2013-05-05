#include <syscall.h>
#include <string.h>

int exit(int ret) {
    exitc(ret);
    printf("yes\n");
    return 0;
}
