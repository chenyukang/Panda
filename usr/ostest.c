#include <fcntl.h>
#include <string.h>
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

static void test_open_existing() {
    char buf[8];
    int fd;

    fd = open("/home/ostest.tmp", O_CREATE | O_RDWR, 0);
    if(fd < 0)
        fail("open create", fd);
    strcpy(buf, "ok");
    if(write(fd, buf, strlen(buf)) != strlen(buf))
        fail("write create", -1);
    close(fd);

    fd = open("/home/ostest.tmp", O_CREATE | O_RDWR, 0);
    if(fd < 0) {
        fail("open existing create", fd);
    }
    close(fd);
}

static void test_write_rodata() {
    char* msg = "ostest: rodata write ok\n";
    int len = strlen(msg);

    if(write(1, msg, len) != len)
        fail("write rodata", -1);
}

int main(int argc, char* argv[]) {
    printf("ostest: start\n");
    run("badfd", "/home/badfd");
    test_open_existing();
    test_write_rodata();
    printf("ostest: ok\n");
    halt();
    return 0;
}
