
// @Name   : PAGE_H 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-09 08:35:39
//
// @Brief  :

#if !defined(PAGE_H)
#define PAGE_H

#include <system.h>


#define PG_TO_VADDR(pte_ptr) (((pte_ptr)->pt_base) * PAGE)
#define PDEX(vaddr)          ((u32) ((vaddr>>22) & 0x3FF))
#define PTEX(vaddr)          ((u32) ((vaddr>>12) & 0x3FF))


struct pde {
    u32 pt_p      : 1;
    u32 pt_flag   : 6;
    /* u32 pt_rw     : 1; */
    /* u32 pt_priv   : 1; */
    /* u32 pt_write  : 1; */
    /* u32 pt_cache  : 1; */
    /* u32 pt_access : 1; */
    /* u32 pt_resv   : 1; */

    u32 pt_pgsz   : 1;  //0 means 4KB
    u32 pt_global : 1;  //ignored
    u32 pt_avial  : 3;  //for system programmer
    u32 pt_base   : 20; //base addr
};
//so struct pde* (with 1024) == pg_dir

struct pte {
    u32 pt_p      : 1;
    u32 pt_flag   : 6;
    /* u32 pt_rw     : 1; */
    /* u32 pt_priv   : 1; */
    /* u32 pt_write  : 1; */
    /* u32 pt_cache  : 1; */
    /* u32 pt_access : 1; */
    /* u32 pt_resv   : 1; */
    
    u32 pt_pgsize : 1;  //0 means 4KB
    u32 pt_gloabl : 1;  //ignored
    u32 pt_avial  : 3;  //for system programmer
    u32 pt_base   : 20; //base addr
};

struct page {
    u32 pg_idx;
    u32 pg_refcnt;
    struct page* pg_next;
};

void mm_init();

struct page* alloc_page();
struct page* find_page(u32 nr);

void free_page(struct page* pg);
void copy_pgd(struct pde* from, struct pde* targ);

void flush_pgd(struct pde* pg_dir);
#endif
