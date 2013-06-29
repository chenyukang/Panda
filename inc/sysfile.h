#ifndef _SYSFILE_H_
#define _SYSFILE_H_
#include <types.h>

s32 do_read(u32 fd, char* buf, u32 cnt);
s32 do_write(u32 fd, char* buf, u32 cnt);
s32 do_open(char* path, int flag, int mode);
s32 do_close(int fd);

int test_file();
int _open(char* name);

#endif
