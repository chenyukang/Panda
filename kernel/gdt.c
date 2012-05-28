
// @Name   : gdt.c 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-03 21:36:57
//
// @Brief  :

#include <system.h>
#include <string.h>

/* Defines a GDT entry  */
/*   http://wiki.osdev.org/Global_Descriptor_Table */
/*   this page is a good refence */
struct gdt_entry
{
    unsigned short limit;
    unsigned short base_low;
    unsigned char  base_middle;
    unsigned char  access;
    unsigned char  granularity;
    unsigned char  base_high;
} __attribute__((packed));

struct gdt_ptr
{
    unsigned short limit;
    unsigned int   base;
} __attribute__((packed));

struct gdt_entry gdt[3];
struct gdt_ptr gp; //also in start.asm

extern void gdt_flush();


/* Setup a descriptor in the Global Descriptor Table */
void gdt_set_entry(int num, unsigned long base, unsigned long limit,
                  unsigned char access, unsigned char gran) {
    /* Setup the descriptor base address */
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    /* Setup the descriptor limits */
    gdt[num].limit = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);

    /* Finally, set up the granularity and access flags */
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

void gdt_init(void) {
    puts("gdt_init ...\n");
    
    /* Setup the GDT pointer and limit */
    gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gp.base = (unsigned int)&gdt;

    /* Our NULL descriptor */
    gdt_set_entry(0, 0, 0, 0, 0);

    /* The second entry is our Code Segment. The base address
    *  is 0, the limit is 4GBytes, it uses 4KByte granularity,
    *  uses 32-bit opcodes, and is a Code Segment descriptor.
    */
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /* The third entry is our Data Segment. It's EXACTLY the
    *  same as our code segment, but the descriptor type in
    *  this entry's access byte says it's a Data Segment */
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    /* Flush out the old GDT and install the new changes! */
    gdt_flush();  // asm volatile( "lgdt %0" :: "m"(gp));
}
