#include <syscall.h>
#include <string.h>
#include <stdlib.h>

int exit(int ret) {
    return kexit(ret);
}
