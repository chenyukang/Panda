
// @Name   : ASM_H 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-03 17:50:52
//
// @Brief  :

#include <asm.h>

#if 0
#define in(port) ({                                             \
            unsigned char v;                                    \
            __asm__ __volatile__ ("inb %%dx, %%al"              \
                                  : "=a" (v) : "d" (port));     \
            v;                                                  \
})

#define out(port, value) \
__asm__ __volatile__ ("outb %%al, %%dx" :: "a" (value),     \
                          "d" (port))                           \

#endif


unsigned short inw (unsigned short _port) {
    unsigned short rv;
    __asm__ __volatile__ ("inw %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

unsigned char inb (unsigned short _port) {
    unsigned char rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

void outb (unsigned short _port, unsigned char _data) {
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}


inline void write_nmi(unsigned char nmi) {
	outb(0x70, nmi);
	inb(0x71);
}

inline void enable_nmi(void) {
	write_nmi(ENABLE_NMI);
}

inline void disable_nmi(void) {
	write_nmi(DISABLE_NMI);
}

