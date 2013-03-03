#ifndef FILE_H__
#define FILE_H__

#include <types.h>
#include <inode.h>

struct file {
    enum { FD_NONE, FD_PIPE, FD_INODE } f_type;
    int ref;
    char readable;
    char writeable;
    struct inode* ip;
    u32 offset;
};


#define I_BUSY  0x1
#define I_VALID 0x2

struct devsw {
    int (*read) (struct inode*, char* , int);
    int (*write) (struct inode* , char* , int);
};

extern struct devsw devsw[];

#define CONSOLE 1
#endif


    
