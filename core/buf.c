#include "buf.h"
#include "task.h"
#include "string.h"
#include "hd.h"
#include "task.h"
#include "asm.h"

struct buf_cache bcache;

void buf_init() {
    init_lock(&bcache.lock, "buf_lock");

    struct buf* bp;
    bcache.head.b_prev = &bcache.head;
    bcache.head.b_next = &bcache.head;
    /* now link all buffers with link list */
    for(bp = bcache.buf; bp < bcache.buf+NBUF; bp++) {
        bp->b_next = bcache.head.b_next;
        bp->b_prev = &bcache.head;
        bp->b_dev  = -1;
        bcache.head.b_next->b_prev = bp;
        bcache.head.b_next = bp;
    }
}


static struct buf* buf_get(u32 dev, u32 sector) {
    struct buf* bp;

    acquire_lock(&bcache.lock);
loop:
    for(bp = bcache.head.b_next; bp != &bcache.head; bp = bp->b_next) {
        if(bp->b_dev == dev && bp->b_sector == sector) {
            if(! (bp->b_flag & B_BUSY) ) {
                bp->b_flag |= B_BUSY;
                release_lock(&bcache.lock);
                return bp;
            }
            do_sleep(bp, &bcache.lock);
            goto loop;
        }
    }

    for(bp = bcache.head.b_prev; bp != &bcache.head; bp = bp->b_prev) {
        if((bp->b_flag & B_BUSY) == 0 && (bp->b_flag & B_DIRTY) == 0) {
            bp->b_dev = dev;
            bp->b_sector = sector;
            bp->b_flag = B_BUSY;
            release_lock(&bcache.lock);
            return bp;
        }
    }

    PANIC("buf_get: no buf for use");
}


struct buf* buf_read(u32 dev, u32 sector) {
    struct buf* bp;
    bp = buf_get(dev, sector);
    kassert(bp);
    if(!(bp->b_flag & B_VALID)) {
        hd_rw(bp);
    }
    return bp;
}


void buf_write(struct buf* bp) {
    kassert(bp);
    if((bp->b_flag & B_BUSY) == 0)
        PANIC("buf_write: error buf");
    cli();
    bp->b_flag |= B_DIRTY;
    sti();
    hd_rw(bp);
}


/* buf_release, release leads to some confusion,
   release means to put this buf at the header of this cache,
   this is visited most recently */

void buf_release(struct buf* bp) {

    if((bp->b_flag & B_BUSY) == 0)
        PANIC("buf_release: error buf");

    acquire_lock(&bcache.lock);

    bp->b_next->b_prev = bp->b_prev;
    bp->b_prev->b_next = bp->b_next;
    bp->b_next = bcache.head.b_next;
    bp->b_prev = &bcache.head;
    bcache.head.b_next->b_prev = bp;
    bcache.head.b_next = bp;
    bp->b_flag &= ~B_BUSY;
    cli();
    do_wakeup(bp);
    sti();
    release_lock(&bcache.lock);
}
