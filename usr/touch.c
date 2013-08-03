#include <fcntl.h>
#include <string.h>
#include <syscall.h>
#include <stat.h>
#include <dirent.h>

int main(int argc, char* argv[] ) {
    if(argc == 1) {
        printf("usage: touch <filename>\n");
        return -1;
    }
    return 0;
}
