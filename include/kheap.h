
// @Name   : SORTLINK_H 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-03-20 05:34:50
//
// @Brief  : this is used for kheap, make sure this link list is sorted 

#if !defined(SORTLINK_H)
#define SORTLINK_H

#include <system.h>

typedef void* type_t;
typedef long Align ;

union header {
    struct {
        union header* next;
        u32  size;
    }s;
    Align x; //this is never used!
};

typedef union header Header;

void kfree(void* ap);
void* kmalloc(u32 nbytes);

#endif

