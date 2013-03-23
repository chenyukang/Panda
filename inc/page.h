
// @Name   : PAGE_H 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-09 08:35:39
//
// @Brief  :

#if !defined(PAGE_H)
#define PAGE_H

#include <system.h>


#define  PAGE_SIZE             0x1000
#define  PG_DIR_NR             1024
#define  MAX_PAGE_NR           (0x8000000/PAGE_SIZE)
#define  PAGE_ROUND_UP(addr)   (((addr + PAGE_SIZE - 1) & (-PAGE_SIZE)))
#define  PAGE_ROUND_DWON(addr) ((addr) & (-PAGE_SIZE))
#define  PG_ADDR(addr)         ((u32)(addr) & ~0xFFF)

#define PG_TO_VADDR(pte_ptr) (((pte_ptr)->pt_base) * PAGE)
#define PDEX(vaddr)          ((u32) ((vaddr>>22) & 0x3FF))
#define PTEX(vaddr)          ((u32) ((vaddr>>12) & 0x3FF))

#define PTE_P  0x001 //Present
#define PTE_W  0x002 //Writeable
#define PTE_U  0x004 //User
#define PTE_A  0x020 //Accessed
#define PTE_D  0x040 //Dirty
#define PTE_PS 0x080 //PAGE SIZE

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
    u32 pg_idx;
    u32 pg_refcnt;
    struct page* pg_next;
};

void mm_init();

struct page* alloc_page();
struct page* find_page(u32 nr);
struct pde* alloc_pde();

void free_page(struct page* pg);
void copy_pgd(struct pde* from, struct pde* targ);

u32  alloc_mem();
void free_mem();

void flush_pgd(struct pde* pg_dir);
#endif
