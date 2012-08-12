
// @Name   : page.c 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-09 08:35:30
//
// @Brief  :

#include <asm.h>
#include <system.h>
#include <page.h>
#include <kheap.h>
#include <string.h>
#include <task.h>

//4kb
#define  PAGE      0x1000
#define  PG_DIR_NR 1024
#define  MAX_PAGE_NR (0x8000000/PAGE)

extern char __kimg_end__;

struct pde pg_dir0[1024] __attribute__((aligned(4096)));
struct pde* cu_pg_dir;

struct page* pages;
struct page  freepg_list;

u32 end_addr;
u32 ini_addr;
u32 use_addr;
u32 page_nr;
u32 free_page_nr;
u32 k_dir_nr; //this is real page number

void init_pgd(struct pde* pg_dir);
void page_fault_handler(struct registers_t* regs);

static inline u32 align_addr(u32 addr) {
    if(addr % 0x1000) {
        addr &= 0xFFFFF000;
        addr += 0x1000;
    }
    return addr;
}

//alloc size*4 bytes,
static void* _alloc(u32 size, int align) {
    if(align != 0)
        use_addr = align_addr(use_addr);
    u32 addr = use_addr;
    use_addr += size;
    return (void*)addr;
}

void switch_pgd(struct pde* pg_dir) {
    u32 cr0, cr4;
    cu_pg_dir = pg_dir;
    asm volatile("mov %0, %%cr3":: "r"((u32)pg_dir)); // put page table addr

    asm volatile("mov %%cr4, %0": "=r"(cr4));
    cr4 |= 0x10;                              // enable 4MB page
    asm volatile("mov %0, %%cr4":: "r"(cr4));

    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000;                        // enable paging!
    asm volatile("mov %0, %%cr0":: "r"(cr0));
}

void init_pgs() {    //init free page list
    u32 k, cnt = 0;
    page_nr = (end_addr)/(PAGE);
    free_page_nr = (ini_addr)/0x1000 + 1;
    for(k=free_page_nr; k<page_nr; k++) {
        cnt++;
    }
    pages = (struct page*)_alloc(sizeof(struct page)*cnt, 0);
    for(k=free_page_nr; k<page_nr; k++) {
        pages[k].pg_idx = k;
        pages[k].pg_refcnt = 0;
        pages[k].pg_next = 0;
    }
    struct page* pg = &freepg_list;
    for(k=free_page_nr; k<page_nr; k++) {
        pg->pg_next = &pages[k];
        pg = pg->pg_next;
    }
    
    printk("used_addr: %d KB\n", use_addr/(1024));
#if 0
    pg = &freepg_list;
    u32 num = 0;
    while(pg) {
        num++;
        printk("page: %x\n", pg);
        pg  = pg->pg_next;
    }
    kassert(num = cnt);
#endif
}


struct page* find_page(u32 nr) {
    kassert(nr > free_page_nr &&
            nr < 1024);
    return &pages[nr];
}

struct page* alloc_page() {
    struct page* pg = &freepg_list;
    if(pg->pg_next) {
        cli();  //no interupt now!
        pg = pg->pg_next;
        freepg_list.pg_next = pg->pg_next;
        pg->pg_next = 0;
        sti();
        return pg;
    }
    return 0;
}



static inline
void* trans_vm(void* pdir) {
    struct pde* addr = (struct pde*)pdir;
    kassert(pdir);
    return (void*)(addr->pt_base * PAGE);
}

void copy_pgd(struct pde* from, struct pde* targ) {
    kassert(from &&
            targ &&
            "error copy_pgd");
    struct pde *fpde, *tpde;
    struct pte *fpte, *tpte;
    struct page *page;
    u32 idx, k;
    for(idx=k_dir_nr; idx<1024; idx++) {
        fpde = &(from[idx]);
        tpde = &(targ[idx]);
        tpde->pt_p  = fpde->pt_p;
        tpde->pt_flag = fpde->pt_flag;
        tpde->pt_pgsz = fpde->pt_pgsz;
        if(fpde->pt_p) {
            fpte = (struct pte*)(fpde->pt_base*PAGE);
            tpte = (struct pte*)(trans_vm(alloc_page()));
            tpte->pt_base = ((u32)tpte>>12);
            for(k=0; k<1024; k++) {
                tpte[k].pt_base = fpte[k].pt_base;
                tpte[k].pt_flag = fpte[k].pt_flag;
                if(fpte[k].pt_p) {
                    fpte[k].pt_flag &= ~(0x001);
                    tpte[k].pt_flag &= ~(0x001);
                    page = find_page(fpte->pt_base);
                    page->pg_refcnt++;
                }
            }
        }
    }
}
        
            
void free_page(struct page* pg) {
    kassert(pg &&
            pg->pg_refcnt > 0 &&
            "try free invalid page");
    cli();
    pg->pg_refcnt--;
    if(pg->pg_refcnt == 0) {
        pg->pg_next = freepg_list.pg_next;
        freepg_list.pg_next = pg;
    }
    sti();
}


/* init memory, end_addr is the max physical addr on machine */
void mm_init() {
    puts("mm init ...\n");
    
    //we put the end_addr at 0x90002
    //during booting process, so got it */
    ini_addr = (u32)(&(__kimg_end__)) + (0x1000);
    end_addr = (1<<20) + ((*(u16*)0x90002)<<10);
    use_addr = ini_addr;
    
    init_pgd(&(pg_dir0[0])); //init the kernel pg_dir

    init_pgs();
    
    irq_install_handler(13, (isq_t)(&page_fault_handler));
    switch_pgd(&(pg_dir0[0]));
}

void init_pgd(struct pde* pg_dir) {
    u32 k;
    memset(pg_dir, 0, sizeof(pg_dir[0])*1024);
    k_dir_nr = (end_addr)/(PAGE*1024);
    for(k=0; k<k_dir_nr; k++) {
        pg_dir[k].pt_base = k<<10;
        pg_dir[k].pt_p = 1;
        pg_dir[k].pt_pgsz = 1;
        pg_dir[k].pt_flag = 0;
    }
    for(k=k_dir_nr; k<1024; k++) {
        pg_dir[k].pt_base = k<<10;
        pg_dir[k].pt_flag = 1; //user
    }
}

void page_fault_handler(struct registers_t* regs) {
    // A page fault has occurred.
    // The faulting address is stored in the CR2 register.
    u32 fault_addr;
    asm volatile("mov %%cr2, %0" : "=r" (fault_addr));
    
    // The error code gives us details of what happened.
    int present = !(regs->err_code & 0x1); // Page not present
    int rw = regs->err_code & 0x2;           // Write operation?
    int us = regs->err_code & 0x4;           // Processor was in user-mode?
    int reserved = regs->err_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
    //int id = regs->err_code & 0x10;        // Caused by an instruction fetch?

    // Output an error message.
    puts("Page fault! ( ");
    if (present) {puts("present ");}
    if (rw) {puts("read-only ");}
    if (us) {puts("user-mode ");}
    if (reserved) {puts("reserved ");}
    puts(") at ");
    printk_hex(fault_addr);
    puts("\n");
    PANIC("Page fault");
}
