
// @Name   : idt.c
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-03 22:48:21
//
// @Brief  : Interrupt Gate Descriptor related

#include <system.h>
#include <screen.h>
#include <string.h>
#include <task.h>
#include <asm.h>
#include <gdt.h>

/*
Interrupt Gate Descriptor

63                       48|47           40|39               32
+------------------------------------------------------------
|                         | |D|D| | | | | | | | |
|   BASE OFFSET (16-31)   |P|P|P|0|1|1|1|0|0|0|0| RESERVED
|                         | |L|L| | | | | | | | |
=============================================================
                           |                                |
   SEGMENT SELECTOR        |   BASE  OFFSET (0-15)          |
                           |                                |
------------------------------------------------------------+
31                       16|15                               0

         - bits   0 to 15 : base offset low
         - bits 16 to 31  : segment selector
         - bits 32 to 37  : reserved
         - bits 37 to 39  : 0
         - bits 40 to 47  : flags/type
         - bits 48 to 63  : base offset high
*/

/* Defines an IDT entry */
struct idt_entry {
    unsigned short base_lo;  //16 bits
    unsigned short selector; //16 bits
    unsigned char  zero;     //8 bits
    unsigned char  flags;    //8 bits
    unsigned short base_hi;  //16 bits
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit;
    unsigned int   base;
} __attribute__((packed));

/* The IDT table , used for interrupt */
struct idt_entry idt[256];
struct idt_ptr   idtp;
isq_t irq_routines[256];

/* These extern directives let us access the addresses of our ASM ISR handlers. */
#define disr(idx) \
    extern void isr##idx();

#define dirq(idx) \
    extern void irq##idx();

disr(0);  disr(1);  disr(2);  disr(3);
disr(4);  disr(5);  disr(6);  disr(7);
disr(8);  disr(9);  disr(10); disr(11);
disr(12); disr(13); disr(14); disr(15);
disr(16); disr(17); disr(18); disr(19);
disr(20); disr(21); disr(22); disr(23);
disr(24); disr(25); disr(26); disr(27);
disr(28); disr(29); disr(30); disr(31);

dirq(0);  dirq(1);  dirq(2);  dirq(3);
dirq(4);  dirq(5);  dirq(6);  dirq(7);
dirq(8);  dirq(9);  dirq(10); dirq(11);
dirq(12); dirq(13); dirq(14); dirq(15);

extern void sys_call();

const char* exception_messages[32] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

/* similar with GDT */
static inline void idt_set(u8 k, u32 base, u16 selector, u8 flags) {
    /* The interrupt routine's base address */
    idt[k].base_lo = (base & 0xFFFF);
    idt[k].base_hi = (base >> 16) & 0xFFFF;

    /* The segment or 'selector' that this IDT entry will use
    *  is set here, along with any access flags */
    idt[k].selector = selector;
    idt[k].zero = 0;
    idt[k].flags = flags;
}

void irq_enable(u8 irq) {
    u16 irq_mask = (inb(0xA0+1)<<8) + inb(0x20+1);
    irq_mask &= ~(1<<irq);
    outb(0x21, irq_mask);
    outb(0xA1, irq_mask >> 8);
}

void irq_eoi(u32 nr) {
    outb(0x20, 0x20);
    if(nr >= 40) {
        outb(0xA0, 0x20);
    }
}

/* This installs a custom IRQ handler for the given IRQ */
void irq_install(int irq, isq_t handler) {
    irq_routines[irq] = handler;
}

/* 1000 1110 */
#define isr_set(idx) \
    idt_set(idx, (u32)(isr##idx), 1<<3, 0x8F);

#define irq_set(idx) \
    idt_set(idx + 32, (u32)(irq##idx), 1<<3, 0x8F);

/* Installs the IDT */
void idt_init() {
    puts("[idt]  .... ");

    /* Sets the special IDT pointer up, just like in 'gdt.c' */
    idtp.limit = (sizeof (struct idt_entry) * 256) - 1;
    idtp.base = (u32)&idt;

    memset(&idt, 0, sizeof(struct idt_entry) * 256);

    /* Remap the irq table. */
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
    irq_enable(2);

    /* Add any new ISRs to the IDT here using idt_set_gate */
    /* Points the processor's internal register to the new IDT */

    isr_set(0);  isr_set(1);  isr_set(2);  isr_set(3);
    isr_set(4);  isr_set(5);  isr_set(6);  isr_set(7);
    isr_set(8);  isr_set(9);  isr_set(10); isr_set(11);
    isr_set(12); isr_set(13); isr_set(14); isr_set(15);
    isr_set(16); isr_set(17); isr_set(18); isr_set(19);
    isr_set(20); isr_set(21); isr_set(22); isr_set(23);
    isr_set(24); isr_set(25); isr_set(26); isr_set(27);
    isr_set(28); isr_set(29); isr_set(30); isr_set(31);

    irq_set(0);  irq_set(1);  irq_set(2);  irq_set(3);
    irq_set(4);  irq_set(5);  irq_set(6);  irq_set(7);
    irq_set(8);  irq_set(9);  irq_set(10); irq_set(11);
    irq_set(12); irq_set(13); irq_set(14); irq_set(15);

    idt_set(0x80, (u32)sys_call, 1<<3, 0xEF);

    /* just flush this */
    asm volatile(
        "lidt %0"
        :: "m"(idtp));

    asm volatile("sti");
    done();
}


/* This gets called from our ASM interrupt handler stub. */
void hwint_handler(struct registers_t* regs) {
    if ((regs->cs & 3) == 3) {
        current->p_trap = regs;
    }

    /* Find out if we have a custom handler to run for this
    *  IRQ and then run it */
    isq_t handler = irq_routines[regs->int_no];
    if (regs->int_no < 32) {
        if(handler)
            handler(regs);
    } else {
        irq_eoi(regs->int_no);
        if(handler)
            handler(regs);
    }
    /* user-mode */
    if((regs->cs & 3) == 3 && current != 0 ) {
        sched();
    }
}
