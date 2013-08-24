#ifndef _INODE_H__
#define _INODE_H__

#include <types.h>
#include <stat.h>

#define NDIRECT 12
#define NINODE  50  //maximum number of active i-nodes

#define I_BUSY  0x1
#define I_VALID 0x2

struct inode {
    u32 dev;     //device number
    u32 inum;    //inode number
    s32 ref_cnt;
    s32 flags;   //I_BUSY, I_VALID

    s16 type;    //copy of disk inode
    s16 major;
    s16 minor;
    s16 nlink;
    u32 size;
    u32 addrs[NDIRECT + 1];
};

void            inode_init();
void            ilock(struct inode*);
void            idrop(struct inode*);
void            iput(struct inode*);
void            iunlock(struct inode*);
void            iunlockput(struct inode*);
void            iupdate(struct inode*);

s32             dir_link(struct inode*, char*, u32);

struct inode*   idup(struct inode*);
struct inode*   ialloc(u32 dev, s16 type);
struct inode*   dir_lookup(struct inode*, char*, u32*);

s32             istat(struct inode*, struct stat*);
s32             readi(struct inode*, char*, u32, u32);
s32             writei(struct inode*, char*, u32, u32);

struct inode*   inode_name(char* path);
struct inode*   inode_name_parent(char* path, char* name);
void            i_unlock_drop(struct inode* ip);

#endif
