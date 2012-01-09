
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

void kmain()
{
    init_video();
    puts("booting Panda OS ...\n");
    puts("gdt_init ...\n");
    gdt_init();
    puts("idt_init ...\n");
    idt_init();
    puts("timer init ...\n");
    
    timer_init(1);
    puts("kb init...\n");

    kb_init();

    kassert(1==1);
//    kassert(1==2);

    printk("%s\n", "hello world");
    printk("%c\n", 'y');
    printk("%d\n", 32);
    printk("%d\n", -32);
    printk("value: %d\n", 1<<31);
    printk("most: %d\n", 1<<30);

    asm volatile ("int $0xF");
    asm volatile ("int $0x04");
    asm volatile ("int $0x06");
    int initial = 1;
    while(1){
        //initial = 0;
        if(initial) {
            puts("running\n");
            initial = 0;
        }
    }
}
