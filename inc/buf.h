
#ifndef _BUF__
#define _BUF__
#include <types.h>

#define NBUF 124
#define PBLK    512         /* physical block size */
#define BLK     1024        /* logical block size */

struct buf {
    u32             b_flag;
    struct buf     *b_next;
    short           b_dev;
    u32             b_blkno;
    char           *b_data;
    int             b_error;
};

#define B_WRITE     0x0
#define B_READ      0x1
#define B_DONE      0x2
#define B_ERROR     0x4
#define B_BUSY      0x8
#define B_WANTED    0x10
#define B_DIRTY     0x20
#define B_ASYNC     0x40


extern struct buf   buff[NBUF];

#endif
