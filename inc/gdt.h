
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

#define NR_GDTENTRY 6

/* Defines a GDT entry  */
/* this page is a good refence */
/* http://wiki.osdev.org/Global_Descriptor_Table */
struct gdt_entry {
    u32        limit_lo:16;    // Low bits of segment limit
    u32        base_lo :16;    // Low bits of segment base address
    u32        base_mi :8;     // Middle bits of segment base address
    
    u32        type    :4;     // Segment type (see STS_ constants)
    u32        s       :1;     // 0 = system, 1 = application
    u32        dpl     :2;     // Descriptor Privilege Level
    u32        present :1;     // Present
    
    u32        limit_hi:4;     // High bits of segment limit
    u32        avl     :1;     // Unused (available for software use)
    u32        r       :1;     // Reserved
    u32        db      :1;     // 0 = 16-bit segment, 1 = 32-bit segment
    u32        g       :1;     // Granularity: limit scaled by 4K when set
    u32        base_hi :8;     // High bits of segment base address
} __attribute__((packed));

struct gdt_ptr {
    u16  limit;
    u32  base;
} __attribute__((packed));


// Application segment type bits
#define STA_X       0x8     // Executable segment
#define STA_E       0x4     // Expand down (non-executable segments)
#define STA_C       0x4     // Conforming code segment (executable only)
#define STA_W       0x2     // Writeable (non-executable segments)
#define STA_R       0x2     // Readable (executable segments)
#define STA_A       0x1     // Accessed


// System segment type bits
#define STS_LDT     0x2     // Local Descriptor Table
#define STS_TG      0x5     // Task Gate / Coum Transmitions
#define STS_TA      0x9     // Available 32-bit TSS
#define STS_TB      0xB     // Busy 32-bit TSS
#define STS_CG      0xC     // 32-bit Call Gate
#define STS_IG      0xE     // 32-bit Interrupt Gate
#define STS_TRG     0xF     // 32-bit Trap Gate
#define RING0 0
#define RING3 3

#define KERN_CS (1<<3)
#define KERN_DS (2<<3)
#define USER_CS ((3<<3)|3)
#define USER_DS ((4<<3)|3)

#endif

