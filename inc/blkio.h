#ifndef _BLK_IO_H__
#define _BLK_IO_H__


#include "buf.h"

struct blk_cache {
    struct buf [NBUF];
    struct buf head;
};

#endif
