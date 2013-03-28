
// @Name   : ASM_H 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-03 17:50:52
//
// @Brief  :

#include <asm.h>


unsigned short inw (unsigned short _port) {
    unsigned short rv;
    __asm__ __volatile__ ("inw %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
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



