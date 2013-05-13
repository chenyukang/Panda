#ifndef _TIMER_H__
#define _TIMER_H__

#include <spinlock.h>
#include <types.h>

struct sys_timer {
    struct spinlock lock;
    u32 ticks;
    u32 seconds;
};

struct sys_timer timer;

u32 get_seconds();
u32 get_sys_ticks();

#endif

