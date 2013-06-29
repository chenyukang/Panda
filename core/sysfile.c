#include <types.h>
#include <task.h>
#include <sysfile.h>
#include <inode.h>
#include <file.h>
#include <system.h>
#include <string.h>
#include <tty.h>
#include <fcntl.h>
#include <dirent.h>

s32 do_read(u32 fd, char* buf, u32 cnt) {
    if(fd == 0) {
        while(tty_get_buf(buf) == -1) {
            do_sleep(&tty_dev, NULL);
        }
        return 1;
    } else {
        return file_read(current_task->ofile[fd], buf, cnt);
    }
    return -1;
}

s32 do_write(u32 fd, char* buf, u32 cnt) {
    printk("now in do_write: %c\n", *buf);
    return -1;
}

s32 do_close(int fd) {
    struct file* f = current_task->ofile[fd];
    file_close(f);
    return 0;
}

static s32
fd_alloc(struct file* f) {
    u32 fd;
    for(fd = 1; fd < NOFILE; fd++) {
        if(current_task->ofile[fd] == 0) {
            current_task->ofile[fd] = f;
            return fd;
        }
    }
    return -1;
}

int test_file() {
    int fd = _open("/README.md");
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    if(fd > 0) {
        file_read(current_task->ofile[fd], buf, 300);
        //printk("contents: %s\n", buf);
    }
    else {
        printk("failed to open file: README\n");
    }
    return 0;
}


struct inode* create(char* path, int type) {
    struct inode* ip;
    struct inode* parent;
    char name[NAME_MAX];
    memset(name, 0, sizeof(name));
    if((parent = inode_name_parent(path, name)) == 0) {
        return 0;
    }
    ilock(parent);
    if((ip = ialloc(parent->dev, type)) == 0) {
        PANIC("create: ialloc");
    }
    ilock(ip);
    ip->nlink = 1;
    idrop(ip);
    if(type == S_IFDIR) {
        //add . ..
    }
    idrop(parent);
    return ip;
}

s32 do_open(char* path, int mode, int flag) {
    struct file* f;
    struct inode* ip;
    s32 fd;
    if(mode & O_CREATE) {
        //create file;
        ip = create(path, S_IFREG);
    } else {
        ip = inode_name(path);
    }
    if(ip == 0) return -1;
    ilock(ip);
    if(( f = file_alloc()) == 0 ||
       (fd = fd_alloc(f)) < 0) {
        if(f) file_close(f);
    }
    f->type = FD_INODE;
    f->ip   = ip;
    f->offset = 0;
    f->readable = (mode & O_WRONLY) ? 0 : 1;
    f->writeable = (mode & O_WRONLY) || (mode & O_RDWR);
    idrop(ip);
    return fd;
}

s32 _open(char* name) {
    struct file* f;
    struct inode* ip;
    int fd;

    ip = inode_name(name);
    if(ip == 0)
        return -1;
    
    if((f=file_alloc()) == 0 ||
       (fd = fd_alloc(f)) < 0) {
        if(f)
            file_close(f);
        return -1;
    }
    f->type = FD_INODE;
    f->ip   = ip;
    f->offset  = 0;
    f->readable = 1;
    f->writeable = 0;
    return fd;
}

