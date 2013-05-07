
/*******************************************************************************
 *
 *      uname.c
 *
 *      @brief
 *
 *      @author   Yukang Chen  @date  2013-05-07 
 *
 *
 *******************************************************************************/

#include <uname.h>
#include <syscall.h>

int main() {
    struct utsname name;
    uname(&name);
    printf("%s %s\n", name.sysname, name.version);
    return 0;
}

