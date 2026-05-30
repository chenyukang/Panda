#include <string.h>
#include <syscall.h>

int main(int argc, char* argv[]) {
    char buf[4];

    memset(buf, 0, sizeof(buf));
    if(read(99, buf, 1) != -1) {
        printf("badfd: read failed\n");
        return -1;
    }
    if(write(99, buf, 1) != -1) {
        printf("badfd: write failed\n");
        return -1;
    }
    if(close(99) != -1) {
        printf("badfd: close high fd failed\n");
        return -1;
    }
    if(close(-1) != -1) {
        printf("badfd: close negative fd failed\n");
        return -1;
    }

    printf("badfd: ok\n");
    return 0;
}
