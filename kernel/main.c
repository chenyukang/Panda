
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

void kmain()
{
    init_video();
    puts("booting Panda OS ...\n");
    gdt_init();
    idt_init();
    timer_init(1);
    kb_init();
    page_init(0x1000000);//16 MB
    int initial = 1;
#if 0    
    u32 *ptr = (u32*)0xA0000000;
    u32 do_page_fault = *ptr;
    kassert(do_page_fault != 0);
#endif 
    while(1){
        if(initial) {
            puts("running\n");
            initial = 0;
        }
    }
}
