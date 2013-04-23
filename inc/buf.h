#ifndef _BUF_H__
#define _BUF_H__

#define NBUF 124

#include "types.h"
#include "spinlock.h"

#define NBUF    124
#define PBLK    512         /* physical block size */
#define BLK     1024        /* logical block size */

struct buf {
    u32             b_flag;
    struct buf*     b_next;
    struct buf*     b_prev;
    struct buf*     b_qnext;
    s16             b_dev;
    u32             b_sector;
    char            b_data[512];
    int             b_error;
};


#define B_BUSY      0x1
#define B_VALID     0x2
#define B_DIRTY     0x4


#define B_WRITE     0x0
#define B_READ      0x1

#define B_DONE      0x2
#define B_ERROR     0x4
#define B_WANTED    0x10
#define B_ASYNC     0x40

struct buf_cache {
    struct spinlock lock;
    struct buf buf[NBUF];
    struct buf head;
};

void buf_init();
struct buf* buf_read(u32 dev, u32 sector);
void buf_write(struct buf* bp);
void buf_release(struct buf* bp);

#endif
