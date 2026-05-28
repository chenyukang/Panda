
// @Name   : kd.c
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-07 16:14:09
//
// @Brief  :

#include <asm.h>
#include <system.h>
#include <screen.h>
#include <kb.h>
#include <string.h>
#include <tty.h>

static int shift_state = 0 ;
static int extend_state = 0;

#define KEY_LEFT  0x11
#define KEY_RIGHT 0x12
#define KEY_UP    0x13
#define KEY_DOWN  0x14

void kb_handler(void) {
    u8 ch;
    u8 code = inb(0x60); //read the data buffer
    if(code == 0xe0) {
        extend_state = 1;
        return;
    }
    if(code == 0x2a || code == 0x36)
        shift_state = 1;
    else if(code == 0xaa || code == 0xb6 )
        shift_state = 0;

    /* If the top bit of the byte we read from the keyboard is
    *  set, that means that a key has just been released */
    if (code & 0x80) {
        /* You can use this one to see if the user released the
        *  shift, alt, or control keys... */
        extend_state = 0;
    } else {
        /* Here, a key was just pressed. Please note that if you
        *  hold a key down, you will get repeated key press
        *  interrupts. */
        if(extend_state) {
            extend_state = 0;
            if(code == 0x48)
                ch = KEY_UP;
            else if(code == 0x50)
                ch = KEY_DOWN;
            else if(code == 0x4b)
                ch = KEY_LEFT;
            else if(code == 0x4d)
                ch = KEY_RIGHT;
            else
                ch = 0;
            if(ch)
                tty_ch(ch);
            return;
        }
        if(code == SHIFT_L || code == SHIFT_R) return;
        ch = shift_state? kbdus_upper[code] : kbdus[code];
        if(ch)
            tty_ch(ch);
    }
}

void kb_init() {
    puts("[kb]   .... ");
    irq_enable(1);
    irq_install(33, (isq_t)(&kb_handler));
    tty_clear();
    done();
}
