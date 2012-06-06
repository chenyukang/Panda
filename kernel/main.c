
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

u32 init_esp_start;

void kmain(u32 init_stack)
{
    init_esp_start = init_stack;
    init_video();
    puts_color_str("Booting Panda OS ...\n", 0x0B);

    gdt_init();
    idt_init();
    
    //asm volatile("sti");
    timer_init(50);
    kb_init();
    
    long mem_end = (1<<20) + ((*(unsigned short*)0x90002)<<10);
    page_init( mem_end );
    
    init_hd((void*)0x90080);
    
    init_task();
    printk("init stack: %x\n", init_stack);

    test_all();
#if 0
    int ret = fork();
    if(ret == 0){
        printk("I am child: %d\n", getpid());
    }
    else
        printk("I am parent: %d\n", getpid());
#endif
    while(1){
        printk("now running: %d\n", getpid());
    }
}
