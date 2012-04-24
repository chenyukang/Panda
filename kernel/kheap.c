
// @Name   : sortlink.c 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-03-20 05:38:33
//
// @Brief  :

#include <kheap.h>
#include <page.h>
#include <string.h>

#define MINI_NALLOC 1024

u32 kheap_start_addr = KHEAP_START_ADDR;
static Header base;
static Header* freep = NULL;


void kfree(void* ap)
{
    printk("now in kfree: %d\n", (unsigned)ap);
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


static
void* __real_get_mem(u32 size)
{
    void* addr = (void*)kheap_start_addr;
    kheap_start_addr += size;
    return addr;
}

static Header* get_more(u32 size)
{
    printk("now in get_more: %d\n", size);
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

void* kmalloc(u32 nbytes)
{
    Header *p, *prev;
    u32 nunits;

    printk("now in kmalloc: %d\n", nbytes);
    nunits = (nbytes+sizeof(Header)-1) / sizeof(Header) + 1;
    if( (prev = freep ) == NULL ) {
        prev = freep = base.s.next = &base;
        base.s.size = 0;
    }
    for( p = prev->s.next; ; prev=p, p=p->s.next) {
        if( p->s.size >= nunits ) { //big enough
            if (p->s.size == nunits )
                prev->s.next = p->s.next;
            else {
                p->s.size -= nunits ;
                p += p->s.size;
                p->s.size = nunits;
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

