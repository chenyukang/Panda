
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



struct pde {
    u32 pt_p      : 1;
    u32 pt_rw     : 1;
    u32 pt_priv   : 1;
    u32 pt_write  : 1;
    u32 pt_cache  : 1;
    u32 pt_access : 1;
    u32 pt_resv   : 1;
    u32 pt_pgsize : 1;  //0 means 4KB
    u32 pt_gloabl : 1;  //ignored
    u32 pt_avial  : 3;  //for system programmer
    u32 pt_base   : 20; //base addr
};

//so struct pde* (with 1024) == pg_dir


void page_init();
void mm_init();
page_t* get_page(page_dir_t* page_dir, u32 addr, int make);
void set_page_frame(page_t* page, int is_kernel, int is_write);
page_dir_t* copy_page_dir(page_dir_t* src);
void switch_page_directory(page_dir_t *dir);

#endif
