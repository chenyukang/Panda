
// @Name   : ASM_H 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-05 22:27:56
//
// @Brief  :

#if !defined(ASM_H)
#define ASM_H


#include <types.h>

#define ENABLE_NMI 0x00
#define DISABLE_NMI 0x80

unsigned short inw (unsigned short _port);
unsigned char inb (unsigned short _port);

inline void outb (unsigned short _port, unsigned char _data);
inline void write_nmi(unsigned char nmi);
inline void enable_nmi(void);
inline void disable_nmi(void);

#define local_irq_disable() (native_irq_disable())
#define local_irq_enable()  (native_irq_enable())
#define safe_halt()         (native_safe_halt())
#define halt()              (native_halt())


#define first_zerobit(x) (first_onebit(~(x)))

/* ffs: if ret == 0 : no one bit found
   return index is begin with 1 */
static inline int first_onebit(int x)
{
    if (!x) {
        return 0;
    }
    else {
        int ret;
        __asm__("bsfl %1, %0; incl %0" : "=r" (ret) : "r" (x));
        return ret;
    }
}

static inline u32 native_irq_save(void)
{
    u32 flags;

    /*
     * "=rm" is safe here, because "pop" adjusts the stack before
     * it evaluates its effective address -- this is part of the
     * documented behavior of the "pop" instruction.
     */
    asm volatile("# __raw_save_flags\n\t"
                 "pushf ; pop %0"
                 : "=rm" (flags)
                 : /* no input */
                 : "memory");

    return flags;
}

static inline void native_irq_restore(u32 flags)
{
    asm volatile("push %0 ; popf"
                 : /* no output */
                 :"g" (flags)
                 :"memory", "cc");
}

static inline void native_irq_disable(void)
{
    asm volatile("cli": : :"memory");
}

static inline void native_irq_enable(void)
{
    asm volatile("sti": : :"memory");
}

static inline void native_safe_halt(void)
{
    asm volatile("sti; hlt": : :"memory");
}

static inline void native_halt(void)
{
    asm volatile("hlt": : :"memory");
}

/* Functions below is according Linux kernel source*/

/* Check at compile time that sth is of a particular type
   Always return 1
*/
#define typecheck(type,x)                       \
    ({  type __dummy;                           \
        typeof(x) __dummy2;                     \
        (void)(&__dummy == &__dummy2);          \
        1;                                      \
    })


#define local_irq_save(flags)                   \
    do { typecheck(unsigned long, flags);       \
        flags = native_irq_save();              \
    } while(0);                                 \

#define local_irq_restore(flags)                \
    do { typecheck(unsigned long, flags);       \
        flags = native_irq_restore();           \
    } while(0);                                 \





#endif

