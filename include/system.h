
// @Name   : SYSTEM_H 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-03 18:02:36
//
// @Brief  : 

#if !defined(SYSTEM_H)
#define SYSTEM_H

#include <types.h>


#define abs(x) ((x < 0) ? (-(x)): (x) )

//debug option
#define _DEBUG_ 1

#ifdef _DEBUG_
#define kassert(_Expression)                                            \
    if(!(_Expression))                                                  \
    {                                                                   \
        printk("FILE:%s LINE:%d  (Assertion: \"%s\" FAILED)\n", __FILE__, __LINE__ , #_Expression); \
        while(1) ;                                                      \
    }                                                                   \

#else
#define kassert(_Expression)                
#endif

/* This defines what the stack looks like after an ISR was running */
struct registers_t
{
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cs, eflags, useresp, ss;    
};

typedef void (*isq_t) (struct registers_t* r);

void irq_install_handler(int irq, isq_t handler);
void idt_set(unsigned char k, unsigned long base,
             unsigned short selector, unsigned char flags);

void timer_init(u32 frequency);
void timer_wait(int ticks);

extern void gdt_init(void);
extern void idt_init(void);
extern void kb_init(void);
extern void test_idt();
#endif

