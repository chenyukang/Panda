
// @Name   : page.c 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-09 08:35:30
//
// @Brief  :

#include <system.h>
#include <page.h>
#include <bitmap.h>
#include <string.h>

extern u32 end;
u32  begin_address = (u32)(0x1000000);

u32* frames;
u32  nframes;


page_directory_t* page_dir = NULL;
page_directory_t* current_page_dir = NULL;


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
void* kmalloc(u32 size, int align)
{
    if(align != 0)
        begin_address = align_addr(begin_address);
    u32 addr = begin_address;
    begin_address += size;
    return (void*)addr;
}


page_t* get_page(page_directory_t* page_dir, u32 addr, int make)
{
    addr /= 0x1000;
    u32 index = addr/1024;
    if(page_dir->tables[index] != NULL){
        return &(page_dir->tables[index]->pages[addr%1024]);
    }
    else if(make){
        void* pte = kmalloc(sizeof(pte_t), 1);
        memset(pte, 0, sizeof(pte_t));
        page_dir->tables[index] = (pte_t*)pte;
        page_dir->tableAddress[index] = (u32)pte | 0x7;
        return &(page_dir->tables[index]->pages[addr%1024]);
    }
    else {
        return 0;
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
    printk("%s\n", "page init...");
    printk("%s:", "begin_address");
    printk_hex((u32)begin_address);
    printk(" %s:", "end_address");
    printk_hex(end_address);
    puts("\n");
    
    nframes = (end_address) / 0x1000; //end_address is aligned
    frames  = (u32*)kmalloc(INDEX(nframes), 0);
    printk("nframes:%d\n", nframes);
    printk("frames address:");
    printk_hex((u32)frames);
    printk("\n");
    
    memset(frames, 0, (INDEX(nframes)) * sizeof(u32));

    page_dir = (page_directory_t*)kmalloc(sizeof(page_directory_t), 1);
    current_page_dir = page_dir;
    memset(page_dir, 0 , sizeof(page_directory_t));

    u32 addr = 0;
    while(addr < begin_address)
    {
        page_t* page = get_page(page_dir, addr, 1);
        kassert(page != NULL);
        set_page_frame(page, 0, 0);
        addr += 0x1000;
    }

    printk("end address:");
    printk_hex((u32)addr);
    printk("\n");
    
    irq_install_handler(13, (isq_t)(&page_fault_handler));
    
    u32 cr0;
    asm volatile("mov %0, %%cr3":: "r"(&current_page_dir->tableAddress));
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000; // Enable paging!
    asm volatile("mov %0, %%cr0":: "r"(cr0));
    puts("end page init...\n");
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
    //int id = regs->err_code & 0x10;          // Caused by an instruction fetch?

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

