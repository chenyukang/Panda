#include <syscall.h>

static void fail(char* name, int status) {
    printf("ostest: %s failed: %d\n", name, status);
    while(1)
        ;
}

static void run(char* name, char* path) {
    int status = -1;

    if(fork() == 0) {
        exec(path, NULL);
        fail(name, OPENERR);
    }
    wait(-1, &status);
    if(status != 0)
        fail(name, status);
}

int main(int argc, char* argv[]) {
    printf("ostest: start\n");
    run("badfd", "/home/badfd");
    printf("ostest: ok\n");
    halt();
    return 0;
}
