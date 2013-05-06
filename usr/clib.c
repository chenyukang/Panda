#include <syscall.h>
#include <string.h>

int exit(int ret) {
    return exitc(ret);
}
