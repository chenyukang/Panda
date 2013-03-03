
#include <inode.h>
#include <string.h>
#include <blk.h>

struct inode icache[NINODE];

static struct inode* iget(u32 dev, u32 num);

void init_inodes() {
    memset(icache, 0, sizeof(icache));
}

           
void stati(struct inode* ip, struct stat* st) {
    st->dev = ip->dev;
    st->inum = ip->inum;
    st->type = ip->type;
    st->nlink = ip->nlink;
    st->size = ip->size;
}



struct inode* ialloc(u32 dev, s16 type) {
    int inum;
    struct buf* bp;
    struct dinode* dip;
    struct superblock sb;
    
    readsb(dev, &sb);
    for(inum = 1; inum < sb.ninodes; inum++) {
        bp = buf_read(dev, IBLOCK(inum));
        dip = (struct dinode*)(bp->b_data) + inum % IPB;
        if(dip->type == 0) {
            memset(dip, 0, sizeof(*dip));
            dip->type = type;
            buf_release(bp);
            return iget(dev, inum);
        }
        buf_release(bp);
    }
    PANIC("ialloc: no inodes");
}

void iupdate(struct inode* ip) {
    struct buf* bp;
    struct dinode* dip;

    bp = buf_read(ip->dev, IBLOCK(ip->inum));
    dip = (struct dinode*)(bp->b_data) + ip->inum%IPB;
    dip->type  = ip->type;
    dip->major = ip->major;
    dip->minor = ip->minor;
    dip->nlink = ip->nlink;
    dip->size  = ip->size;
    memmove(dip->addrs, ip->addrs, sizeof(ip->addrs));
    buf_release(bp);
}

struct inode*
idup(struct inode* ip) {
    ip->ref_cnt++;
    return ip;
}

static struct inode*
iget(u32 dev, u32 inum) {
    struct inode* ip;
    struct inode* empty; //first empty slot 
    
    empty = 0;
    for(ip = &icache[0]; ip < &icache[NINODE]; ip++) {
        if(ip->ref_cnt > 0 && (ip->dev == dev) && ip->inum == inum) {
            return idup(ip);
        }
        if(empty == 0 && ip->ref_cnt == 0)
            empty = ip;
    }

    if(empty == 0)
        PANIC("iget: no inodes");
    ip = empty;
    ip->dev   = dev;
    ip->inum  = inum;
    ip->ref_cnt   = 1;
    ip->flags = 0;
    return ip;
}

void iunlock(struct inode* ip) {
    if(ip == 0 || !(ip->flags & I_BUSY) || ip->ref_cnt < 1)
        PANIC("iunlock: invalid inode");
    ip->flags &= ~I_BUSY;
}

void idrop(struct inode* ip) {
    if(ip->ref_cnt == 1 && (ip->flags & I_VALID) && ip->nlink == 0) {
        if(ip->flags & I_BUSY)
            PANIC("idrop busy");
        ip->flags |= I_BUSY;
//        itrunc(ip);
        ip->type = 0;
        iupdate(ip);
        ip->flags = 0;
    }
    ip->ref_cnt--;
}

void i_unlock_drop(struct inode* ip) {
    iunlock(ip);
    idrop(ip);
}


#if 0
static u32
bmap(struct inode* ip, u32 bn) {
    u32 addr;
    u32* extend;
    struct buf* bp;

    if(bn < NDIRECT) {
        if((addr = ip->addrs[bn]) == 0)
            ip->addrs[bn] = addr = blk_alloc(ip->dev);
        return addr;
    }

    bn -= NDIRECT;
    if(bn < NINDIRECT) {
        //load indirect block, allocating if necessary.
        if((addr = ip->addrs[NINDIRECT]) == 0)
            ip->addrs[NDIRECT] = addr = blk_alloc(ip->dev);
        bp = buf_read(ip->dev, addr);
        extend = (u32*)bp->b_data;
        if((addr = extend[bn]) == 0) {
            extend[bn] = addr = blk_alloc(ip->dev);
        }
        buf_release(bp);
        return addr;
    } else {
        PANIC("bmap: out of range");
        return -1;
    }
    
}
#endif

int readi(struct inode* ip, char* addr, u32 off, u32 n) {
    return 0;
}

int writei(struct inode* ip, char* addr, u32 off, u32 n) {
    return 0;
}

