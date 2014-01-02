
// @Name   : mm.c
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-09 08:35:30
//
// @Brief  :

#include <asm.h>
#include <system.h>
#include <mm.h>
#include <string.h>
#include <task.h>

//4kb
extern char  __kimg_end__;
struct pde   pg_dir0[1024] __attribute__((aligned(4096)));

struct page  pages[NPAGE] __attribute__((aligned(4096)));;
struct page  freepg_list;

static u32 end_addr, ker_addr, used_addr;

static u32 page_nr;      //all page number
static u32 free_nr;      //free page number
static u32 kern_nr;      //this is number of page mapped by kernel

void page_fault_handler(struct registers_t* regs);

void flush_pgd(struct pde* pgdir) {
    asm volatile("mov %%eax, %%cr3":: "a"(pgdir));    // put page table addr
}

void enable_page() {
    s32 cr0 , cr4;
    asm volatile("mov %%cr4, %0": "=r"(cr4));
    cr4 |= 0x10;                              // enable 4MB page
    asm volatile("mov %0, %%cr4":: "r"(cr4));

    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000;                        // enable paging!
    asm volatile("mov %0, %%cr0":: "r"(cr0));
}

/* ========================== begin pyhsical page =========================== */

//init page list, link free pages with a list
void init_pages() {
    u32 k;
    page_nr = (end_addr)/(PAGE_SIZE);
    used_addr = PAGE_ROUND_UP(used_addr);
    free_nr = (used_addr)/0x1000;

    //this is used by kernel
    for(k=0; k<free_nr; k++) {
        pages[k].pg_idx = k;
        pages[k].pg_refcnt = 1;
        pages[k].pg_next = 0;
    }

    //free pages
    for(k=free_nr; k<NPAGE; k++) {
        pages[k].pg_idx = k;
        pages[k].pg_refcnt = 0;
        pages[k].pg_next = 0;
    }
    struct page* pg = &freepg_list;
    for(k=free_nr; k<NPAGE; k++) {
        pg->pg_next = &pages[k];
        pg = pg->pg_next;
    }
}

struct page* find_page(u32 nr) {
    kassert(nr >= 0 &&
            nr < NPAGE);
    return &pages[nr];
}

void free_page(struct page* pg) {
    kassert(pg && pg->pg_refcnt > 0 &&
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
    struct page* pg = find_page(PPN(addr));
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

    if( vaddr < 0x8000000 ) {
        printk("vaddr: %x\n", vaddr);
        PANIC("find_pte() error: invalid virtual address");
    }

    pde = &pg_dir[PDEX(vaddr)];
    if((pde->pt_flags & PTE_P) == 0) { //not present
        if(new == 0)
            return 0;
        pg = alloc_page(); //kassert(0);
        if(pg == 0) {
            return 0;
        }
        pde->pt_flags = PTE_P | PTE_U | PTE_W;
        pde->pt_base = (pg->pg_idx);
        pte = (struct pte*)(pde->pt_base * PAGE_SIZE );
        memset(pte, 0, PAGE_SIZE);
        flush_pgd(current->p_vm.vm_pgd);
    } else {
    }
    pte = (struct pte*)(pde->pt_base * PAGE_SIZE);
    return &pte[PTEX(vaddr)];
}

/* set vaddr presented */
struct pte* put_page(struct pde* pg_dir, u32 vaddr, struct page* pg) {
    struct pte* pte = find_pte(pg_dir, vaddr, 1);  kassert(pte);
    pte->pt_base = pg->pg_idx;
    pte->pt_flags = PTE_U | PTE_W | PTE_P;
    flush_pgd(pg_dir);
    return pte;
}

void copy_pgd(struct pde* from, struct pde* targ) {
    kassert(from && targ && "error copy_pgd");
    struct pde *fpde, *tpde;
    struct pte *fpte, *tpte;
    struct page *page;
    u32 idx, k;
    for(idx=kern_nr; idx<1024; idx++) {
        fpde = &(from[idx]);
        tpde = &(targ[idx]);
        tpde->pt_flags = fpde->pt_flags;
        if(fpde->pt_flags & PTE_P) {
            fpte = (struct pte*)(fpde->pt_base * PAGE_SIZE);
            tpte = (struct pte*)(alloc_page()->pg_idx * PAGE_SIZE);
            tpde->pt_base = ((u32)tpte>>12);
            for(k=0; k<1024; k++) {
                tpte[k].pt_base  = fpte[k].pt_base;
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

s32 free_pgd(struct pde* pgd) {
    struct pde* pde;
    struct pte* pte;
    struct pte* pt;
    struct page* page;
    u32 pdr, pr;
    for(pdr = PDEX(PMEM); pdr < 1024; pdr++) {
        pde = &pgd[pdr];
        if(pde->pt_flags & PTE_P) {
            pt = (struct pte*)(pde->pt_base * PAGE_SIZE);
            for(pr = 0; pr < 1024; pr++) {
                pte = &pt[pr];
                if(pte->pt_flags & PTE_P) {
                    page = find_page(pte->pt_base);
                    free_page(page);
                }
            }
            free_mem((u32)pt);
        }
    }
    return 0;
}

void init_page_dir(struct pde* pg_dir) {
    u32 k;
    kern_nr = (PMEM)/(PAGE_SIZE*1024);
    for(k=0; k<kern_nr; k++) {
        pg_dir[k].pt_base  = k<<10;
        pg_dir[k].pt_flags = PTE_P | PTE_W | PTE_PS;
    }

    for(k=kern_nr; k<1024; k++) {
        pg_dir[k].pt_base  = 0;
        pg_dir[k].pt_flags = PTE_U;
    }
}

/* init memory, end_addr is the max physical addr on machine
   ker_addr is address where kernel code ended ,
   used_addr will be ker_end_addr added kernel stack etc */
void mm_init() {
    puts("[mm]   .... ");

    // we put the end_addr at 0x90002
    // during booting process, so got it
    // 0~ker_addr is for kernel code and data
    //end_addr = (1<<20) + ((*(u16*)0x90002)<<10);
    end_addr = 0x8000000;
    ker_addr = (u32)(&(__kimg_end__));
    ker_addr = PAGE_ROUND_UP(ker_addr);

    // don't use ker_addr ~ 0x100000
    // free memory begin with 0x100000
    used_addr = 0x100000;
#if 0
    printk("\nmem_size : %dMB\n", end_addr/(KB*KB));
    printk("ker_size : %dKB\n", ker_addr/(KB));
    printk("page_size: %dKB\n", PAGE_SIZE/(KB));
    printk("used_size: %dKB\n", used_addr/(KB));
#endif
    init_page_dir(&(pg_dir0[0])); //init the kernel pg_dir
    init_pages();
    irq_install(14, (isq_t)(&page_fault_handler));
    flush_pgd(&(pg_dir0[0]));
    enable_page();
    done();
}

void do_wt_page(void* vaddr) {
    struct vma *vp;
    struct pte *pte;
    struct page *pg;
    char *old_page, *new_page;

    vp = find_vma((u32)vaddr);
    if (vp->v_flag & VMA_RDONLY) {
        //sigsend(cu->p_pid, SIGSEGV, 1);
        printk("task:%d name:%s address:%x\n",
               current->pid, current->name, (u32)vaddr);
        PANIC("memory error");
    }

    if (vp->v_flag & VMA_PRIVATE) {
        pte = find_pte(current->p_vm.vm_pgd, (u32)vaddr, 1);
        pg = find_page(pte->pt_base);
        if (pg->pg_refcnt > 1) {
            pg->pg_refcnt--;
            old_page = (char*)(pte->pt_base * PAGE_SIZE);
            new_page = (char*)alloc_mem();
            memcpy(new_page, old_page, PAGE_SIZE);
            pte->pt_base = PPN(new_page);
            pte->pt_flags |= PTE_W;
            flush_pgd(current->p_vm.vm_pgd);
        }
        else if (pg->pg_refcnt==1) {
            pte->pt_flags |= PTE_W;
            flush_pgd(current->p_vm.vm_pgd);
        }
    }
}

void do_no_page(void* vaddr) {
    struct vm *vm;
    struct vma *vp;
    struct pte *pte;
    struct page *pg;
    char *buf;
    u32 off;

    vm = &current->p_vm;
    // if this page lies on the edge of user stack,
    // grows the stack.
    if (vm->vm_stack.v_base - (u32)vaddr <= PAGE_SIZE) {
        vm->vm_stack.v_base -= PAGE_SIZE;
        vm->vm_stack.v_size += PAGE_SIZE;
        pg = alloc_page();
        put_page(vm->vm_pgd, PG_ADDR(vaddr), pg);
        return;
    }

    vp = find_vma((u32)vaddr);
    if (vp == NULL) {
        printk("vaddr: %x\n", (u32)vaddr);
        printk("pid: %d name: %s\n", current->pid, current->name);
        kassert(0);
        //sigsend(current->p_pid, SIGSEGV, 1);
        return;
    }
    // demand bss/heap
    if (vp->v_flag & VMA_ZERO) {
        pg = alloc_page();
        put_page(vm->vm_pgd, PG_ADDR(vaddr), pg);
        memset((void*)PG_ADDR(vaddr), 0, PAGE_SIZE);
        return;
    }

    // demand file
    if (vp->v_flag & VMA_MMAP) {
        pg = alloc_page();
        pte = put_page(vm->vm_pgd, PG_ADDR(vaddr), pg);
        // fill this new-allocated page
        buf = (char*)PG_ADDR(vaddr);
        off = (u32)buf - vp->v_base + vp->v_ioff;
        ilock(vp->v_ino);
        readi(vp->v_ino, buf, off, PAGE_SIZE);
        iunlock(vp->v_ino);
        pte->pt_flags &= ~(vp->v_flag & VMA_RDONLY? 0:PTE_W);
        flush_pgd(current->p_vm.vm_pgd);
    }
}

void page_fault_handler(struct registers_t* regs) {
    u32 fault_addr;
    //fetch falting address
    asm volatile("mov %%cr2, %0" : "=r" (fault_addr));

    //present error
    if((regs->err_code & 0x001) == 0) {
        do_no_page((void*)fault_addr);
        return;
    }
    //write error
    if(regs->err_code & 0x002) {
        do_wt_page((void*)fault_addr);
        return;
    }
    if (regs->err_code & 0x004) {
        printk("address: %x %d %s %d\n",
               (u32)fault_addr, current->pid, current->name, current->stat);
        PANIC("user-mode ");
        return;
    }
    if (regs->err_code & 0x008) {
        PANIC("touch reserved address");
    }
    if (regs->err_code & 0x10) {
        PANIC("instruction fetch error");
    }
    printk("Page fault at [%x]\n", (u32)fault_addr);
}
