
/*******************************************************************************
 *
 *      stdio.c
 *
 *      @brief
 *
 *      @author   Yukang Chen  @date  2013-08-04 11:12:54
 *
 *      COPYRIGHT (C) 2006~2012, Nextop INC., ALL RIGHTS RESERVED.
 *
 *******************************************************************************/

#include <stdio.h>
#include <syscall.h>

char getchar() {
    int cnt;
    char buf[1024];
    while((cnt = read(0, buf, 1) > 0)) {
            return buf[0];
    }
    return 0;
}
