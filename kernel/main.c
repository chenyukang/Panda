
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

u32 init_esp_start;

extern char __kimg_end__;
extern u32 end_addr;
extern task_t* current_task;

void do_init_Ajob();
void do_init_Bjob();
void do_init_Cjob();

void init_user();

int init = 0;

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
    sti();

    //detect_cpu();
    spawn(init_user);

#if 0
    int* p = (int*)0x08000010;
    printk("value: %d\n", *p);
    *p = 1;
    printk("value: %d\n", *p);
    kassert(0);
#endif
    
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
    printk("finished exec\n");
    while(1) {
        ;
    }
}

static int astep = 0;
void do_init_Ajob() {
    printk("job A: %d\n", astep++);
    while(1) {
        //printk("job A: %d\n", astep++);
        astep++;
    }
}

static int bstep = 0;
void do_init_Bjob() {
    printk("job B: %d\n", bstep++);
    while(1) {
        //printk("job B: %d\n", bstep++);
        bstep++;
   }
}

static int cstep = 0;
void do_init_Cjob() {
    printk("job C: %d\n", cstep++);
    while(1) {
        cstep++;
        //printk("job C: %d\n", cstep++);
    }
}
