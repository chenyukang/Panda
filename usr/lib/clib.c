#include <syscall.h>
#include <string.h>
#include <stdlib.h>

void exit(int ret) {
    kexit(ret);
    while(1)
        ;
}
