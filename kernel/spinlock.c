
#include <spinlock.h>
#include <system.h>
#include <string.h>
#include <asm.h>

void init_lock(struct spinlock* lk, char* name) {
    lk->name = name;
    lk->locked = 0;
}


int holding(struct spinlock* lk) {
    return lk->locked == 1;
}

/* as we just support one cpu, so this idle wait is uncessary,
   anyway keep it */
void acquire_lock(struct spinlock* lk) {
    printk("got lock: %s\n", lk->name);
    if(holding(lk)){
        printk("acquire_lock: %s\n", lk->name);
        PANIC("acquire_lock");
    }
    cli();
    while(xchg(&lk->locked, 1) != 0)
        ;
}


void release_lock(struct spinlock* lk) {
    printk("release lock: %s\n", lk->name);
    if(!holding(lk)){
        PANIC("try release un-acquired lock");
    }
    xchg(&lk->locked, 0);
    sti();
}

