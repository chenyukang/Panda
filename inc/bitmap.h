
// @Name   : BITMAP_H 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-09 08:34:09
//
// @Brief  :

#if !defined(BITMAP_H)
#define BITMAP_H

#include <types.h>

#define INDEX(a)  ((a)/(4*8))
#define OFFSET(a) ((a)%(4*8))

void set_frame(u32 addr);
void clear_frame(u32 addr);
u32  first_frame();

#endif

