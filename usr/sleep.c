#include <syscall.h>
#include <string.h>

int main() {
    printf("begin sleep\n");
    sleep(200);
    printf("end sleep\n");
    return 0;
}

