#ifndef _STAT_H__
#define _STAT_H__


#define T_DIR  1   // Directory
#define T_FILE 2   // File
#define T_DEV  3   // Device
#define u32 unsigned int

struct stat {
  short type;  // Type of file
  u32 dev;     // File system's disk device
  u32 ino;     // Inode number
  short nlink; // Number of links to file
  u32 size;    // Size of file in bytes
};

#endif
