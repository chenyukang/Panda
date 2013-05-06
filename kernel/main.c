
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
#include <exec.h>
#include <syscall.h>

u32 init_esp_start;

extern char __kimg_end__;

void init_user();

int init = 0;

void kmain(u32 init_stack) {
    init_esp_start = init_stack;
    init_video();
    puts_color_str("Booting Panda OS ...\n", 0x0B);
    puts_color_str("Welcome ...\n", 0x0E);

    cli();
    time_init();
    gdt_init();
    idt_init();
    syscall_init();
    timer_init();
    kb_init();
    mm_init();
    buf_init();
    file_init();
    init_inodes();
    init_ide();
    init_tasks();

    struct task* t = spawn(init_user);
    kassert(t);
    t->stat = RUNNABLE;
    sti();
    init = 0;
    while(1) {
        if(!init) {
            printk("kernel running ...\n");
            init = 1;
        }
        sti();
        sched();
    }
}


void init_user() {
    test_file();
    do_exec("/init", NULL);
    while(1) {
        ;
    }
}


