#include <types.h>
#include <task.h>
#include <sysfile.h>
#include <inode.h>
#include <file.h>
#include <string.h>

extern task_t* current_task;

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
    printk("test_file \n");
    int fd = _open("/README");
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    if(fd > 0) {
        printk("open file: README\n");
        file_read(current_task->ofile[fd], buf, 300);
        printk("contents: %s\n", buf);
    }
    else {
        printk("failed to open file: README\n");
    }
    return 0;
}

int _open(char* name) {
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

