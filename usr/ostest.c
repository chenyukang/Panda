#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <syscall.h>

static void fail(char* name, int status) {
    printf("ostest: %s failed: %d\n", name, status);
    while(1)
        ;
}

static int run_status(char* name, char* path) {
    int status = -1;

    if(fork() == 0) {
        exec(path, NULL);
        fail(name, OPENERR);
    }
    wait(-1, &status);
    return status;
}

static void run(char* name, char* path) {
    int status = run_status(name, path);

    if(status != 0)
        fail(name, status);
}

static void run_fail(char* name, char* path) {
    int status = run_status(name, path);

    if(status == 0)
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

static void test_stdio_file() {
    FILE* fp;
    char buf[8];
    int fd;
    int n;

    fp = fopen("/home/stdio.tmp", "w");
    if(fp == NULL)
        fail("fopen write", -1);
    if(fprintf(fp, "abc") != 3)
        fail("fprintf", -1);
    if(fclose(fp) != 0)
        fail("fclose", -1);

    memset(buf, 0, sizeof(buf));
    fd = open("/home/stdio.tmp", O_RDONLY, 0);
    if(fd < 0)
        fail("open stdio tmp", fd);
    n = read(fd, buf, 3);
    close(fd);
    if(n != 3 || strncmp(buf, "abc", 3) != 0)
        fail("read stdio tmp", n);
}

int main(int argc, char* argv[]) {
    printf("ostest: start\n");
    run("badfd", "/home/badfd");
    run_fail("fault", "/home/fault");
    test_open_existing();
    test_write_rodata();
    test_stdio_file();
    printf("ostest: ok\n");
    halt();
    return 0;
}
