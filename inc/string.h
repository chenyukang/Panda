
// @Name   : STRING_H
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-08 21:55:28
//
// @Brief  :

#if !defined(STRING_H)
#define STRING_H

#include <types.h>
#include <screen.h>

#define abs(x) ((x < 0) ? (-(x)): (x) )

typedef char* va_list;

#define INTSIZEOF(n) ((sizeof(n)+sizeof(int)-1) & ~(sizeof(int)-1))
#define va_start(ap, format) ( ap = (va_list)(&format) + INTSIZEOF(format))
#define va_end(ap) ( ap=(va_list)0 )
#define va_arg(ap, type) ( *(type*) ((ap += INTSIZEOF(type)) - INTSIZEOF(type)))

void strcpy(char* dest, char* src);
void strncpy(char* dest, char* src, size_t cnt);
void* strcat(char* dest, const char* src);
s32  strncmp(const char* v1, const char* v2, u32 n);
s32  strcmp(const char* v1, const char* v2);

void puts(const char* text);
size_t strlen(const char* str);

int atoi(char* s);
int isspace(char c);
int isalpha(char c);
int isdigit(char c);

void* memset(void* addr, unsigned char v, size_t cnt);
void* memcpy(void *dest, const void *src, size_t cnt);
void* memmove(void* dest, const void* src, size_t cnt);
u16*  memsetw(u16* dest, u16 val, size_t count);
s32   memcmp(const void* v1, const void* v2, u32 n);

#ifdef USR
#include <syscall.h>
int _sprintf(char* buf, const char* format, va_list args);
int printf(const char* format, ... );
int sprintf(char* buf, const char* format, ...);
#else
int printk(const char* format, ... );
int sprintk(char* buf, const char* format, ...);
#endif

#endif
