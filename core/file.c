#include <system.h>
#include <string.h>
#include <fs.h>
#include <blk.h>
#include <file.h>

struct devsw devsw[NDEV];

struct file file_buf[NFILE];

void file_init(void) {
    memset(file_buf, 0, sizeof(file_buf));
}

//Allocate a file structure
struct file* file_alloc(void) {
    struct file* f;
    for(f = file_buf; f < file_buf + NFILE; f++) {
        if(f->ref == 0) {
            kassert(f->type == FD_NONE);
            f->ref = 1;
            return f;
        }
    }
    kassert(0);
    return 0;
}

struct file* file_dup(struct file* f) {
    if(f->ref < 1)
        PANIC("file_dup: try to dup a invalid file");
    f->ref++;
    return f;
}

void file_close(struct file* f) {
    if(f->ref < 1)
        PANIC("file_close: try to close a invalid file");
    if(--f->ref > 0){
        return;
    }
    struct file back = *f;
    f->ref = 0;
    f->type = FD_NONE;

    if(back.type == FD_INODE) {
        idrop(back.ip);
    }
}

int file_stat(struct file* f, struct stat* st) {
    if(f->type == FD_INODE) {
        istat(f->ip, st);
        return 0;
    }
    return -1;
}

int file_read(struct file* f, char* addr, int n) {
    int r;
    if(f->readable == 0){
        return -1;
    }
    if(f->type == FD_INODE) {
        ilock(f->ip);
        if((r = readi(f->ip, addr, f->offset, n)) > 0)
            f->offset += r;
        iunlock(f->ip);
        return r;
    }
    return -1;
}

int file_write(struct file* f, char* addr, int n) {
    int r;
    if(f->writeable == 0)
        return -1;

    if(f->type == FD_INODE) {
        r = writei(f->ip, addr, f->offset, n);
        if( r == n ) {
            f->offset += r;
            return r;
        }
    }
    return -1;
}
