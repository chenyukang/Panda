/*******************************************************************************
 *
 *      @name   : inode.c
 *
 *      @author : Yukang Chen (moorekang@gmail.com)
 *      @date   : 2012-05-28 23:19:23
 *
 *      @brief  : ref xv6
 *
 *******************************************************************************/

#include <inode.h>
#include <string.h>
#include <blk.h>
#include <buf.h>
#include <task.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

struct inode icache[NINODE];

static struct inode* iget(u32 dev, u32 num);

void inode_init() {
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
    if(ip == 0 ||  ip->ref_cnt < 1)
        PANIC("iunlock: invalid inode");
    ip->flags &= ~I_BUSY;
}

void itrunc(struct inode* ip) {
    int i;
    struct buf* bp;

    for(i=0; i<NDIRECT; i++) {
        if(ip->addrs[i]) {
            blk_free(ip->dev, ip->addrs[i]);
            ip->addrs[i] = 0;
        }
    }

    //have extra contents
    if(ip->addrs[NDIRECT] ) {
        bp = buf_read(ip->dev, ip->addrs[NDIRECT]);
        u32* addr = (u32*)bp->b_data;
        for(i=0; i<NINDIRECT; i++) {
            if(addr[i])
                blk_free(ip->dev, addr[i]);
        }
        buf_release(bp);
        blk_free(ip->dev, addr[NDIRECT]);
        ip->addrs[NDIRECT] = 0;
    }
    kassert(0);
    ip->size = 0;
    iupdate(ip);
}


void ilock(struct inode* ip) {
    struct buf* bp;
    struct dinode* dip;
    if(ip->flags & I_BUSY) {
        PANIC("ilock: busy");
    }
    if(!(ip->flags & I_VALID)) {
        bp = buf_read(ip->dev, IBLOCK(ip->inum));
        dip = (struct dinode*)bp->b_data + ip->inum%IPB;
        ip->type = dip->type;
        ip->major = dip->major;
        ip->minor = dip->minor;
        ip->nlink = dip->nlink;
        ip->size = dip->size;
        memmove(ip->addrs, dip->addrs, sizeof(ip->addrs));
        buf_release(bp);
        ip->flags |= I_VALID;
        if(ip->type == 0)
            PANIC("ilock: no type");
    }
}

void idrop(struct inode* ip) {
    if(ip->ref_cnt == 1 && (ip->flags & I_VALID) && ip->nlink == 0) {
        if(ip->flags & I_BUSY)
            PANIC("idrop busy");
        ip->flags |= I_BUSY;
        itrunc(ip);
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


static u32
bmap(struct inode* ip, u32 bn) {
    u32 addr;
    u32* extend;
    struct buf* bp;

    //printk("bmap: %d\n", bn);
    if(bn < NDIRECT) {
        if((addr = ip->addrs[bn]) == 0)
            ip->addrs[bn] = addr = blk_alloc(ip->dev);
        //printk("return: %d\n", addr);
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

int readi(struct inode* ip, char* addr, u32 off, u32 n) {
    u32 total, done;
    struct buf* bp;
    
    if(ip->type == T_DEV) {
        return -1;
    }
    if(off > ip->size || off + n < off) {
        return -1;
    }
    if(off + n > ip->size)
        n = ip->size - off;
    //printk("ip->size: %x %d need read: %d\n", (u32)addr, ip->size, n);
    for(total=0; total<n; total+=done, off+=done, addr+=done) {
        //printk("reading ... \n");
        bp = buf_read(ip->dev, bmap(ip, off/BSIZE));
        done = min(n-total, BSIZE - off%BSIZE);
        memmove(addr, bp->b_data + off%BSIZE, done);
        buf_release(bp);
    }
    return n;
}

int writei(struct inode* ip, char* addr, u32 off, u32 n) {
    u32 total, done;
    struct buf* bp;

    if(ip->type == T_DEV) {
        return -1;
    }

    if(off > ip->size || off + n < off) 
        return -1;

    /* if size is larger than the biggest file size*/
    if(off + n > MAXFILE * BSIZE)
        return -1;

    for(total=0; total<n; total+=done, off+=done, addr+=done) {
        bp = buf_read(ip->dev, bmap(ip, off/BSIZE));
        done = min(n-total, BSIZE - off/BSIZE);
        memmove(bp->b_data + off%BSIZE, addr, done);
        buf_release(bp);
    }
    if(n>0 && off > ip->size) {
        ip->size = off;
        iupdate(ip);
    }
    return n;
}


s32 namecmp(const char* s, const char* t) {
    return strncmp(s, t, DIRSIZ); //DIRSIZE is the max length of name
}


struct inode*
dir_lookup(struct inode* dp, char* name, u32* poff) {
    u32 off;
    struct dirent dire;
    if(dp->type != T_DIR)
        PANIC("dir_lookup: error type of inode ");
    for(off=0; off<dp->size; off+=sizeof(dire)) {
        if(readi(dp, (char*)&dire, off, sizeof(dire)) != sizeof(dire))
            PANIC("dir_lookup: error readi");
        if(dire.inum == 0) continue;
        if(namecmp(name, dire.name) == 0) {
            if(poff)
                *poff = off;
            return iget(dp->dev, dire.inum);
        }
    }
    return 0;
}

s32 dir_link(struct inode* dp, char* name, u32 inum) {
    u32 off;
    struct dirent* dire;
    struct inode*  ip;

    if((ip = dir_lookup(dp, name, 0)) != 0) {
        idrop(ip);
        return -1;
    }

    //look for an empty dirent
    for(off=0; off<dp->size; off+=sizeof(dire)) {
        if(readi(dp, (char*)&dire, off, sizeof(dire)) != sizeof(dire))
            PANIC("dir_link: error readi");
        if(dire->inum == 0)
            break;
    }
    strncmp(dire->name, name, DIRSIZ);
    dire->inum = inum;
    if(writei(dp, (char*)&dire, off, sizeof(dire)) != sizeof(dire))
        PANIC("dir_link: error writei");
    return 0;
}

            
static char*
_skip(char* path, char* name) {
    const char* s;
    int len;
    while( *path == '/') path++;
    if(*path == 0) return 0;
    s = path;
    while( *path != '/' && *path != 0 )
        path++;
    len = path - s;
    len = len > DIRSIZ ? DIRSIZ : len;
    memmove(name, s, len);
    name[len] = 0;
    while(*path == '/')
        path++;
    return path;
}

static struct inode*
inode_namex(char* path, char* name, u32 parent) {
    struct inode* ip;
    struct inode* next;

    if(*path == '/')
        ip = iget(ROOTDEV, ROOTINO);
    else
        ip = idup(current_task->cwd);

    //kassert(ip); //fix me
    if(ip == 0)
        return 0;
    while((path = _skip(path, name)) != 0) {
        ilock(ip);
        if(ip->type != T_DIR) {
            i_unlock_drop(ip);
            return 0;
        }
        if(parent && *path == '\0') {
            i_unlock_drop(ip);
            return ip;
        }
        if((next = dir_lookup(ip, name, 0)) == 0) {
            i_unlock_drop(ip);
            return 0;
        }
        i_unlock_drop(ip);
        ip = next;
    }
    if(parent) {
        idrop(ip);
        return 0;
    }
    return ip;
}

        
struct inode* inode_name(char* path) {
    char name[DIRSIZ];
    memset(name, 0, sizeof(name));
    return inode_namex(path, name, 0);
}

struct inode* inode_name_parent(char* path, char* name) {
    return inode_namex(path, name, 1);
}

