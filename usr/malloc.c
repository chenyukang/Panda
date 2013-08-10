
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
    char buf[1024];
    printf("size: %d\n", sizeof(int));
    int* p = (int*)malloc(sizeof(int));
    printf("pointer: %x\n",(unsigned int)p);
    printf("value  : %d\n",*p);
    *p = 1;
    printf("value  : %d\n", *p);
    *p = 2;
    printf("value  : %d\n", *p);
    int i;
    for(i=0; i<10; i++) {
        p = (int*)malloc(sizeof(int));
        printf("pointer: %x value: %d\n", (u32)p, (u32)(*p));
        free(p);
    }

    memset(buf, 0, sizeof(buf));
    procs(buf, 1024);
    printf("buf: %s\n", buf);
    return 0;
}
