
// @Name   : main.c
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-03 17:33:50
//
// @Brief  : The kernel main entry point

#include <asm.h>
#include <screen.h>
#include <string.h>
#include <mm.h>
#include <hd.h>
#include <test.h>
#include <task.h>
#include <time.h>
#include <file.h>
#include <sysfile.h>
#include <exec.h>
#include <syscall.h>

u32 init_esp_start;
s32 init = 0;

void init_user();
extern char __kimg_end__;

void kmain(u32 init_stack) {
    init_esp_start = init_stack;
    init_video();
    puts_color_str("Booting Panda OS ...\n", 0x0B);

    cli();
    time_init();
    gdt_init();
    idt_init();
    kb_init();
    mm_init();
    buf_init();
    file_init();
    inode_init();
    ide_init();
    task_init();
    timer_init();
    sysc_init();

    spawn(init_user);
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
    //test_file();
    do_exec("/home/init", NULL);
    while(1) {
        ;
    }
}
