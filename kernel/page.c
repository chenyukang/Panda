
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
u32 used_addr;

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

//alloc size*4 bytes
static void* _alloc(u32 size, int align) {
    if(align != 0) {
        used_addr = align_addr(used_addr);
    }
    u32 addr = used_addr;
    used_addr += size;
    return (void*)addr;
}

void flush_pgd(struct pde* pg_dir) {
    u32 cr0, cr4;
    // put page table addr
    cu_pg_dir = pg_dir;
    asm volatile("mov %0, %%cr3":: "r"((u32)pg_dir)); 

    asm volatile("mov %%cr4, %0": "=r"(cr4));
    cr4 |= 0x10;                              // enable 4MB page
    asm volatile("mov %0, %%cr4":: "r"(cr4));

    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000;                        // enable paging!
    asm volatile("mov %0, %%cr0":: "r"(cr0));
}

void init_pgs() {    //init free page list
    u32 k;
    page_nr = (end_addr)/(PAGE);
    pages = (struct page*)_alloc(sizeof(struct page)*page_nr, 0);
    free_page_nr = (used_addr)/0x1000 + 1;
    for(k=0; k<free_page_nr; k++) { //this is used by kernel
        pages[k].pg_idx = k;
        pages[k].pg_refcnt = 1;
        pages[k].pg_next = 0;
    }
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
    
    printk("used_addr: %d KB\n", used_addr/(1024));
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
        printk("return pg->pg_idx:%d\n", pg->pg_idx);
        return pg;
    }
    return 0;
}

struct pte* find_pte(struct pde* pg_dir, u32 vaddr , u32 new) {
    struct pde* pde;
    struct pte* pte;
    struct page* pg;
    if( vaddr < end_addr ) {
        PANIC("find_pte() error: invalid virtual address");
    }

    pde = &pg_dir[PDEX(vaddr)];
    if(pde->pt_p == 0) { //not present
        if(new == 0){
            return 0;
        }
        pg = alloc_page();
        if(pg == 0) {
            return 0;
        }
        pde->pt_p = 1;
        pde->pt_pgsz = 1;
        pde->pt_flag = (0x001);
        pde->pt_base = pg->pg_idx;
        pte = (struct pte*)(pde->pt_base * PAGE);
        memset(pte, 0, PAGE);
        flush_pgd(pg_dir);
    }
    pte = (struct pte*)(pde->pt_base * PAGE);
    printk("in find_pte() : %x\n", &pte);
    return &pte[PTEX(vaddr)];
}

int put_page(struct pde* pg_dir, u32 vaddr, struct page* pg) {
    struct pte* pte = find_pte(pg_dir, vaddr, 1);
    if(pte == 0)
        return 0;
    pte->pt_base = pg->pg_idx;
    pte->pt_p = 1;
    pte->pt_flag = (0x001);
    flush_pgd(pg_dir);
    return 1;
}

void copy_pgd(struct pde* from, struct pde* targ) {
    kassert(from && targ && "error copy_pgd");
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
            tpte = (struct pte*)(alloc_page()->pg_idx*PAGE);
            tpte->pt_base = ((u32)tpte>>12);
            for(k=0; k<1024; k++) {
                tpte[k].pt_base = fpte[k].pt_base;
                tpte[k].pt_flag = fpte[k].pt_flag;
                if(fpte[k].pt_p) {
                    //turn off PTE_W 
                    fpte[k].pt_flag &= (0x110);
                    tpte[k].pt_flag &= (0x110);
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
    printk("end_addr: %x\n", end_addr);
    used_addr = ini_addr;
    
    init_pgd(&(pg_dir0[0])); //init the kernel pg_dir

    //printk("pg_dir0: %x\n", &pg_dir0);
    init_pgs();
    
    irq_install_handler(13, (isq_t)(&page_fault_handler));
    flush_pgd(&(pg_dir0[0]));


#if 0    
    int* p = (int*)(end_addr-0x1000);
    printk("addr: %x\n", p);
    kassert(p);
    printk("addr: %x --> value: %d\n", p, *p);


    int k;
    for(k=0; k<end_addr + (0x2000); k+=(0x1000)){
        printk("addr: %x --> ", k);
        int* p = (int*)k;
        printk("value: %d\n", *p);
    }
#endif
}

void init_pgd(struct pde* pg_dir) {
    u32 k;
    memset(pg_dir, 0, sizeof(pg_dir[0])*1024);
    k_dir_nr = (end_addr)/(PAGE*1024);
    for(k=0; k<k_dir_nr; k++) {
        pg_dir[k].pt_base = k<<10;
        pg_dir[k].pt_p = 1;    //present
        pg_dir[k].pt_pgsz = 1; //4MB
        pg_dir[k].pt_flag = 0; //system
    }
    for(k=k_dir_nr; k<1024; k++) {
        pg_dir[k].pt_base = 0;
        pg_dir[k].pt_p = 0;
        pg_dir[k].pt_flag = 1; //user
    }
}

void do_wt_page(void* addr) {
    printk("in do_wt_page\n");

    return;
}

void do_no_page(void* addr) {
    printk("in do_no_page: %x\n", addr);
    struct page* pg = alloc_page();
    if(pg == 0) {
        PANIC("out of memory");
    }
    printk("pg: %x\n", pg);
    
#define PG_ADDR(addr)   ((u32)(addr) & ~0xFFF)
    
    put_page(cu_pg_dir, (u32)PG_ADDR(addr), pg);
}

void page_fault_handler(struct registers_t* regs) {
    // The faulting address is stored in the CR2 register.
    u32 fault_addr;
    asm volatile("mov %%cr2, %0" : "=r" (fault_addr));

    if((regs->err_code & 0x1) == 0) { //present error
        do_no_page((void*)fault_addr);
        return;
    }
    if(regs->err_code & 0x2) {
        do_wt_page((void*)fault_addr);
        return;
    }
    //if (rw) {puts("read-only ");}
    if (regs->err_code & 0x4) {
        puts("user-mode ");
    }
    if (regs->err_code & 0x8) {
        PANIC("touch reserved addr");
    }
    if (regs->err_code & 0x10) {
        PANIC("instruction fetch error");
    }
    puts("Page fault! ( ");
    puts(") at ");
    printk_hex(fault_addr);
    puts("\n");
    PANIC("Page fault");
}
