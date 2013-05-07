
/*******************************************************************************
 *
 *      UNAME_H
 *
 *      @brief
 *
 *      @author   Yukang Chen  @date  2013-05-07
 *
 *
 *******************************************************************************/

#if !defined(UNAME_H)
#define UNAME_H

#define _UTSNAME_LENGTH 24

struct utsname {
    char sysname[_UTSNAME_LENGTH];
    char release[_UTSNAME_LENGTH];
    char version[_UTSNAME_LENGTH];
    char machine[_UTSNAME_LENGTH];
};

#endif

