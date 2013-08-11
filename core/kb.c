
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

void kb_handler(void) {
    u8 ch;
    u8 code = inb(0x60); //read the data buffer
    if(code == 0x2a || code == 0x36)
        shift_state = 1;
    else if(code == 0xaa || code == 0xb6 )
        shift_state = 0;

    /* If the top bit of the byte we read from the keyboard is
    *  set, that means that a key has just been released */
    if (code & 0x80) {
        /* You can use this one to see if the user released the
        *  shift, alt, or control keys... */
    } else {
        /* Here, a key was just pressed. Please note that if you
        *  hold a key down, you will get repeated key press
        *  interrupts. */
        if(code == SHIFT_L || code == SHIFT_R) return;
        ch = shift_state? kbdus_upper[code] : kbdus[code];
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
