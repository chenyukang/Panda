
// @Name   : MM.H
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-09 08:35:39
//
// @Brief  :

#ifndef _MM_H_
#define _MM_H_

#include <system.h>


#define  PAGE_SIZE             0x1000
#define  PG_DIR_NR             1024
//#define  MAX_PAGE_NR           (0x8000000/PAGE_SIZE)
#define  PAGE_ROUND_UP(addr)   (((addr + PAGE_SIZE - 1) & (-PAGE_SIZE)))
#define  PAGE_ROUND_DWON(addr) ((addr) & (-PAGE_SIZE))
#define  PG_ADDR(addr)         ((u32)(addr) & ~0xFFF)

#define  PG_TO_VADDR(pte_ptr) (((pte_ptr)->pt_base) * PAGE_SIZE)
#define  PDEX(vaddr)          ((u32) ((vaddr>>22) & 0x3FF))
#define  PTEX(vaddr)          ((u32) ((vaddr>>12) & 0x3FF))
#define  PPN(vaddr)           (((u32) (vaddr)) >> 12)

//64MB
#define PMEM     0x8000000
#define NPAGE    (PMEM/PAGE_SIZE)
//Present
//Writeable
//User
//Accessed
//Dirty
//PAGE SIZE
#define PTE_P  0x001
#define PTE_W  0x002
#define PTE_U  0x004
#define PTE_A  0x020
#define PTE_D  0x040
#define PTE_PS 0x080

/*
31 ---------------------------------------- 0
|pagetable 4kb aligned base| Unused | Flags |
|   20 bits                | 3      |  9    |

flags details:
8 ---------------------------------------- 0
| Ignored | Page Size |  0  | Accessed | Cache Disable | Write through | User/Super | Read/Write | Present |
|   1     |      1    |  1  |     1    |       1       |       1       |     1      |     1      |    1    |
 */
struct pde {
    u32 pt_flags  : 9;
    u32 pt_avial  : 3;  //for system programmer
    u32 pt_base   : 20; //base addr
};
//so struct pde* (with 1024) == pg_dir

struct pte {
    u32 pt_flags  : 9;
    u32 pt_avial  : 3;  //for system programmer
    u32 pt_base   : 20; //base addr
};

struct page {
    u16 pg_idx;
    u8  pg_refcnt;
    struct page* pg_next;
};

void mm_init();

struct page* alloc_page();
struct page* find_page(u32 nr);
struct pde* alloc_pde();

void free_page(struct page* pg);
void init_page_dir(struct pde* pgd);
s32  free_pgd(struct pde* pgd);
void copy_pgd(struct pde* from, struct pde* targ);
struct pte* find_pte(struct pde* pg_dir, u32 vaddr , u32 new);

u32  alloc_mem();
void free_mem();

void flush_pgd(struct pde* pg_dir);

void do_no_page(void* addr);
void do_wt_page(void* addr);

#endif
