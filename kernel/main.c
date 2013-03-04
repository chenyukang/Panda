
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

u32 init_esp_start;

extern char __kimg_end__;
extern u32 end_addr;

void do_init_job();

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
    init_ide();
    init_multi_task();
    sti();

#if 0
    int* p = (int*)(end_addr + 0x20);
    int v = *p;
    printk("addr: %x %d\n", p, v);
#endif

//    spawn((void*)do_init_job);
#if 0
    int pid = fork();
    if(pid > 0) {
        printk("parent\n");
    } else {
        printk("child\n");
    }
#endif

    int init = 0;
    while(1) {
        if(!init) {
            init = 1;
            printk("kernel running ...\n");
        }
    }
}

void do_init_job() {
//    while(1) {
        printk("job: B\n");
//    }
}
