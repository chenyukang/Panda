
// @Name   : timer.c 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-05 22:46:52
//
// @Brief  :

#include <system.h>
#include <string.h>
#include <asm.h>

static u32 ticks = 0;
static u32 seconds = 0;
static void timer_callback(void)
{
    ticks++;
    /* Every 18 clocks (approximately 1 second), we will
     *  display a message on the screen */
    if (ticks % 70 == 0)
    {
//        puts("now:");
//        printk_int(seconds++);
        seconds++;
//        puts("\n");
    }
}

void timer_init(u32 frequency)
{
    puts("timer init ...\n");
    // Firstly, register our timer callback.
    irq_install_handler(32, (isq_t)(&timer_callback));

    // The value we send to the PIT is the value to divide it's input clock
    // (1193180 Hz) by, to get our required frequency. Important to note is
    // that the divisor must be small enough to fit into 16-bits.
    u32 divisor = 1193180 / frequency;

    // Send the command byte.
    outb(0x43, 0x36);

    // Divisor has to be sent byte-wise, so split here into upper/lower bytes.
    u8 l = (u8)(divisor & 0xFF);
    u8 h = (u8)((divisor>>8) & 0xFF );

    // Send the frequency divisor.
    outb(0x40, l);
    outb(0x40, h);
}


/* This will continuously loop until the given time has
*  been reached */
void timer_wait(int wait)
{
    unsigned long eticks;

    eticks = ticks + wait;
    while(ticks < eticks);
}
