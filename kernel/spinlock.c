
#include <spinlock.h>
#include <system.h>
#include <string.h>
#include <asm.h>


void initlock(struct spinlock* lk, char* name) {
    lk->name = name;
    lk->locked = 0;
}


void acquire_lock(struct spinlock* lk) {
    cli();
    while(xchg(&lk->locked, 1) != 0)
        ;
}


void release_lock(struct spinlock* lk) {
    if(!lk->locked) {
        PANIC("try release un-acquired lock");
    }
    xchg(&lk->locked, 0);
    cli();
}

