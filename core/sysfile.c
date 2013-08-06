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
#include <asm.h>

struct inode* create(char* path, int type);
static s32 fd_alloc(struct file* f);

s32 do_read(u32 fd, char* buf, u32 cnt) {
    if(fd == 0) {
        while(tty_get_buf(buf, cnt) == -1) {
            do_sleep(&tty_dev, NULL);
        }
        return 1;
    } else {
        //read from file
        return file_read(current_task->ofile[fd], buf, cnt);
    }
    return -1;
}

s32 do_write(u32 fd, char* buf, u32 cnt) {
    if(fd == 0 || fd == 1) {
        int k;
        for(k=0; k<cnt; k++) {
            putch(buf[k]);
        }
    } else {
        //write to file
        return file_write(current_task->ofile[fd], buf, cnt);
    }

    return -1;
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
       (fd = fd_alloc(f)) < 0 ) {
        if(f)
            file_close(f);
    }
    f->type = FD_INODE;
    f->ip   = ip;
    f->offset = 0;
    f->readable = (mode & O_WRONLY) ? 0 : 1;
    f->writeable = (mode & O_WRONLY) || (mode & O_RDWR);
    idrop(ip);
    return fd;
}

s32 do_close(int fd) {
    struct file* f = current_task->ofile[fd];
    file_close(f);
    return 0;
}

static s32 fd_alloc(struct file* f) {
    u32 fd;
    for(fd = 2; fd < NOFILE; fd++) {
        if(current_task->ofile[fd] == 0) {
            current_task->ofile[fd] = f;
            return fd;
        }
    }
    return -1;
}

struct inode* create(char* path, int type) {
    struct inode* dp;
    struct inode* ip;
    char name[NAME_MAX];
    u32 off;

    memset(name, 0, sizeof(name));
    if((dp = inode_name_parent(path, name)) == 0) {
        kassert(0);
        return 0;
    }

    ilock(dp);
    //check present
    if((ip = dir_lookup(dp, name, &off)) != 0) {
        i_unlock_drop(dp);
        ilock(ip);
        if(ip->type == type) {
            return ip;
        }
        i_unlock_drop(ip);
        return 0;
    }
    if((ip = ialloc(dp->dev, type)) == 0) {
        PANIC("create: ialloc");
    }
    ilock(ip);
    ip->nlink = 1;
    ip->major = 0;
    ip->minor = 0;
    iupdate(ip);

    if(type == S_IFDIR) {
        //add . ..
    }
    if(dir_link(dp, name, ip->inum) < 0)
        PANIC("create: dir_link");
    i_unlock_drop(dp);
    return ip;
}

s32 do_stat(char* path, struct stat* stat) {
    struct inode* ip = inode_name(path);
    if(ip == 0)
        return -1;
    return stati(ip, stat);
}

s32 do_getcwd(char* buf) {
    strcpy(buf, current_task->cwd_path);
    return 0;
}

s32 do_chdir(char* path) {
    struct inode* ip = inode_name(path);
    if(ip == 0)
        return 0;
    struct task* cu = current_task;
    idrop(cu->cwd);
    cu->cwd = ip;
    if(strncmp(path, "..", 2) == 0) { //change to parent,
        u32 i = strlen(cu->cwd_path);
        do {
            cu->cwd_path[i--] = 0;
        } while(cu->cwd_path[i] != '/');
    }
    else if(strncmp(path, ".", 1) == 0) {
        //pass;
    }
    else {
        //memset(cu->cwd_path, 0, sizeof(cu->cwd_path));
        //strcpy(cu->cwd_path, path);
        strcat(cu->cwd_path, path);
    }
    return 0;
}

static int test_write() {
    int fd = do_open("/tmp", O_CREATE|O_RDWR, 0);
    char buf[1024];
    char* text = "hello world";
    memset(buf, 0, sizeof(buf));
    strncpy(buf, text, strlen(text));
    if(fd > 0) {
        file_write(current_task->ofile[fd], buf, strlen(text));
    } else {
        printk("failed to create file: tmp\n");
        return 0;
    }
    do_close(fd);
    return 1;
}

static int test_read() {
    int fd = do_open("/tmp", O_RDONLY, 0);
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    if(fd > 0) {
        file_read(current_task->ofile[fd], buf, 300);
        printk("contents: %s\n", buf);
    }
    else {
        printk("failed to open file: tmp\n");
        return 0;
    }
    do_close(fd);
    return 1;
}

#if 0
static in test_remove() {
    return 0;
}
#endif


int test_file() {
    if(!test_write())
        return 0;

    if(!test_read())
        return 0;
    return 1;
}
