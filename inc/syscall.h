
#ifndef _SYS_CALL_H
#define _SYS_CALL_H

#include <system.h>
#include <uname.h>
#include <time.h>

// System call numbers
#define NSYSC       64

extern int errno;

void do_syscall(struct registers_t* tf);
void sysc_init();

#define _SYS0(T0, FN)                           \
    T0 FN(){                                    \
        register int r;                         \
        asm volatile(                           \
            "int $0x80"                         \
            :"=a"(r),                           \
             "=b"(errno)                        \
            :"a"(NR_##FN)                       \
        );                                      \
        return r;                               \
    }

#define _SYS1(T0, FN, T1)                       \
    T0 FN(T1 p1){                               \
        register int r;                         \
        asm volatile(                           \
            "int $0x80"                         \
            :"=a"(r),                           \
             "=b"(errno)                        \
            :"a"(NR_##FN),                      \
             "b"((int)p1)                       \
        );                                      \
        if (r<0){                               \
            errno = -r;                         \
            return -1;                          \
        }                                       \
        return r;                               \
    }

#define _SYS2(T0, FN, T1, T2)                   \
    T0 FN(T1 p1, T2 p2){                        \
        register int r;                         \
        asm volatile(                           \
            "int $0x80"                         \
            :"=a"(r),                           \
             "=b"(errno)                        \
            :"a"(NR_##FN),                      \
             "b"((int)p1),                      \
             "c"((int)p2)                       \
        );                                      \
        return r;                               \
    }

#define _SYS3(T0, FN, T1, T2, T3)               \
    T0 FN(T1 p1, T2 p2, T3 p3){                 \
        register int r;                         \
        asm volatile(                           \
            "int $0x80"                         \
            :"=a"(r),                           \
             "=b"(errno)                        \
            :"a"(NR_##FN),                      \
             "b"((int)p1),                      \
             "c"((int)p2),                      \
             "d"((int)p3)                       \
        );                                      \
        return r;                               \
    }


enum {
    NR_setup,
    NR_fork,
    NR_exec,
    NR_read,
    NR_exitc,
    NR_wait,
    NR_write,
    NR_uname,
    NR_open,
    NR_time
};
    
static inline _SYS1(int, write, char);
static inline _SYS1(int, exitc,  int);
static inline _SYS1(int, uname, struct utsname*);
static inline _SYS1(int, time, struct tm*);
static inline _SYS0(int, fork);
static inline _SYS2(int, wait, int, int*);
static inline _SYS2(int, exec, char*, char**);
static inline _SYS3(int, open, char*, int, int);
static inline _SYS3(int, read, int, char*, int);

#endif
