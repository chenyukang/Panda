
// @Name   : STRING_H 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-08 21:55:28
//
// @Brief  :

#if !defined(STRING_H)
#define STRING_H

#include <system.h>

void strcpy(char* dest, char* src);
void puts(const char* text);
void printk_hex(u32 val);
void printk_int(u32 val);
void* memset(void* addr, unsigned char v, size_t cnt);
void* memcpy(void *dest, const void *src, size_t cnt);
void* memmove(void* dest, const void* src, size_t cnt);
u16*  memsetw(u16* dest, u16 val, size_t count);
    
int printk(const char* format, ... );

#endif

