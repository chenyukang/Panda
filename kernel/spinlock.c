
#include <spinlock.h>
#include <system.h>
#include <string.h>
#include <asm.h>

void init_lock(struct spinlock* lk, char* name) {
    lk->name = name;
    lk->locked = 0;
}


/* as PandaOs is just support one cpu, so this idle wait is uncessary,
   anyway keep it */
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

