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
static struct file* fd_get(u32 fd);

static struct file* fd_get(u32 fd) {
    if(fd >= NOFILE)
        return 0;
    return current->ofile[fd];
}

s32 do_read(u32 fd, char* buf, u32 cnt) {
    u32 r;
    struct file* f;
    if(fd == 0) {
        tty_dev.read_need = cnt;
        while((r = tty_get_buf(buf, cnt)) == -1) {
            do_sleep(&tty_dev, NULL);
        }
        tty_dev.read_need = 0;
        return r;
    } else {
        //read from file
        f = fd_get(fd);
        if(f == 0)
            return -1;
        return file_read(f, buf, cnt);
    }
    return -1;
}

s32 do_write(u32 fd, char* buf, u32 cnt) {
    struct file* f;
    if(fd == 1 || fd == 2) {
        screen_write(buf, cnt);
        return cnt;
    } else {
        //write to file
        f = fd_get(fd);
        if(f == 0)
            return -1;
        return file_write(f, buf, cnt);
    }
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
        return 0;
    }
    f->type = FD_INODE;
    f->ip   = ip;
    f->offset = 0;
    f->readable = (mode & O_WRONLY) ? 0 : 1;
    f->writable = (mode & O_WRONLY) || (mode & O_RDWR);
    if((mode & O_TRUNC) && S_ISREG(ip->type))
        itrunc(ip);
    iunlock(ip);
    return fd;
}

s32 do_close(int fd) {
    struct file* f;
    if(fd < 0 || fd >= NOFILE)
        return -1;
    f = current->ofile[fd];
    if(f == 0)
        return -1;
    current->ofile[fd] = 0;
    file_close(f);
    return 0;
}

static s32 fd_alloc(struct file* f) {
    u32 fd;
    for(fd = 3; fd < NOFILE; fd++) {
        if(current->ofile[fd] == 0) {
            current->ofile[fd] = f;
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
        if((ip->type & S_IFMT) == (type & S_IFMT)) {
            iunlock(ip);
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
    return istat(ip, stat);
}

s32 do_getcwd(char* buf) {
    strcpy(buf, current->cwd_path);
    return 0;
}

s32 do_chdir(char* path) {
    struct inode* ip = inode_name(path);
    if(ip == 0)
        return 0;
    struct task* cu = current;
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
        strcat(cu->cwd_path, path);
    }
    return 0;
}
