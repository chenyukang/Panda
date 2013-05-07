#include <fcntl.h>
#include <string.h>
#include <syscall.h>

int main() {
    printf("%s\n", "Hello world!\n");
    int k;
    int fp = open("/README.md", O_RDWR, 0);
    if(fp != -1) {
        printf("open README\n");
    } else {
        printf("open failed\n");
    }
    return 0;
}
