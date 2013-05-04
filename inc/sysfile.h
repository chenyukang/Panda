#ifndef _SYSFILE_H_
#define _SYSFILE_H_
#include <types.h>

int do_read(u32 fd, char* buf, u32 cnt);

int test_file();
int _open(char* name);

#endif
