#ifndef _TIMER_H__
#define _TIMER_H__

#include <spinlock.h>
#include <types.h>

struct sys_timer {
    struct spinlock lock;
    u32 ticks;
};

struct sys_timer timer;

#endif

