#ifndef _DIRENT_H__
#define _DIRENT_H__

#ifdef NAME_MAX
#undef NAME_MAX
#endif
#define NAME_MAX 30

struct dirent {
    unsigned short d_ino;
    char d_name[NAME_MAX];
};

#endif
