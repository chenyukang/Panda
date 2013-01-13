
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


#define ASM	 __asm__ __volatile__

u16 inw (u16 _port);

u8 inb (u16 _port);
inline void outb (u16 _port, u8 _data);

inline void write_nmi(u8 nmi);
inline void enable_nmi(void);
inline void disable_nmi(void);

#define local_irq_disable() (native_irq_disable())
#define local_irq_enable()  (native_irq_enable())
#define safe_halt()         (native_safe_halt())
#define halt()              (native_halt())



#define first_zerobit(x) (first_onebit(~(x)))

#define sti() __asm__ ("sti"::)
#define cli() __asm__ ("cli"::)
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

static inline void
outsl(int port, const void *addr, int cnt)
{
  asm volatile("cld; rep outsl" :
               "=S" (addr), "=c" (cnt) :
               "d" (port), "0" (addr), "1" (cnt) :
               "cc");
}

static inline void
insl(int port, void *addr, int cnt)
{
  asm volatile("cld; rep insl" :
               "=D" (addr), "=c" (cnt) :
               "d" (port), "0" (addr), "1" (cnt) :
               "memory", "cc");
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


void	port_read(u16 port, void* buf, int n);
void	port_write(u16 port, void* buf, int n);

#if 0
#define port_read(port, buf, nr)                                        \
    __asm__("cld;rep;insw"::"d"(port), "D"(buf), "c"(nr):"cx", "di")

#define port_write(port, buf, nr) \
    __asm__("cld;rep;outsw"::"d"(port), "S"(buf), "c"(nr):"cx", "si")
#endif

#endif

