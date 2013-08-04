#ifndef _STAT_H__
#define _STAT_H__

/*
 * st_mode flags
 */
/* this is also used in user space, 
   protect from redefine */
#ifndef S_IFMT
#define         S_IFMT  0170000 /* type of file ，文件类型掩码*/
#define         S_IFREG 0100000 /* regular file*/
#define         S_IFBLK 0060000 /* block special */
#define         S_IFDIR 0040000 /* directory */
#define         S_IFCHR 0020000 /* character special */
#define         S_IFIFO 0010000 /* fifo */
#define         S_IFNAM 0050000 /* special named file */
#define         S_IFLNK 0120000 /* symbolic link */

#define         S_ISREG(m)      (((m) & S_IFMT) == S_IFREG)
#define         S_ISBLK(m)      (((m) & S_IFMT) == S_IFBLK)
#define         S_ISDIR(m)      (((m) & S_IFMT) == S_IFDIR)
#define         S_ISCHR(m)      (((m) & S_IFMT) == S_IFCHR)
#define         S_ISFIFO(m)     (((m) & S_IFMT) == S_IFIFO)
#define         S_ISNAM(m)      (((m) & S_IFMT) == S_IFNAM)
#define         S_ISLNK(m)      (((m) & S_IFMT) == S_IFLNK)


struct stat {
    short st_mode;
    short st_dev;
    short st_nlink;
    short st_uid;
    short st_ugid;
    unsigned int st_ino;
    unsigned int st_size;
};

#endif
#endif
