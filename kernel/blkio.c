
#include "spinlock.h"
#include "blkio.h"
#include "task.h"
#include "string.h"
#include "hd.h"
#include "task.h"

struct blk_cache bcache;

void blk_init() {
    init_lock(&bcache.lock, "blk_lock");

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


static struct buf*
blk_get(u32 dev, u32 sector) {
    struct buf* bp;
    acquire_lock(&bcache.lock);

loop:
    for(bp = bcache.head.b_next; bp!=&bcache.head; bp = bp->b_next) {
        if(bp->b_dev == dev && bp->b_blkno == sector) {
            if(! (bp->b_flag & B_BUSY) ) {
                release_lock(&bcache.lock);
                return bp;
            }
        }
        sleep(bp, &bcache.lock);
        goto loop;
    }

    for(bp = bcache.head.b_prev; bp != &bcache.head; bp = bp->b_prev) {
        if((bp->b_flag & B_BUSY) == 0 && (bp->b_flag & B_DIRTY) == 0) {
            bp->b_dev = dev;
            bp->b_blkno = sector;
            bp->b_flag = B_BUSY;
            release_lock(&bcache.lock);
            return bp;
        }
    }

    PANIC("blk_get: no buf for use");
}


struct buf*
blk_read(u32 dev, u32 sector) {
    struct buf* bp;
    bp = blk_get(dev, sector);
    kassert(bp);
    if(!(bp->b_flag & B_VALID)) {
        hd_rw(bp);
    }
    return bp;
}


void* 
blk_write(struct buf* bp) {
    kassert(bp);
    if((bp->b_flag & B_BUSY) == 0)
        PANIC("blk_write: error blk");
    bp->b_flag |= B_DIRTY;
    hd_rw(bp);
}



void blk_release(struct buf* bp) {
    if((bp->b_flag & B_BUSY) == 0)
        PANIC("blk_release: error blk");
    acquire_lock(&bcache.lock);

    bp->b_next->b_prev = bp->b_prev;
    bp->b_prev->b_next = bp->b_next;
    bp->b_next = bcache.head.b_next;
    bp->b_prev = &bcache.head;
    bcache.head.b_next->b_prev = bp;
    bcache.head.b_next = bp;
    bp->b_flag &= ~B_BUSY;
    wakeup(bp);
    release_lock(&bcache.lock);
}

