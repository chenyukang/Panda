#ifndef _TTY_H__
#define _TTY_H__
#include <types.h>
#include <spinlock.h>

#define TTY_BUF_LEN 1024

struct tty_t {
    u32 head, tail;
    u32 count;
    u32 flush;
    u8  buf[TTY_BUF_LEN];
};

struct tty_t tty_dev;

void tty_clear();
u32 tty_ch(u8 c);
u32 tty_push(u8 c);
u32 tty_pop();
u32 tty_get_buf(char* buf, u32 need);

#endif











