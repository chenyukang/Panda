
// @Name   : CPU_H 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-08 00:05:20
//
// @Brief  :

#if !defined(CPU_H)
#define CPU_H

#define cpuid(in, a, b, c, d) __asm__("cpuid": "=a" (a), "=b" (b), "=c" (c), "=d" (d) : "a" (in));

int detect_cpu(void);

#endif
