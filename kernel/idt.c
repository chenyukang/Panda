
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
//#include <syscall.h>

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

#define PREV 1
#ifdef PREV
/* Defines an IDT entry */
struct idt_entry {
    unsigned short base_lo;  //16 bits
    unsigned short selector; //16 bits
    unsigned char  zero;  //8 bits
    unsigned char  flags; //8 bits
    unsigned short base_hi; //16 bits
} __attribute__((packed));

#else
/* Defines an IDT entry */
struct idt_entry {
    unsigned short base_lo;
    unsigned short selector;
    unsigned int zero : 8;
    unsigned int type : 4;
    unsigned int  sys : 1;
    unsigned int  dpl : 2;
    unsigned int  p   : 1;
    unsigned short base_hi;
} __attribute__((packed));
#endif

struct idt_ptr {
    unsigned short limit;
    unsigned int   base;
} __attribute__((packed));

/* The IDT table , used for interrupt */
struct idt_entry idt[256];
struct idt_ptr   idtp;
isq_t irq_routines[256]; 

// These extern directives let us access the addresses of our ASM ISR handlers.
extern void isr0 ();
extern void isr1 ();
extern void isr2 ();
extern void isr3 ();
extern void isr4 ();
extern void isr5 ();
extern void isr6 ();
extern void isr7 ();
extern void isr8 ();
extern void isr9 ();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void irq0 ();
extern void irq1 ();
extern void irq2 ();
extern void irq3 ();
extern void irq4 ();
extern void irq5 ();
extern void irq6 ();
extern void irq7 ();
extern void irq8 ();
extern void irq9 ();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

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

#ifdef PREV
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
    int* p = (int*)&(idt[k]);
    int i;
    for(i=0; i<2; i++) {
        printk("%x ", *p);
        p++;
    }
    printk("\n");
}
#else
static inline void idt_set(u8 k, u32 base, u16 selector, u8 type, u8 dpl) {
    idt[k].base_lo = (base & 0xFFFF);
    idt[k].base_hi = (base >> 16) & 0xFFFF;
    idt[k].selector = selector;
    idt[k].zero = 0;
    idt[k].type = type;
    idt[k].dpl  = dpl; //2 bits
    idt[k].sys  = 0;
    idt[k].p    = 1;
    int* p = (int*)&(idt[k]);
    int i;
    for(i=0; i<2; i++) {
        printk("%x ", *p);
        p++;
    }
    printk("\n");
}


static inline void trap_gate(u32 nr, u32 base){
    idt_set(nr, base, KERN_CS, STS_TRG, RING0);
}

static inline void intr_gate(u32 nr, u32 base){
    idt_set(nr, base, KERN_CS, STS_IG, RING0);
}

#if 0
static inline void syst_gate(u32 nr, u32 base){
    idt_set(nr, base, KERN_CS, STS_TRG, RING3);
}
#endif

#endif

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
void irq_install_handler(int irq, isq_t handler) {
    irq_routines[irq] = handler;
}

/* Installs the IDT */
void idt_init() {
    puts("idt_init ...\n");
    
    /* Sets the special IDT pointer up, just like in 'gdt.c' */
    idtp.limit = (sizeof (struct idt_entry) * 256) - 1;
    idtp.base = (u32)&idt;
    /* Clear out the entire IDT, initializing it to zeros */
    memset(&idt, 0, sizeof(struct idt_entry) * 256);

    // Remap the irq table.
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

#ifdef PREV
    /* Add any new ISRs to the IDT here using idt_set_gate */
    /* Points the processor's internal register to the new IDT */
    idt_set( 0, (u32)isr0 , 1<<3, 0x8F); //1000 1110
    idt_set( 1, (u32)isr1 , 1<<3, 0x8F);
    idt_set( 2, (u32)isr2 , 1<<3, 0x8F);
    idt_set( 3, (u32)isr3 , 1<<3, 0x8F);
    idt_set( 4, (u32)isr4 , 1<<3, 0x8F);
    idt_set( 5, (u32)isr5 , 1<<3, 0x8F);
    idt_set( 6, (u32)isr6 , 1<<3, 0x8F);
    idt_set( 7, (u32)isr7 , 1<<3, 0x8F);
    idt_set( 8, (u32)isr8 , 1<<3, 0x8F);
    idt_set( 9, (u32)isr9 , 1<<3, 0x8F);
    idt_set(10, (u32)isr10, 1<<3, 0x8F);
    idt_set(11, (u32)isr11, 1<<3, 0x8F);
    idt_set(12, (u32)isr12, 1<<3, 0x8F);
    idt_set(13, (u32)isr13, 1<<3, 0x8F);
    idt_set(14, (u32)isr14, 1<<3, 0x8F);
    idt_set(15, (u32)isr15, 1<<3, 0x8F);
    idt_set(16, (u32)isr16, 1<<3, 0x8F);
    idt_set(17, (u32)isr17, 1<<3, 0x8F);
    idt_set(18, (u32)isr18, 1<<3, 0x8F);
    idt_set(19, (u32)isr19, 1<<3, 0x8F);
    idt_set(20, (u32)isr20, 1<<3, 0x8F);
    idt_set(21, (u32)isr21, 1<<3, 0x8F);
    idt_set(22, (u32)isr22, 1<<3, 0x8F);
    idt_set(23, (u32)isr23, 1<<3, 0x8F);
    idt_set(24, (u32)isr24, 1<<3, 0x8F);
    idt_set(25, (u32)isr25, 1<<3, 0x8F);
    idt_set(26, (u32)isr26, 1<<3, 0x8F);
    idt_set(27, (u32)isr27, 1<<3, 0x8F);
    idt_set(28, (u32)isr28, 1<<3, 0x8F);
    idt_set(29, (u32)isr29, 1<<3, 0x8F);
    idt_set(30, (u32)isr30, 1<<3, 0x8F);
    idt_set(31, (u32)isr31, 1<<3, 0x8F);
    
    idt_set(32, (u32)irq0, 1<<3, 0x8E);
    idt_set(33, (u32)irq1, 1<<3, 0x8E);
    idt_set(34, (u32)irq2, 1<<3, 0x8E);
    idt_set(35, (u32)irq3, 1<<3, 0x8E);
    idt_set(36, (u32)irq4, 1<<3, 0x8E);
    idt_set(37, (u32)irq5, 1<<3, 0x8E);
    idt_set(38, (u32)irq6, 1<<3, 0x8E);
    idt_set(39, (u32)irq7, 1<<3, 0x8E);
    idt_set(40, (u32)irq8, 1<<3, 0x8E);
    idt_set(41, (u32)irq9, 1<<3, 0x8E);
    idt_set(42, (u32)irq10, 1<<3, 0x8E);
    idt_set(43, (u32)irq11, 1<<3, 0x8E);
    idt_set(44, (u32)irq12, 1<<3, 0x8E);
    idt_set(45, (u32)irq13, 1<<3, 0x8E);
    idt_set(46, (u32)irq14, 1<<3, 0x8E);
    idt_set(47, (u32)irq15, 1<<3, 0x8E);

#else
    trap_gate(0, (u32)isr0);
    trap_gate(1, (u32)isr1);
    trap_gate(2, (u32)isr2);
    trap_gate(3, (u32)isr3);
    trap_gate(4, (u32)isr4);
    trap_gate(5, (u32)isr5);
    trap_gate(6, (u32)isr6);
    trap_gate(7, (u32)isr7);
    trap_gate(8, (u32)isr8);
    trap_gate(9, (u32)isr9);
    trap_gate(10, (u32)isr10);
    trap_gate(11, (u32)isr11);
    trap_gate(12, (u32)isr12);
    trap_gate(13, (u32)isr14);
    trap_gate(15, (u32)isr15);
    trap_gate(16, (u32)isr16);
    trap_gate(17, (u32)isr17);
    trap_gate(18, (u32)isr19);
    trap_gate(20, (u32)isr20);
    trap_gate(21, (u32)isr21);
    trap_gate(22, (u32)isr22);
    trap_gate(23, (u32)isr23);
    trap_gate(24, (u32)isr24);
    trap_gate(25, (u32)isr25);
    trap_gate(26, (u32)isr26);
    trap_gate(27, (u32)isr27);
    trap_gate(28, (u32)isr28);
    trap_gate(29, (u32)isr29);
    trap_gate(30, (u32)isr30);
    trap_gate(31, (u32)isr31);

    intr_gate(32, (u32)irq0);
    intr_gate(33, (u32)irq1);
    intr_gate(34, (u32)irq2);
    intr_gate(35, (u32)irq3);
    intr_gate(36, (u32)irq4);
    intr_gate(37, (u32)irq5);
    intr_gate(38, (u32)irq6);
    intr_gate(39, (u32)irq7);
    intr_gate(40, (u32)irq8);
    intr_gate(41, (u32)irq9);
    intr_gate(42, (u32)irq10);
    intr_gate(43, (u32)irq11);
    intr_gate(44, (u32)irq12);
    intr_gate(45, (u32)irq13);
    intr_gate(46, (u32)irq14);
    intr_gate(47, (u32)irq15);
#endif

//    syst_gate(0x80, (u32)sys_call); // syscall
//    irq_install_handler(0x80, &do_syscall);  // in syscall.c
    
    /* just flush this */
    asm volatile(
        "lidt %0"
        :: "m"(idtp));

    asm volatile("sti");
}



// This gets called from our ASM interrupt handler stub.
void hwint_handler(struct registers_t* regs) {
#if 0
    puts("recieved interrupt: ");
    printk_hex(regs->int_no);
    puts("\n");
#endif

    if ((regs->cs & 3) == 3) {
        current_task->p_trap = regs; 
    }

    /* Find out if we have a custom handler to run for this
    *  IRQ, and then finally, run it */
    isq_t handler = irq_routines[regs->int_no];
    /* Is this a fault whose number is from 0 to 31? */
    if (regs->int_no < 32) {
        if(handler){
            handler(regs);
        }
    } else {
        irq_eoi(regs->int_no);
        if(handler) {
            handler(regs);
        }
    }   

    if((regs->cs & 3) == 3) {
        sched();
    }
}

void test_idt(){
    asm volatile ("int $0xD");
    asm volatile ("int $0x04");
    asm volatile ("int $0x06");
}

