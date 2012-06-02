
// @Name   : page.c 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-09 08:35:30
//
// @Brief  :

#include <system.h>
#include <page.h>
#include <kheap.h>
#include <bitmap.h>
#include <string.h>

u32  begin_address = (u32)(0x1000000);

u32* frames;
u32  nr_frames;

page_dir_t* kernel_page_dir = NULL;
page_dir_t* current_page_dir = NULL;

void page_fault_handler(struct registers_t* regs);

inline u32 align_addr(u32 addr)
{
    if(addr & 0xFFFFF000) {
        addr &= 0xFFFFF000;
        addr += 0x1000;
    }
    return addr;
}


//alloc size*4 bytes,
static void*
_internel_alloc(u32 size, int align)
{
    if(align != 0)
        begin_address = align_addr(begin_address);
    u32 addr = begin_address;
    begin_address += size;
    return (void*)addr;
}

/* set addr used, and make a new pte if neccessary */
page_t* get_page(page_dir_t* page_dir, u32 addr, int make)
{
    addr /= 0x1000;
    u32 index = addr/1024;
    if(page_dir->tables[index] != NULL){
        return &(page_dir->tables[index]->pages[addr%1024]);
    }
    else if(make){
        void* pte = _internel_alloc(sizeof(pte_t), 1);
        memset(pte, 0, sizeof(pte_t));
        page_dir->tables[index] = (pte_t*)pte;
        page_dir->tableAddress[index] = (u32)pte | 0x7;
        return &(page_dir->tables[index]->pages[addr%1024]);
    }
    else {
        return NULL;
    }
}

void set_page_frame(page_t* page, int is_kernel, int is_write)
{
    if(page->pb_addr != 0){
        return;
    }
    else {
        u32 addr = first_frame();
        if(addr == (u32)-1) {
            return;
        }
        else {
            set_frame( addr * 0x1000);
            page->present = 1;
            page->rw = (is_write)? 1 : 0;
            page->user = (is_kernel)? 1: 0;
            page->pb_addr = addr;
        }
    }
}

void page_init(u32 end_address)
{
    puts("page init...\n");

    u32 k, addr, cr0;
#ifndef NDEBUG
    printk("begin_address: %x \nend_address  : %x  \n",
           begin_address, end_address);
#endif

    nr_frames = (end_address) / 0x1000; //end_address is aligned
    frames  = (u32*)_internel_alloc(INDEX(nr_frames), 0);
    
#ifndef NDEBUG
    printk("nr_frames:%d\n", nr_frames);
    printk("frames address: %x\n", (u32)frames);
#endif
    
    memset(frames, 0, (INDEX(nr_frames)) * sizeof(u32));

    /* make page direcotry */
    kernel_page_dir = (page_dir_t*)_internel_alloc(sizeof(page_dir_t), 1);
    //kernel_page_dir->physicalAddr = (u32)kernel_page_dir->tableAddress;
    memset(kernel_page_dir, 0 , sizeof(page_dir_t));
    current_page_dir = kernel_page_dir;

    for( k=KHEAP_START_ADDR; k<KHEAP_END_ADDR; k+=0x1000)
        get_page(kernel_page_dir, k, 1);
        
    addr = 0x0;
    while(addr < begin_address) {
        set_page_frame(get_page(kernel_page_dir, addr, 1), 0, 0);
        addr += 0x1000;
    }

    // Now allocate those pages we mapped earlier.
    for (k=KHEAP_START_ADDR; k<KHEAP_END_ADDR; k+=0x1000)
        set_page_frame(get_page(kernel_page_dir, k, 1), 0, 0);

    irq_install_handler(13, (isq_t)(&page_fault_handler));

    //switch to page directory
    asm volatile("mov %0, %%cr3":: "r"(&current_page_dir->tableAddress));
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000; // Enable paging!
    asm volatile("mov %0, %%cr0":: "r"(cr0));

    kheap_init((void*)KHEAP_START_ADDR, (void*)KHEAP_END_ADDR);
#ifndef NDEBUG
    puts("end page init...\n");
#endif
}

void page_fault_handler(struct registers_t* regs)
{
    // A page fault has occurred.
    // The faulting address is stored in the CR2 register.
    u32 faulting_address;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));
    
    // The error code gives us details of what happened.
    int present   = !(regs->err_code & 0x1); // Page not present
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
    printk_hex(faulting_address);
    puts("\n");
    PANIC("Page fault");
}


void* get_page_align(u32* phys) {
    page_dir_t* dir_addr = kmalloc_align(sizeof(page_dir_t), 1);
    kassert(dir_addr);
    kassert(((u32)dir_addr%0x1000) == 0);
    if(phys!=0) {
        page_t* page = get_page(kernel_page_dir, (u32)dir_addr, 0);
        *phys = page->pb_addr*0x1000 + ((u32)dir_addr&0xFFF);
    }
    return dir_addr;
}


page_dir_t* copy_page_dir(page_dir_t* src) {
    kassert(src);
    u32 phys;
    u32 offset;
    page_dir_t* dir = (page_dir_t*)get_page_align(&phys);
    memset(dir, 0, sizeof(page_dir_t));
    offset = (u32)dir->tableAddress  - (u32)dir;
    dir->physicalAddr = phys + offset;
    if(src)
        return src;
    return NULL;
}


