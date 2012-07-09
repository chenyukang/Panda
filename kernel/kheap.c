// @Name   : kheap.c
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-03-20 05:38:33
//
// @Brief  : this is according to K&R, simple and elegant
//           align is malloc memory align to 0x1000

#include <kheap.h>
#include <page.h>
#include <string.h>

#define MINI_NALLOC 1024

static u32 kheap_start_addr;
static u32 kheap_end_addr;
static Header base;
static Header* freep = NULL;
u32 kheap_inited = 0;

void kheap_init(void* start_addr, void* end_addr) {
    kheap_start_addr = (u32)start_addr;
    kheap_end_addr = (u32)end_addr;
    freep = NULL;
    kheap_inited = 1;
}

void kfree(void* ap)
{
    Header *bp, *p;
    bp = (Header*)ap - 1; /* point back to header */
    p = freep;

    /* try to find a good position to insert free node,
       do not worry this will get into a infinite loop,
       at the end of free list, the cond of break will
       always confirom */
    for(p=freep; !(p->s.next >= bp && bp >= p); p=p->s.next) {
        if( p >= p->s.next ) {     /* this is end */
            if( bp > p ||          /* insert at end */
                bp < p->s.next )   /* insert at start */
                break;
            }
        }
    
    /*now p hold the postion for insert */
    if( bp + bp->s.size == p->s.next) { /* join upper */
        bp->s.size += p->s.next->s.size;
        bp->s.next = p->s.next->s.next;
    } else
        bp->s.next = p->s.next;

    if( p + p->s.size == bp ) { /* join lower */
        p->s.size += bp->s.size;
        p->s.next = bp->s.next;
    } else
        p->s.next = bp;

    freep = p;
}


static void* __real_get_mem(u32 size) {
    asm volatile("cli");    
    if(kheap_start_addr + size < kheap_end_addr) {
        void* addr = (void*)kheap_start_addr;
        kheap_start_addr += size;
        asm volatile("sti");
        return addr;
    } else {
        puts("out of kheap memory\n");
        asm volatile("sti");
        return NULL;
    }
}

static Header* get_more(u32 size)
{
    char* mem = NULL;
    Header* head = NULL;
    if ( size < MINI_NALLOC )
        size = MINI_NALLOC;
    mem = (char*)(__real_get_mem( size * sizeof(Header) ));
    if ( mem == NULL )
        return NULL;

    head = (Header*)mem;
    head->s.size = size;
    kfree((void*)(head + 1));
    return freep;
}


inline int valid_align(int align, u32 location) {
    if(align == 0)
        return 1;
    return (location % 0x1000) == 0;
}

/* notes: align is default return addr align 0x1000,
   this may cost more time for search a free slot,
   and do not call this align=1 requently */
void* kmalloc_align(u32 nbytes, u32 align) {
    Header *p, *prev;
    u32 nunits;
    u32 offset;
    u32 old_size;
    u32 loc;

    nunits = (nbytes+sizeof(Header)-1) / sizeof(Header) + 1;
    
    if( (prev = freep ) == NULL ) {
        prev = freep = base.s.next = &base;
        base.s.size = 0;
    }
    for( p = prev->s.next; ; prev=p, p=p->s.next ) {
        loc = (u32)(p+1);
        if(!valid_align(align, loc)) { //need align, recompute nunits
            offset = 0x1000 - (loc%0x1000);
            nunits = (nbytes + offset + sizeof(Header) - 1) /
                     sizeof(Header) + 1;
        }
        if(p->s.size >= nunits ) {
            if(valid_align(align, loc)) { //aligned
                if (p->s.size == nunits )
                    prev->s.next = p->s.next;
                else {
                    p->s.size -= nunits ;
                    p += p->s.size;
                    p->s.size = nunits;
                }
                freep = prev;
                return (void*)(p+1);
            } else { //not aligned, skip offset
                old_size = p->s.size;
                p->s.size = offset/sizeof(Header); //need check?
                p += (offset/sizeof(Header));      //just skip
                p->s.size = old_size - offset/sizeof(Header);
                freep = prev;
                return (void*)(p+1);
            }
            freep = prev;
            return (void*)(p+1);
        }

        if ( p == freep ){
            if ((p = get_more(nunits)) == NULL) {
                return NULL;
            }
        }
    }
}


void* kmalloc(u32 nbytes)
{
    return kmalloc_align(nbytes, 0);
}

