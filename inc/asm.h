
// @Name   : ASM_H
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-05 22:27:56
//
// @Brief  :

#if !defined(ASM_H)
#define ASM_H


#include <types.h>

#define ASM	 __asm__ __volatile__

#define xhalt()  (native_halt())

#define first_zerobit(x) (first_onebit(~(x)))

#define sti() __asm__ ("sti")
#define cli() __asm__ ("cli")

/* load TSS into tr */
static inline void ltr(u32 n) {
    asm volatile("ltr %%ax"::"a"(n));
}

/* ffs: if ret == 0 : no one bit found
   return index is begin with 1 */
static inline int first_onebit(int x) {
    if (!x) {
        return 0;
    }
    else {
        int ret;
        __asm__("bsfl %1, %0; incl %0" : "=r" (ret) : "r" (x));
        return ret;
    }
}

static inline void native_halt(void) {
    asm volatile("hlt": : :"memory");
}

static inline void outb(u16 port, u8 data) {
    asm volatile("out %0,%1" : : "a" (data), "d" (port));
}

static inline u8 inb(u16 port) {
  u8 data;
  asm volatile("in %1,%0" : "=a" (data) : "d" (port));
  return data;
}

static inline void outsl(int port, const void *addr, int cnt) {
  asm volatile("cld; rep outsl" :
               "=S" (addr), "=c" (cnt) :
               "d" (port), "0" (addr), "1" (cnt) :
               "cc");
}

static inline void insl(int port, void *addr, int cnt) {
  asm volatile("cld; rep insl" :
               "=D" (addr), "=c" (cnt) :
               "d" (port), "0" (addr), "1" (cnt) :
               "memory", "cc");
}

static inline u32 xchg(volatile u32 *addr, u32 newval) {
  u32 result;

  // The + in "+m" denotes a read-modify-write operand.
  asm volatile("lock; xchgl %0, %1" :
               "+m" (*addr), "=a" (result) :
               "1" (newval) :
               "cc");
  return result;
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

#define _SYS0(T0, FN)                           \
    T0 FN(){                                    \
        register int r;                         \
        asm volatile(                           \
            "int $0x80"                         \
            :"=a"(r),                           \
             "=b"(errno)                        \
            :"a"(NR_##FN)                       \
            );                                  \
        return r;                               \
    }

#define _SYS1(T0, FN, T1)                       \
    T0 FN(T1 p1){                               \
        register int r;                         \
        asm volatile(                           \
            "int $0x80"                         \
            :"=a"(r),                           \
             "=b"(errno)                        \
            :"a"(NR_##FN),                      \
             "b"((int)p1)                       \
            );                                  \
        if (r<0){                               \
            errno = -r;                         \
            return -1;                          \
        }                                       \
        return r;                               \
    }

#define _SYS2(T0, FN, T1, T2)                   \
    T0 FN(T1 p1, T2 p2){                        \
        register int r;                         \
        asm volatile(                           \
            "int $0x80"                         \
            :"=a"(r),                           \
             "=b"(errno)                        \
            :"a"(NR_##FN),                      \
             "b"((int)p1),                      \
             "c"((int)p2)                       \
            );                                  \
        return r;                               \
    }

#define _SYS3(T0, FN, T1, T2, T3)               \
    T0 FN(T1 p1, T2 p2, T3 p3){                 \
        register int r;                         \
        asm volatile(                           \
            "int $0x80"                         \
            :"=a"(r),                           \
             "=b"(errno)                        \
            :"a"(NR_##FN),                      \
             "b"((int)p1),                      \
             "c"((int)p2),                      \
             "d"((int)p3)                       \
            );                                  \
        return r;                               \
    }


#endif
