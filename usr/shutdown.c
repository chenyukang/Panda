
/*******************************************************************************
 *
 *      shutdown.c
 *
 *      @brief
 *
 *      @author   Yukang Chen  @date  2013-08-10 16:14:58
 *
 *      COPYRIGHT (C) 2006~2012, Nextop INC., ALL RIGHTS RESERVED.
 *
 *******************************************************************************/

#include <string.h>
#include <syscall.h>

int main() {
    printf("power off now ! \n");
    halt();
    return 0;
}
