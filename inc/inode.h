#ifndef _INODE_H__
#define _INODE_H__

#include <types.h>

#define NDIRECT 12
#define NINODE       50  // maximum number of active i-nodes

#define I_BUSY  0x1
#define I_VALID 0x2

struct inode {
    u32 dev;  //device number
    u32 inum; //inode number
    s32 ref_cnt;
    s32 flags;  //I_BUSY, I_VALID

    s16 type; //copy of disk inode
    s16 major;
    s16 minor;
    s16 nlink;
    u32 size;
    u32 addrs[NDIRECT + 1];
};

#define T_DIR  1   // Directory
#define T_FILE 2   // File
#define T_DEV  3   // Device

struct stat {
    s16 type;  // Type of file
    s32 dev;     // File system's disk device
    u32 inum;    // Inode number
    s16 nlink; // Number of links to file
    u32 size;   // Size of file in bytes
};


void            init_inodes();

struct inode*   idup(struct inode*);
void            iinit(void);
void            ilock(struct inode*);
void            idrop(struct inode*);
void            iput(struct inode*);
void            iunlock(struct inode*);
void            iunlockput(struct inode*);
void            iupdate(struct inode*);
u32             bmap(struct inode*, u32);

s32             dir_link(struct inode*, char*, u32);
int             namecmp(const char*, const char*);
struct inode*   namei(char*);
struct inode*   nameiparent(char*, char*);
int             readi(struct inode*, char*, u32, u32);
void            stati(struct inode*, struct stat*);
int             writei(struct inode*, char*, u32, u32);

struct inode* inode_name(char* path);
struct inode* inode_name_parent(char* path, char* name);

#endif
