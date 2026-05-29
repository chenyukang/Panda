#ifndef _VM_H__
#define _VM_H__

#include <aout.h>
#include <inode.h>
#include <mm.h>

/* each vma asscociated with a file descripter, on page fault raised
 * do what it deserves. */
struct vma {
    u32            v_flag;
    u32            v_base; // must be one page aligned
    u32            v_size; // must be one page aligned
    u32            v_ioff; // keep block aligned
    struct inode   *v_ino; // inode
};

#define VMA_RDONLY  0x1  // read only
#define VMA_STACK   0x2  // this vma indicates a stack, which grows downwards.
#define VMA_ZERO    0x4  // demand-zero
#define VMA_MMAP    0x8  // mapped from a file
#define VMA_PRIVATE 0x10 // Copy On Write

#define  SZ_ROUND_UP(addr)   (((addr + 4 - 1) & (-4)))

/*
 * each proc got one struct vm, which indicated it's page directory
 * and misc on address space.
 * */
struct vm {
    struct pde*  vm_pgd;
    u32          vm_entry;
    u32          vm_used_heap;
    struct vma   vm_area[0];  // trick here, treat the fields downblow as an array.
    struct vma   vm_text;
    struct vma   vm_data;
    struct vma   vm_bss;
    struct vma   vm_heap;
    struct vma   vm_stack;
};

#define NVMA 5

#define VM_STACK 0x80000000 /* 2gb */

u32 vma_init(struct vma* vp, u32 base, u32 size, u32 flag, struct inode* ip, u32 ioff);
u32 vm_clone(struct vm* to);
u32 vm_clear(struct vm* vm);
u32 vm_renew(struct vm* vm, struct header* header, struct inode* ip);
u32 vm_verify(u32 vaddr, u32 size);
struct vma* find_vma(u32 addr);


#endif
