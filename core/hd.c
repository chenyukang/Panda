/*******************************************************************************
 *
 *      @name   : hd.c
 *
 *      @author : Yukang Chen (moorekang@gmail.com)
 *      @date   : 2012-05-28 21:51:02
 *
 *      @brief  :
 *
 *******************************************************************************/

#include <asm.h>
#include <task.h>
#include <system.h>
#include <string.h>
#include <hd.h>

#define IDE_STAT  0x1F7
#define IDE_BUSY  0x80
#define IDE_READY 0x40
#define IDE_ERROR 0x01
#define IDE_DF    0x20

#define CMD_READ  0x20
#define CMD_WRITE 0x30

struct hd_struct {
    u32 head;
    u32 sect;
    u32 cyl;
    u32 wpcom;
    u32 lzone;
    u32 ctl;
};

static int havedisk1;
static struct buf* ide_queue;
static struct spinlock hdlock;

/* disk details, haven't used now */
struct hd_struct hd_inf[] = { {0,0,0,0,0,0},
                              {0,0,0,0,0,0} };

static int waitfor_ready(int check_error) {
    int retries = 1000;
    int r;
    while( --retries ) {
        r = inb(IDE_STAT);
        if ((r & (IDE_BUSY | IDE_READY)) == IDE_READY ) {
            break;
        }
    }
    if(check_error & ((r & (IDE_DF | IDE_ERROR)) != 0))
        return -1;
    return 0;
}

/* Start the request for buf.  Caller must hold idelock. */
static void ide_start(struct buf *b) {
  if(b == 0)
    PANIC("ide_start");

  waitfor_ready(1);
  /* generate interrupt */
  outb(0x3f6, 0);
  /* number of sectors */
  outb(0x1f2, 1);
  outb(0x1f3, b->b_sector & 0xff);
  outb(0x1f4, (b->b_sector >> 8) & 0xff);
  outb(0x1f5, (b->b_sector >> 16) & 0xff);
  outb(0x1f6, 0xe0 | ((b->b_dev&1)<<4) | ((b->b_sector>>24) & 0x0f));
  if(b->b_flag & B_DIRTY) {
      outb(0x1f7, CMD_WRITE);
      outsl(0x1f0, b->b_data, 512/4);
  } else {
      outb(0x1f7, CMD_READ);
  }
}

void hd_interupt_handler(void) {
    waitfor_ready(1);
    struct buf* bp = ide_queue;
    if(bp == 0) {
        return;
    }

    ide_queue = bp->b_qnext;
    if(!(bp->b_flag & B_DIRTY) && waitfor_ready(1) >= 0){
        insl(0x1F0, bp->b_data, 512/4);
    }

    bp->b_flag |= B_VALID;
    bp->b_flag &= ~B_DIRTY;
    do_wakeup(bp);

    if(ide_queue) {
        ide_start(ide_queue);
    }
}

void hd_rw(struct buf* bp) {
    if(!(bp->b_flag & B_BUSY))
        PANIC("hd_rw: buf is not busy");
    if(bp->b_dev != 0 && havedisk1 == 0)
        PANIC("hd_rw: error device number");

    bp->b_qnext = 0;
    struct buf* p = ide_queue;
    if( p == 0 ) {
        ide_queue = bp;
    } else {
        while(p->b_qnext != 0)  p = p->b_qnext;  //put at the end
        p->b_qnext = bp;
    }
    if(ide_queue == bp) {
        cli();
        ide_start(bp);
    }
    //NOTE, we need to sti after sleep, since ide_start
    //may cause a interupt before this, for example Qemu
    while((bp->b_flag & (B_VALID | B_DIRTY)) != B_VALID) {
        do_sleep(bp, &hdlock);
        sti();
    }
}

void init_hd() {
    int i;
    init_lock(&hdlock, "disklock");
    void* bios = (void*)0x90080;

    /* get the number of divers, from the BIOS data area */
    hd_inf[0].cyl   = *(u16*)bios;
    hd_inf[0].head  = *(u8*)(2+bios);
    hd_inf[0].wpcom = *(u16*)(5+bios);
    hd_inf[0].ctl   = *(u8*)(8+bios);
    hd_inf[0].lzone = *(u16*)(12+bios);
    hd_inf[0].sect  = *(u8*)(14+bios);

    irq_enable(14);
    irq_install(32+14, (isq_t)(&hd_interupt_handler));
    waitfor_ready(0);

    /* to check if disk 1 is present */
    outb(0x1f6, 0xe0 | (1<<4));
    for(i=0; i<10; i++){
        if(inb(0x1f7) != 0) {
            havedisk1 = 1;
            break;
        }
    }
    // Switch back to disk 0.
    outb(0x1f6, 0xe0 | (0<<4));

    waitfor_ready(0);
    ide_queue = 0;
}

void ide_init() {
    printk("[hd]   .... ");
    init_hd();
    done();
}
