
// @Name   : main.c 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-03 17:33:50
//
// @Brief  : The kernel entry point

#include <asm.h>
#include <cpu.h>
#include <screen.h>
#include <string.h>
#include <page.h>
#include <kheap.h>
#include <hd.h>
#include <test.h>
#include <task.h>

void kmain()
{
    init_video();
    puts("booting Panda OS ...\n");
    gdt_init();
    idt_init();
    timer_init(1);
    kb_init();

    long mem_end = (1<<20) + ((*(unsigned short*)0x90002)<<10);
    page_init( mem_end ); 

    init_hd((void*)0x90080);
    
    init_task();

    int k;
    for(k=0; k<4; k++) {
        u32 *ptr = (u32*)kmalloc(sizeof(u32)*100);
        *ptr = 0xA;
        kfree(ptr);
        char* cp = (char*)kmalloc(sizeof(char)*100);
        *cp = 'c';
        printk("ptr1:%x ptr2:%x \n",ptr,  cp);
        kfree(cp);
    }

    
    int initial = 1;
    while(1){
        if(initial) {
            puts("kernel running\n");
            initial = 0;
        }
    }
}
