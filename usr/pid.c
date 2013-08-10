
/*******************************************************************************
 *
 *      pid.c
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
    int pid = getpid();
    int ppid = getppid();
    printf("current pid: %d \nparent  pid: %d\n", pid, ppid);
#if 0
    int* p = (int*)malloc(sizeof(int));
    printf("pointer: %x\n",(unsigned int)p);
    printf("value  : %d\n",*p);
    *p = 1;
    printf("value  : %d\n", *p);
#endif
    return 0;
}
