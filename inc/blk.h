
/*******************************************************************************
 *
 *      @name   : blk.h
 *
 *      @author : Yukang Chen (moorekang@gmail.com)
 *      @date   : 2012-05-28 23:12:47
 *
 *      @brief  :
 *
 *******************************************************************************/

#if !defined(BLK_H)
#define BLK_H


#include "types.h"
#include "buf.h"

#define ROOTDEV       1  // device number of file system root disk
#define ROOTINO 1  // root i-number
#define BSIZE 512  // block size

// File system super block
struct superblock {
  u32 size;         // Size of file system image (blocks)
  u32 nblocks;      // Number of data blocks
  u32 ninodes;      // Number of inodes.
  u32 nlog;         // Number of log blocks
};

#define NDIRECT 12
#define NINDIRECT (BSIZE / sizeof(u32))
#define MAXFILE (NDIRECT + NINDIRECT)

// On-disk inode structure
struct dinode {
  short type;           // File type
  short major;          // Major device number (T_DEV only)
  short minor;          // Minor device number (T_DEV only)
  short nlink;          // Number of links to inode in file system
  u32 size;            // Size of file (bytes)
  u32 addrs[NDIRECT+1];   // Data block addresses
};

// Inodes per block.
#define IPB           (BSIZE / sizeof(struct dinode))

// Block containing inode i
#define IBLOCK(i)     ((i) / IPB + 2)

// Bitmap bits per block
#define BPB           (BSIZE*8)

// Block containing bit for block b
#define BBLOCK(b, ninodes) (b/BPB + (ninodes)/IPB + 3)

void readsb(u32 dev, struct superblock* sb);
void blk_zero(u32 dev, u32 bn);
void blk_free(u32 dev, u32 bn);
u32  blk_alloc(u32 dev);

#endif

