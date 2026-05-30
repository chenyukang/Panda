#include <syscall.h>

int main(int argc, char* argv[]) {
    volatile int* bad = (int*)0x1234;
    *bad = 1;
    return 0;
}
