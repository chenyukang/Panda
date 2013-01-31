#ifndef _BLK_IO_H__
#define _BLK_IO_H__

#include "spinlock.h"
#include "buf.h"

#define NBUF 124

struct blk_cache {
    struct spinlock lock;
    struct buf buf[NBUF];
    struct buf head;
};

void blk_init();
struct buf* blk_read(u32 dev, u32 sector);
void blk_write(struct buf* bp);
void blk_release(struct buf* bp);
#endif
