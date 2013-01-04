
/*******************************************************************************
 *
 *      @name   : GDT_H
 *
 *      @author : Yukang Chen (moorekang@gmail.com)
 *      @date   : 2012-08-18 10:01:00
 *
 *      @brief  :
 *
 *******************************************************************************/

#if !defined(GDT_H)
#define GDT_H

#include <types.h>

#define NR_GDTENTRY 5

/* Defines a GDT entry  */
/* this page is a good refence */
/* http://wiki.osdev.org/Global_Descriptor_Table */
struct gdt_entry {
    u16 limit;
    u16 base_low;
    u8  base_middle;
    u8  access;
    u8  granularity;
    u8  base_high;
} __attribute__((packed));

struct gdt_ptr {
    u16  limit;
    u32  base;
} __attribute__((packed));


#endif

