#include <vm.h>
#include <task.h>
#include <string.h>

u32 vma_init(struct vma* vp, u32 base, u32 size,
             u32 flag, struct inode* ip, u32 ioff) {
    vp->v_flag = flag;
    vp->v_base = base;
    vp->v_size = size;
    vp->v_ioff = ioff;
    if (ip){
        ip->ref_cnt++;
        vp->v_ino = ip;
    }
    return 0;
}

u32 vm_clone(struct vm* to) {
    struct vma* pva;
    int i;

    to->vm_pgd = (struct pde*)alloc_pde();
    init_page_dir(to->vm_pgd);
    for(i=0; i<NVMA; i++) {
        pva = &(current->p_vm.vm_area[i]);
        if(pva->v_flag != 0) {
            to->vm_area[i] = *pva;
        }
    }
    copy_pgd(current->p_vm.vm_pgd, to->vm_pgd);
    return 0;
}

u32 vm_clear(struct vm* vm) {
    struct vma* pva;
    int i;
    for(i=0; i<NVMA; i++) {
        pva = &vm->vm_area[i];
        if(pva->v_flag != 0) {
            if(pva->v_ino)
                idrop(pva->v_ino);
            pva->v_ino  = 0;
            pva->v_flag = 0;
            pva->v_base = 0;
            pva->v_size = 0;
        }
    }
    free_pgd(vm->vm_pgd);
    return 0;
}

u32 vm_renew(struct vm* vm, struct header* header, struct inode* ip) {
    u32 base, text, data, bss, heap;

    // NOTE: keep alignment
    base = header->a_entry - sizeof(struct header);
    text = header->a_entry - sizeof(struct header);
    data = text + header->a_tsize;
    bss  = data + header->a_dsize;
    heap = bss  + header->a_bsize;
    //
    init_page_dir(vm->vm_pgd);
    vm->vm_entry = header->a_entry;
    vm->vm_used_heap = 0;
    vma_init(&(vm->vm_text),  text,  header->a_tsize, VMA_MMAP | VMA_RDONLY | VMA_PRIVATE, ip, text-base);
    vma_init(&(vm->vm_data),  data,  header->a_dsize, VMA_MMAP | VMA_PRIVATE, ip, data-base);
    vma_init(&(vm->vm_bss),   bss,   header->a_bsize, VMA_ZERO | VMA_PRIVATE, NULL, 0);
    vma_init(&(vm->vm_heap),  heap,  PAGE_SIZE*20,       VMA_ZERO | VMA_PRIVATE, NULL, 0);
    vma_init(&(vm->vm_stack), VM_STACK, PAGE_SIZE,    VMA_STACK | VMA_ZERO | VMA_PRIVATE, NULL, 0);
    return 0;
}

u32 vm_verify(u32 vaddr, u32 size) {
    struct pte *pte;
    u32 page;
    if (vaddr<0x8000000 || size<0) {
        return -1;
    }

    for (page=PG_ADDR(vaddr); page<=PG_ADDR(vaddr+size-1); page+=PAGE_SIZE) {
        pte = find_pte(current->p_vm.vm_pgd, page, 1);
        if ((pte->pt_flags & PTE_P)==0) {
            do_no_page((void*)page);
        }
        else if ((pte->pt_flags & PTE_W)==0) {
            do_wt_page((void*)page);
        }
    }
    return 0;
}

struct vma* find_vma(u32 vaddr) {
    struct vma* vma;
    struct vma* vp;

    vma = current->p_vm.vm_area;
    for (vp=&vma[0]; vp<&vma[NVMA]; vp++) {
        if (vaddr >= vp->v_base && vaddr < vp->v_base+vp->v_size) {
            return vp;
        }
    }
    return NULL;
}
