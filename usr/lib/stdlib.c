#include <syscall.h>
#include <stdlib.h>

char getchar() {
    int cnt;
    char buf[1024];
    while((cnt = read(0, buf, 1) > 0)) {
        return buf[0];
    }
    return 0;
}


typedef long Align ;

union header {
    struct {
        union header* ptr;
        u32  size;
    }s;
    Align x; //this is never used!
};

typedef union header Header;

static Header base;
static Header *freep;

void free(void* ap) {
    Header *bp, *p;
    bp = (Header*)ap - 1; /* point back to header */
    p = freep;

    /* try to find a good position to insert free node,
       do not worry this will get into a infinite loop,
       at the end of free list, the cond of break will
       always confirom */
    for(p=freep; !(p->s.ptr >= bp && bp >= p); p = p->s.ptr) {
        if( p >= p->s.ptr ) {     /* this is end */
            if( bp > p ||          /* insert at end */
                bp < p->s.ptr )   /* insert at start */
                break;
        }
    }

    /*now p hold the postion for insert */
    if( bp + bp->s.size == p->s.ptr) { /* join upper */
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    } else
        bp->s.ptr = p->s.ptr;

    if( p + p->s.size == bp ) { /* join lower */
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    } else
        p->s.ptr = bp;

    freep = p;
}

static Header* morecore(u32 nu) {
    char* p;
    Header* hp;
    if(nu < 10)
        nu = 10;
    p = (char*)sbrk(nu * sizeof(Header));
    if( p == (char*)-1)
        return 0;
    hp = (Header*)p;
    hp->s.size = nu;
    free((void*)(hp + 1));
    return freep;
}

void* malloc(u32 nbytes) {
    Header *p, *prev;
    u32 nunits;
    nunits = (nbytes + sizeof(Header)-1) / sizeof(Header) + 1;

    if( (prev = freep ) == NULL ) {
        prev = freep = base.s.ptr = &base;
        base.s.size = 0;
    }
    for( p = prev->s.ptr; ; prev=p, p=p->s.ptr ) {
        if(p->s.size >= nunits ) {
            if (p->s.size == nunits )
                prev->s.ptr = p->s.ptr;
            else {
                p->s.size -= nunits ;
                p += p->s.size;
                p->s.size = nunits;
            }
            freep = prev;
            return (void*)(p+1);
        }
        if ( p == freep ){
            if ((p = morecore(nunits)) == NULL) {
                return NULL;
            }
        }
    }
}
