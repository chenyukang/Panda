
#include <vm.h>


u32 vma_init(struct vma* vp, u32 base, u32 size, u32 flag, struct inode* ip, u32 ioff) {
    return 0;
}

u32 vm_clone(struct vm* to) {
    return 0;
}

u32 vm_clear(struct vm* vm) {
    return 0;
}

u32 vm_renew(struct vm* vm, struct header* header, struct inode* ip) {
    return 0;
}

u32 vm_verify(u32 vaddr, u32 size) {
    return 0;
}

struct vma* find_vma(u32 addr) {
    return NULL;
}

