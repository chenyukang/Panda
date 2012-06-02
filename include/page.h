
// @Name   : PAGE_H 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-09 08:35:39
//
// @Brief  :

#if !defined(PAGE_H)
#define PAGE_H

#include <system.h>

typedef struct _page {
    u32 present : 1; 
    u32 rw      : 1; //Read/Write
    u32 user    : 1; //User/Supervisor
    u32 pwt     : 1; //Write-through
    u32 pcd     : 1; //cache disable
    u32 access  : 1; //access flag
    u32 dirty   : 1; //write  flag
    u32 unused  : 5;
    u32 pb_addr : 20; //page base address
} page_t;

//page table entry
typedef struct _pte {
    page_t pages[1024];
}pte_t;

//page directory entry
typedef struct _pde {
    pte_t* tables[1024];
    u32    tableAddress[1024];
    u32    physicalAddr;
}page_dir_t;

void page_init(u32 end_address);
#endif
