/*******************************************************************************
 *
 *      @name   : blk.c
 *
 *      @author : Yukang Chen (moorekang@gmail.com)
 *      @date   : 2012-05-28 23:19:23
 *
 *      @brief  : ref xv6
 *
 *******************************************************************************/

#include <system.h>
#include <string.h>
#include <blk.h>

void readsb(u32 dev, struct superblock* sb) {
    //read the 1 sector, which is super block
    struct buf* bp = buf_read(dev, 1);
    memmove(sb, bp->b_data, sizeof(struct superblock));
    buf_release(bp);
}

void blk_zero(u32 dev, u32 bn) {
    struct buf* bp = buf_read(dev, bn);
    memset(bp->b_data, 0, BSIZE);
    buf_release(bp);
}

void blk_free(u32 dev, u32 bn) {
    struct buf *bp;
    struct superblock sb;
    int bi, m;

    readsb(dev, &sb);
    bp = buf_read(dev, BBLOCK(bn, sb.ninodes));
    bi = bn % BPB;
    m = 1 << (bi % 8);
    if((bp->b_data[bi/8] & m) == 0)
        PANIC("freeing free block");
    bp->b_data[bi/8] &= ~m;
    buf_release(bp);
}

u32  blk_alloc(u32 dev) {
    u32 b, bit, mark;
    struct buf* bp;
    struct superblock sb;
    readsb(dev, &sb);
    for(b = 0; b < sb.size; b += BPB) {
        bp = buf_read(dev, BBLOCK(b, sb.ninodes));
        for( bit = 0; bit < BPB && b+bit < sb.size; bit++) {
            mark = 1 << (bit % 8);
            if((bp->b_data[bit/8] & mark) == 0) { //a free block found
                bp->b_data[bit/8] |= mark;
                buf_release(bp);
                blk_zero(dev, b+bit);
                return b + bit; //return block number
            }
        }
        buf_release(bp);
    }
    PANIC("alloc_block: out of blocks");
    return -1;
}
