
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

void kmain(u32 init_stack)
{
    init_esp_start = init_stack;
    
    init_video();
    puts_color_str("Booting Panda OS ...\n", 0x0B);
    
    time_init();    
    cli();
    gdt_init();
    idt_init();
    timer_init(50);
    kb_init();
    page_init();
    init_hd((void*)0x90080);
    init_task();
    sti();
    
//    int ret = fork();
    //printk("fork ret: %d\n", ret);
    


    while(1){
        //printk("runing: %s\n", get_current_name());
    }

}
