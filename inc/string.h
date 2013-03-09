
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
s32  strncmp(const char* v1, const char* v2, u32 n);

void puts(const char* text);
void printk_hex(u32 val);
void printk_int(u32 val);
size_t strlen(const char* str);

void* memset(void* addr, unsigned char v, size_t cnt);
void* memcpy(void *dest, const void *src, size_t cnt);
void* memmove(void* dest, const void* src, size_t cnt);
u16*  memsetw(u16* dest, u16 val, size_t count);
s32   memcmp(const void* v1, const void* v2, u32 n);

int printk(const char* format, ... );

#endif

