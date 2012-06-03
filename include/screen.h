
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
void putch(char c);
void puts_color_str(char* str, unsigned color);
void puts_mid_str(char* str);

#endif

