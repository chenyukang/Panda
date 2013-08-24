
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
    if(holding(lk)){
        printk("acquire_lock error: %s\n", lk->name);
        kassert(0);
    }
    cli();
    while(xchg(&lk->locked, 1) != 0)
        ;
}

void release_lock(struct spinlock* lk) {
    if(!holding(lk)){
        printk("release_lock error: %s\n", lk->name);
        kassert(0);
    }
    xchg(&lk->locked, 0);
    sti();
}
