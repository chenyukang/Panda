
// @Name   : gdt.c 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-03 21:36:57
//
// @Brief  :

#include <system.h>
#include <string.h>
#include <gdt.h>

struct gdt_entry gdt[NR_GDTENTRY];
struct gdt_ptr gp; //also in start.asm

extern void gdt_flush();

/* Setup a descriptor in the Global Descriptor Table */
void gdt_set_entry(int num, unsigned long base, unsigned long limit,
                  u8 access, u8 gran) {
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;
    gdt[num].limit = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

void gdt_init(void) {
    puts("gdt_init ...\n");
    memset(gdt, 0, sizeof(gdt));
    gp.limit = (sizeof(struct gdt_entry) * NR_GDTENTRY) - 1;
    gp.base = (unsigned int)&gdt;

    gdt_set_entry(0, 0, 0, 0, 0);    //Null gdt, this is just needed for some simulator 
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); //code
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF); //data

    gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); //user code
    gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); //user data

    /* flush GDT */
    gdt_flush();  //asm volatile( "lgdt %0" :: "m"(gp));
}
