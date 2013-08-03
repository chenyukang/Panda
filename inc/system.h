
// @Name   : SYSTEM_H 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-03 18:02:36
//
// @Brief  : 

#if !defined(SYSTEM_H)
#define SYSTEM_H

#include <types.h>


#define KB  1024

//debug option
//#define NDEBUG 1
#ifndef NDEBUG
#define kassert(_Expression)                                            \
  if(!(_Expression))                                                    \
  {                                                                     \
      printk("FILE:%s LINE:%d  (Assertion: \"%s\" FAILED)\n",           \
             __FILE__, __LINE__ , #_Expression);                        \
      while(1) ;                                                        \
  }                                                                     \

#else
#define kassert(_Expression)                
#endif

/* this panic just get into dead loop */
#define  PANIC(Expression)                         \
    {                                              \
        printk("FATAL ERROR:%s\n", Expression);    \
        while(1) ;                                 \
    }                                              \


#define done() { puts_color_str("[Done]\n", 0x0E); }

/* This defines what the stack looks like after an ISR was running */
struct registers_t {
    s32 gs, fs, es, ds;
    s32 edi, esi, ebp, _esp, ebx, edx, ecx, eax;
    s32 int_no, err_code;
    s32 eip, cs, eflags, esp, ss;    
} __attribute__((packed));

typedef void (*isq_t) (struct registers_t* r);
typedef void (*isr_t) (struct registers_t* r);

void irq_install(int irq, isq_t handler);

void timer_init(void);
void timer_wait(int ticks);

u32  get_sys_ticks(void);

extern void gdt_init(void);
extern void idt_init(void);
extern void kb_init(void);
extern void irq_enable(u8);
extern void irq_eoi(u32);
extern void test_idt();

#endif

