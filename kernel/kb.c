
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

static int shift_state = 0 ;

/* Handles the keyboard interrupt */ 
void keyboard_handler(void)
{
    unsigned char scancode;
    scancode = inb(0x60); //read the data buffer

//    printk_hex((int)scancode);
    if(scancode == 0x2a || scancode == 0x36)
        shift_state = 1;
    else if(scancode == 0xaa || scancode == 0xb6 )
        shift_state = 0;
    
    /* If the top bit of the byte we read from the keyboard is
    *  set, that means that a key has just been released */
    if (scancode & 0x80)
    {
        /* You can use this one to see if the user released the
        *  shift, alt, or control keys... */
        
    }
    else
    {
        /* Here, a key was just pressed. Please note that if you
        *  hold a key down, you will get repeated key press
        *  interrupts. */
        putch(shift_state?
              kbdus_upper[scancode]:kbdus[scancode]);
    }
}

void kb_init()
{
    puts("kb init...\n");
    irq_install_handler(33, (isq_t)(&keyboard_handler));
}

