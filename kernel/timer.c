
// @Name   : timer.c 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-05 22:46:52
//
// @Brief  :

#include <system.h>
#include <screen.h>
#include <asm.h>

#if 0
/* This will keep track of how many ticks that the system
*  has been running for */
int timer_ticks = 0;

/* Handles the timer. In this case, it's very simple: We
*  increment the 'timer_ticks' variable every time the
*  timer fires. By default, the timer fires 18.222 times
*  per second. Why 18.222Hz? Some engineer at IBM must've
*  been smoking something funky */
void timer_handler(struct registers_t* regs)
{
    /* Increment our 'tick count' */
    timer_ticks++;

    /* Every 18 clocks (approximately 1 second), we will
    *  display a message on the screen */
    if (timer_ticks % 18 == 0)
    {
        puts("One second has passed\n");
    }
}

/* This will continuously loop until the given time has
*  been reached */
void timer_wait(int ticks)
{
    unsigned long eticks;

    eticks = timer_ticks + ticks;
    while(timer_ticks < eticks);
}

/* Sets up the system clock by installing the timer handler
*  into IRQ0 */
void timer_install()
{
    /* Installs 'timer_handler' to IRQ0 */
    irq_install_handler(0, timer_handler);
}

#endif

static u32 ticks = 0;
static u32 seconds = 0;
static void timer_callback(void)
{
    ticks++;
    /* Every 18 clocks (approximately 1 second), we will
     *  display a message on the screen */
    if (ticks % 70 == 0)
    {
        //puts("now:");
        //printk_int(seconds++);
        seconds++;
        //puts("\n");
    }
}

void timer_init(u32 frequency)
{
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
