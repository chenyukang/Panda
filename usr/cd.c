/*******************************************************************************
 *
 *      cd.c
 *
 *      @brief
 *
 *      @author   Yukang Chen  @date  2013-08-04 18:00:01
 *
 *******************************************************************************/
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stat.h>
#include <dirent.h>
#include <syscall.h>

int main(int argc, char* argv[]) {
    char path[1024];
    struct stat s;
    memset(&s, 0, sizeof(s));
    memset(path, 0, sizeof(path));
    
    if(argc != 2) {
        printf("usage: cd <directory>\n");
        return -1;
    }

    strcpy(path,  argv[1]);
    if(stat(path, &s) < 0) {
        printf("no such dir: %s\n", path);
        return -1;
    }
    if(!S_ISDIR(s.st_mode)) {
        printf("not a dir: %s\n", path);
        return -1;
    }
    memset(path, 0, sizeof(path));
    getcwd(path, 1024);
    strcat(path, argv[1]);
    if(chdir(path) != 0) {
        printf("chdir error: %s\n", path);
        return -1;
    }
    return 0;
}





















