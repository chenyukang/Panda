
// @Name   : timer.c 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-05 22:46:52
//
// @Brief  :

#include <system.h>
#include <string.h>
#include <asm.h>
#include <time.h>
#include <task.h>

static u32 ticks = 0;
extern task_t* current_task;

u32 get_sys_ticks(void) {
    return ticks;
}

static void timer_callback(void) {
    ticks++;
    if (ticks%18 == 0){
        cli();
        update_time();
        sti();
        print_time_local();
    }

    if(ticks%20 == 0){
        //printk("begin switch\n");
        switch_task();
        //printk("end switch\n");
    }
}

void timer_init() {
    puts("timer init ...\n");
    // Firstly, register our timer callback.
    irq_install_handler(32, (isq_t)(&timer_callback));

    // The value we send to the PIT is the value to divide it's input clock
    // (1193180 Hz) by, to get our required frequency. Important to note is
    // that the divisor must be small enough to fit into 16-bits.
    u32 divisor = 1193180 / 100;

    // Send the command byte.
    outb(0x43, 0x36);

    // Divisor has to be sent byte-wise, so split here into upper/lower bytes.
    u8 l = (u8)(divisor & 0xFF);
    u8 h = (u8)(divisor>>8);

    // Send the frequency divisor.
    outb(0x40, l);
    outb(0x40, h);
}


/* This will continuously loop until the given time has
*  been reached */
void timer_wait(int wait) {
    unsigned long end_tic;

    end_tic = ticks + wait;
    while(ticks < end_tic);
}
