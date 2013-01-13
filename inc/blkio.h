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

#endif
