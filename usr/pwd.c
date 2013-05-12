#include <string.h>
#include <syscall.h>

int main() {
    int k;
    char buf[512];
    memset(buf, 0, sizeof(buf));
    getcwd(buf, 512);
    printf("cwd: %s\n", buf);
    return 0;
}
