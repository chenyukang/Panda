#ifndef _SYS_CALL_H
#define _SYS_CALL_H

#include <system.h>
#include <uname.h>
#include <time.h>
#include <stat.h>
#include <asm.h>

// System call numbers
#define NSYSC 64
#define OPENERR 0x8ffffff

extern int errno;

void do_syscall(struct registers_t* regs);
void sysc_init();

enum {
    NR_setup,
    NR_fork,
    NR_getpid,
    NR_getppid,
    NR_kexit,
    NR_close,
    NR_sleep,
    NR_uname,
    NR_time,
    NR_chdir,
    NR_sbrk,
    NR_wait,
    NR_exec,
    NR_getcwd,
    NR_stat,
    NR_open,
    NR_write,
    NR_read,
    NR_halt,
    NR_procs
};

static inline _SYS0(int, halt);
static inline _SYS0(int, fork);
static inline _SYS0(int, getpid);
static inline _SYS0(int, getppid);
static inline _SYS1(int, kexit,  int);
static inline _SYS1(int, close,  int);
static inline _SYS1(int, sleep,  int);
static inline _SYS1(int, uname,  struct utsname*);
static inline _SYS1(int, time,   struct tm*);
static inline _SYS1(int, chdir,  char*);
static inline _SYS1(int, sbrk,   u32);
static inline _SYS2(int, wait,   int,   int*);
static inline _SYS2(int, exec ,  char*, char**);
static inline _SYS2(int, getcwd, char*, int);
static inline _SYS2(int, procs,  char*, int);
static inline _SYS2(int, stat,   char*, struct stat*);
static inline _SYS3(int, open,   char*, int, int);
static inline _SYS3(int, write,  int,   char*, int);
static inline _SYS3(int, read,   int,   void*, int);

#endif
