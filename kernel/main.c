
// @Name   : main.c 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-03 17:33:50
//
// @Brief  :

#include <asm.h>
#include <screen.h>

void kmain()
{
    init_video();
    puts("booting Panda OS ...\n");
    puts("gdt_init ...\n");
    gdt_init();
    puts("idt_init ...\n");
    idt_init();
    puts("timer init ...\n");
    //asm volatile ("int $0x3");
    //asm volatile ("int $0x4");
    asm volatile("sti");
    timer_init(1);
    puts("kb init...\n");
    kb_init();
//    asm volatile ("int $0xF");
    int initial = 1;
    while(1){
        //initial = 0;
        if(initial) {
            puts("running\n");
            initial = 0;
        }
    }
    safe_halt();
}
