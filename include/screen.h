
// @Name   : SCREEN_H 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-03 18:04:55
//
// @Brief  :

#if !defined(SCREEN_H)
#define SCREEN_H

#include <system.h>

void init_video(void);
void putch(unsigned char c);
void puts(const char* text);
void printk(const char* text);
void printk_hex(u32 val);
void printk_int(u32 val);
void* memset(void* addr, unsigned char v, size_t cnt);
void* memcpy(void *dest, const void *src, size_t cnt);
void* memmove(void* dest, const void* src, size_t cnt);


extern void gdt_init(void);
extern void idt_init(void);
extern void kb_init(void);
#endif

