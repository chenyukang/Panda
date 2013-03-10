
// @Name   : main.c 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-03 17:33:50
//
// @Brief  : The kernel main entry point

#include <asm.h>
#include <cpu.h>
#include <screen.h>
#include <string.h>
#include <page.h>
#include <kheap.h>
#include <hd.h>
#include <test.h>
#include <task.h>
#include <time.h>
#include <file.h>
#include <sysfile.h>


u32 init_esp_start;

extern char __kimg_end__;
extern u32 end_addr;
extern task_t* current_task;

void do_init_job();
void do_init_Aojb();

void kmain(u32 init_stack) {

    init_esp_start = init_stack;
    init_video();
    puts_color_str("Booting Panda OS ...\n", 0x0B);

    puts_color_str("Welcome ...\n", 0x0A);

    cli();
    time_init();
    gdt_init();
    idt_init();
    timer_init();
    kb_init();
    mm_init();
    buf_init();
    file_init();
    init_inodes();
    init_ide();
    init_multi_task();
    printk("proc name: %s\n", current_task->name);
    sti();

    spawn((void*)do_init_job);
    spawn((void*)do_init_Aojb);
    
    int init = 0;
    printk("proc name: %s\n", current_task->name);

    extern char _binary_initcode_start[], _binary_initcode_size[];

    printk("init_start: %x init_size: %x\n",
           _binary_initcode_start, _binary_initcode_size);
           
    while(1) {
        if(!init) {
            printk("kernel running ...\n");
            init = 1;
        }
        sti();
        sched();
    }
}

static int bstep = 0;
void do_init_job() {
    printk("job B: %d\n", bstep++);
    while(1) {
        //printk("job B: %d\n", bstep++);
    }
}

static int astep = 0;
void do_init_Aojb() {
    printk("job A: %d\n", astep++);
    while(1) {
        //printk("job A: %d\n", astep++);
    }
}
