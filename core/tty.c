#include <tty.h>
#include <task.h>
#include <string.h>

void tty_clear() {
    memset(tty_dev.buf, 0, TTY_BUF_LEN);
    tty_dev.head  = 0;
    tty_dev.tail  = 0;
    tty_dev.count = 0;
    tty_dev.flush = 0;
}

u32 tty_ch(u8 c) {
    if(c == 0x0A) {
        tty_dev.flush = 1;
        do_wakeup(&tty_dev);
    }
    else if(c == 0x08) {
        return tty_pop();
    }
    else {
        return tty_push(c);
    }
    return -1;
}

u32 tty_push(u8 c) {
    if(tty_dev.count == TTY_BUF_LEN)
        return -1;
    else {
        tty_dev.buf[tty_dev.tail++] = c;
        tty_dev.count++;
    }
    return 0;
}

u32 tty_pop() {
    if(tty_dev.count == 0)
        return -1;
    else {
        tty_dev.buf[--tty_dev.tail] = 0;
        tty_dev.count--;
    }
    return 0;
}
            
u32 tty_get_buf(char* buf) {
    char* p = buf;
    u32 i;
    u32 ret;
    u32 count = tty_dev.count;
    if(count == 0 || tty_dev.flush == 0) {
        return -1;
    }
    for(i=tty_dev.head; i<tty_dev.head+count; i++) {
        *p++ = tty_dev.buf[i%TTY_BUF_LEN];
    }
    ret = count;
    tty_clear();
    do_wakeup(&tty_dev);
    return ret;
}

