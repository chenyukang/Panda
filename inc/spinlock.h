#ifndef _SPIN_LOCK_H__
#define _SPIN_LOCK_H__

#include "types.h"

struct spinlock {
    u32   locked;
    char* name;
};

void init_lock(struct spinlock* lk, char* name);
void acquire_lock(struct spinlock* lk);
void release_lock(struct spinlock* lk);
    
#endif
