#include <string.h>
#include <syscall.h>

int main() {
    printf("power off now ! \n");
    halt();
    return 0;
}
