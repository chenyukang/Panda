
/*******************************************************************************
 *
 *      ps.c
 *
 *      @brief
 *
 *      @author   Yukang Chen  @date  2013-08-09 21:45:43
 *
 *      COPYRIGHT (C) 2006~2012, Nextop INC., ALL RIGHTS RESERVED.
 *
 *******************************************************************************/
#include <string.h>
#include <stdlib.h>
#include <syscall.h>

int main() {
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    procs(buf, 1024);
    printf("current task list:\n");
    printf("%s", buf);
    return 0;
}
