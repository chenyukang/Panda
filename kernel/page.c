
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

extern char __kimg_end__;

struct pde pg_dir0[1024] __attribute__((aligned(4096)));
struct pde* cu_pg_dir;

struct page* pages;
struct page  freepg_list;

u32 end_addr, ker_addr, used_addr;

u32 page_nr;      //all page number
u32 free_page_nr; //free page number
u32 k_dir_nr;     //this is number of page mapped by kernel

void page_fault_handler(struct registers_t* regs);

void flush_pgd(struct pde* pg_dir) {
    cli();
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
    sti();
}

/* ========================== begin pyhsical page =========================== */

static void* _alloc(u32 size ) {
    used_addr = PAGE_ROUND_UP(used_addr);
    u32 addr = used_addr;
    used_addr += size;
    return (void*)addr;
}

//init page list, link free pages with a list 
void init_pages() {
    u32 k;
    page_nr = (end_addr)/(PAGE_SIZE);
    printk("page_nr: %d\n", page_nr);
    
    pages = (struct page*)_alloc(sizeof(struct page) * page_nr);
    used_addr = PAGE_ROUND_UP(used_addr);
    free_page_nr = (used_addr)/0x1000;

    for(k=0; k<free_page_nr; k++) { //this is used by kernel
        pages[k].pg_idx = k;
        pages[k].pg_refcnt = 1;
        pages[k].pg_next = 0;
    }
    for(k=free_page_nr; k<page_nr; k++) { //free pages 
        pages[k].pg_idx = k;
        pages[k].pg_refcnt = 0;
        pages[k].pg_next = 0;
    }
    struct page* pg = &freepg_list; //usr freepg_list link all free pages
    for(k=free_page_nr; k<page_nr; k++) {
        pg->pg_next = &pages[k];
        pg = pg->pg_next;
    }
    
    //fill this for danglig refs.
    for(k=free_page_nr; k<page_nr; k++) {
        memset((void*)(k*PAGE_SIZE), 1, PAGE_SIZE);
    }
    printk("free pages: %d\n", page_nr - free_page_nr);
}

struct page* find_page(u32 nr) {
    kassert(nr > free_page_nr &&
            nr < 1024);
    return &pages[nr];
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

struct page* alloc_page() {
    struct page* pg = &freepg_list;
    if(pg->pg_next) {
        cli();  //no interupt now!
        pg = pg->pg_next;
        freepg_list.pg_next = pg->pg_next;
        pg->pg_refcnt = 1;
        sti();
        return pg;
    }
    return 0;
}


/* alloc_mem alloc PAGE_SIZE memory for use */
u32 alloc_mem() {
    struct page* pg = alloc_page();
    return pg->pg_idx * PAGE_SIZE;
}

void free_mem(u32 addr) {
    struct page* pg = find_page((addr >> 12));
    free_page(pg);
}


/*========================end pyhsical page ========================== */


struct pde* alloc_pde() {
    struct page* p = alloc_page();
    if(p == 0)
        return 0;
    return (struct pde*)(p->pg_idx * PAGE_SIZE);
}

struct pte*
find_pte(struct pde* pg_dir, u32 vaddr , u32 new) {
    struct pde* pde;
    struct pte* pte;
    struct page* pg;
    
#if 1
    if( vaddr < end_addr ) {
        PANIC("find_pte() error: invalid virtual address");
    }
#endif
    
    pde = &pg_dir[PDEX(vaddr)];
    if((pde->pt_flags & PTE_P) == 0) { //not present
        if(new == 0)
            return 0;

        pg = alloc_page();
        if(pg == 0) 
            return 0;
        pde->pt_flags = PTE_P | PTE_U | PTE_W;
        pde->pt_base = (pg->pg_idx);
        pte = (struct pte*)(pde->pt_base * PAGE_SIZE );
        memset(pte, 0, PAGE_SIZE);
        flush_pgd(pg_dir);
    }
    pte = (struct pte*)(pde->pt_base * PAGE_SIZE);
    return &pte[PTEX(vaddr)];
}

/* set vaddr presented */
int put_page(struct pde* pg_dir, u32 vaddr, struct page* pg) {
    struct pte* pte = find_pte(pg_dir, vaddr, 1);
    if(pte == 0)
        return 0;
    kassert(pte);
    pte->pt_base = pg->pg_idx;
    pte->pt_flags |= PTE_P;
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
        tpde->pt_flags = fpde->pt_flags;
        if(fpde->pt_flags & PTE_P) {
            fpte = (struct pte*)(fpde->pt_base * PAGE_SIZE);
            tpte = (struct pte*)(alloc_page()->pg_idx * PAGE_SIZE);
            tpte->pt_base = ((u32)tpte>>12);
            for(k=0; k<1024; k++) {
                tpte[k].pt_base = fpte[k].pt_base;
                tpte[k].pt_flags = fpte[k].pt_flags;
                if(fpte[k].pt_flags & PTE_P) {
                    //turn off PTE_W 
                    fpte[k].pt_flags &= ~PTE_W;
                    tpte[k].pt_flags &= ~PTE_W;
                    page = find_page(fpte->pt_base);
                    page->pg_refcnt++;
                }
            }
        }
    }
}
            

void init_page_dir(struct pde* pg_dir) {
    u32 k;
    memset(pg_dir, 0, sizeof(pg_dir[0])*1024);
    k_dir_nr = (end_addr)/(PAGE_SIZE*1024);
    for(k=0; k<k_dir_nr; k++) {
        pg_dir[k].pt_base  = k<<10;
        pg_dir[k].pt_flags = PTE_P | PTE_W | PTE_PS;
    }

    for(k=k_dir_nr; k<1024; k++) {
        pg_dir[k].pt_base  = 0;
        pg_dir[k].pt_flags = PTE_U;
    }
}

/* init memory, end_addr is the max physical addr on machine
   ker_addr is address where kernel code ended ,
   used_addr will be ker_end_addr added kernel stack etc */
void mm_init() {
    puts("mm init ...\n");

    // we put the end_addr at 0x90002
    // during booting process, so got it
    // 0~ker_addr is for kernel code and data
    end_addr = (1<<20) + ((*(u16*)0x90002)<<10);
    ker_addr = (u32)(&(__kimg_end__));
    ker_addr = PAGE_ROUND_UP(ker_addr);
    // don't use ker_addr ~ 0x100000
    // free memory begin with 0x100000
    used_addr = 0x100000;
    printk("mem_size : %d MB\n", end_addr/(KB*KB));
    printk("ker_size : %dKB\n", ker_addr/(KB));
    printk("page_size: %dKB\n", PAGE_SIZE/(KB));
    printk("used_size: %dKB\n", used_addr/(KB));

    init_page_dir(&(pg_dir0[0])); //init the kernel pg_dir

    init_pages();

    irq_install_handler(13, (isq_t)(&page_fault_handler));
    flush_pgd(&(pg_dir0[0]));
}


void do_wt_page(void* addr) {
    printk("in do_wt_page\n");
    return;
}

void do_no_page(void* addr) {
    struct page* pg = alloc_page();
    if(pg == 0) {
        PANIC("out of memory");
    }

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
        PANIC("touch reserved address");
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
