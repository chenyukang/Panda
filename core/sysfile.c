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
    u32 r;
    if(fd == 0) {
        while((r = tty_get_buf(buf, cnt)) == -1) {
            do_sleep(&tty_dev, NULL);
        }
        return r;
    } else {
        //read from file
        return file_read(current->ofile[fd], buf, cnt);
    }
    return -1;
}

s32 do_write(u32 fd, char* buf, u32 cnt) {
    if(fd == 1 || fd == 2) {
        int k;
        for(k=0; k<cnt; k++) {
            putch(buf[k]);
        }
    } else {
        //write to file
        return file_write(current->ofile[fd], buf, cnt);
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
        return 0;
    }
    f->ref++;
    f->type = FD_INODE;
    f->ip   = ip;
    f->offset = 0;
    f->readable = (mode & O_WRONLY) ? 0 : 1;
    f->writeable = (mode & O_WRONLY) || (mode & O_RDWR);
    iunlock(ip);
    return fd;
}

s32 do_close(int fd) {
    struct file* f = current->ofile[fd];
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
