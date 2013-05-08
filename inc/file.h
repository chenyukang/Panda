#ifndef FILE_H__
#define FILE_H__

#include <types.h>
#include <inode.h>

struct file {
    enum { FD_NONE, FD_PIPE, FD_INODE } type;
    int ref;
    char readable;
    char writeable;
    struct inode* ip;
    u32 offset;
};


void file_init(void);

struct file* file_alloc(void);
struct file* file_dup(struct file* f);

void file_close(struct file* f);
int file_stat(struct file* f, struct stat* st) ;
int file_read(struct file* f, char* addr, int n) ;
int file_write(struct file* f, char* addr, int n) ;

#define I_BUSY  0x1
#define I_VALID 0x2

struct devsw {
    int (*read) (struct inode*, char* , int);
    int (*write) (struct inode* , char* , int);
};

extern struct devsw devsw[];

#define CONSOLE 1
#endif


    
