#include <tty.h>
#include <task.h>
#include <string.h>

inline static u32 prev(u32 i) {
    if( i == 0 ) return TTY_BUF_LEN - 1;
    else return i-1;
}

inline static u32 next(u32 i) {
    if( i == TTY_BUF_LEN -1 ) return 0;
    else return i + 1;
}

void tty_clear() {
    memset(tty_dev.buf, 0, TTY_BUF_LEN);
    tty_dev.head  = 0;
    tty_dev.tail  = 0;
    tty_dev.count = 0;
}

u32 tty_ch(u8 c) {
    putch(c);
    if( c == 0x08 ) {
        //delete
        return tty_pop();
    }

    tty_push(c);
    if(c == 0x0A) {
        //newline
        do_wakeup(&tty_dev);
    }
    return -1;
}

u32 tty_push(u8 c) {
    u32 tail = tty_dev.tail;
    if(tty_dev.count == TTY_BUF_LEN)
        return -1;
    else {
        tty_dev.buf[tail] = c;
        tty_dev.tail = next(tail);
        tty_dev.count++;
    }
    return 0;
}

u32 tty_pop() {
    u32 tail = tty_dev.tail;
    if(tty_dev.count == 0)
        return -1;
    else {
        tty_dev.tail = prev(tail);
        tty_dev.count--;
    }
    return 0;
}

u32 tty_get_buf(char* buf, u32 need) {
    u32 i, h, head, count;
    char* p = buf;

    count = tty_dev.count;
    head  = tty_dev.head;
    if(count == 0) {
        return -1;
    }
    for(i=head, h = 0; h < need && i< head + count;
        i++, h++) {
        *p++ = tty_dev.buf[tty_dev.head];
        tty_dev.count--;
        tty_dev.head = next(tty_dev.head);
    }
    if(tty_dev.count == 0) {
        tty_clear();
    }
    do_wakeup(&tty_dev);
    return need > count ? count : need;
}
