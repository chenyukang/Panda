#include <syscall.h>
#include <string.h>

int main() {
    printf("begin sleep\n");
    sleep(10);
    printf("end sleep\n");
    return 0;
}

