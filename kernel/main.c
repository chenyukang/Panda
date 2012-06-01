
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

void kmain()
{
    init_video();
    puts("booting Panda OS ...\n");
    gdt_init();
    idt_init();
    timer_init(1);
    kb_init();

    long mem_end = (1<<20) + ((*(unsigned short*)0x90002)<<10);
    printk("mem_end:%x\n", mem_end);
    page_init( mem_end ); 

    init_hd((void*)0x90080);

#if 0
    int k;
    for(k=0; k<10; k++) {
        u32 *ptr = (u32*)kmalloc(sizeof(u32)*100);
        printk("ptr: %x\n", ptr);
        *ptr = 0xA;
        printk("value: %x\n", (*ptr));
        char* cp = (char*)kmalloc(sizeof(char)*100);
        *cp = 'c';
        printk("value: %c\n", (*cp));
        kfree(cp);
    }
#endif
    
    
    int initial = 1;
    while(1){
        if(initial) {
            puts("kernel running\n");
            initial = 0;
        }
    }
}
