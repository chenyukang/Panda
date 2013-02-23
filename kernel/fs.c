#include <fs.h>
#include <blkio.h>
#include <string.h>

static void 
zero_blk(int dev, int sector) {
    struct buf* bp = blk_read(dev, sector);
    memset(bp->b_data, 0, BSIZE);
    //write log 
    blk_release(bp);
}

void read_super_blk(u32 dev, struct superblock* sb) {
    struct buf* bp = blk_read(dev, 1); //read the 1 sector, which is super block
    memmove(sb, bp->b_data, sizeof(struct superblock));
    blk_release(bp);
}

u32 alloc_block(u32 dev) {
    u32 b, bit, mark;
    struct buf* bp;
    struct superblock sb;
    read_super_blk(dev, &sb);
    for(b = 0; b < sb.size; b += BPB) {
        bp = blk_read(dev, BBLOCK(b, sb.ninodes));
        for( bit = 0; bit < BPB && b+bit < sb.size; bit++) {
            mark = 1 << (bit % 8);
            if((bp->b_data[bit/8] & mark) == 0) { //a free block found
                bp->b_data[bit/8] |= mark;
                blk_release(bp);
                zero_blk(dev, b+bit);
                return b + bit; //return block number
            }
        }
    }
    PANIC("alloc_block: out of blocks");
    return -1;
}

void free_block(u32 dev, u32 blkno) {
    struct buf* bp;
    struct superblock sb;
    u32 bit, mask;
    read_super_blk(dev, &sb);
    bp = blk_read(dev, BBLOCK(blkno, sb.ninodes));
    bit = blkno % BPB;
    mask = 1 << (blkno % 8);
    if((bp->b_data[bit/8] & mask) == 0) {
        PANIC("try to free a free block");
    }
    bp->b_data[bit/8] &= ~mask;
    blk_release(bp);
}



