
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

struct hd_i_struct {
    unsigned int head;
    unsigned int sect;
    unsigned int cyl;
    unsigned int wpcom,lzone,ctl;
};

static struct buf* ide_queue;

struct hd_i_struct hd_inf[] = {{0,0,0,0,0,0},
                               {0,0,0,0,0,0}};

static int waitfor_ready(int check_error) {
    int retries = 1000;
    int r;
    while( --retries ) {
        r = inb(IDE_STAT);
        if ( (r & (IDE_BUSY | IDE_READY)) == IDE_READY ) {
            break;
        }
    }
    if(check_error & (( r & (IDE_DF|IDE_ERROR)) != 0))
        return -1;
    return 0;
}

void do_hd_cmd(struct hd_cmd* cmd) {
    waitfor_ready(0);
    outb(0x3F6, 0); //interrupt
    /* Activate the Interrupt Enable (nIEN) bit */
	outb(0x1F1,  cmd->feature);
	outb(0x1F2,  cmd->count);
	outb(0x1F3,  cmd->lba_low);
	outb(0x1F4,  cmd->lba_mid);
	outb(0x1F5,  cmd->lba_high);
	outb(0x1F6,  cmd->device);
	/* Write the command code to the Command Register */
	outb(0x1F7,  cmd->command);
}


void set_ready(struct buf* pb) {
}

void ide_start(struct buf* pb) {
    u32         lba;
    if(pb == 0) {
        PANIC("ide_start for null buf");
    }
    struct hd_cmd cmd;
    if(pb->b_flag & B_READ)
        cmd.command = CMD_READ;
    else cmd.command = CMD_WRITE;
    lba = pb->b_sector * BLK / PBLK;
    cmd.feature = 0;
    cmd.lba_low = lba & 0xFF;
    cmd.lba_mid = (lba >> 8) & 0xFF ;
    cmd.lba_high = (lba >> 16) & 0xFF;
    cmd.count = BLK/PBLK;
    cmd.device = 0xE0 | ((lba >> 24) &0x0F); //alway for drive 0
    do_hd_cmd(&cmd);
    if(pb->b_flag & B_WRITE)
        outsl(0x1F0, pb->b_data, BLK/4);
}

void hd_interupt_handler(void) {
    waitfor_ready(1);
    struct buf* b = ide_queue;
    if(b == 0) {
        return;
    }
    ide_queue = b->b_next;
    if( b->b_flag & B_READ )
        insl(0x1F0, b->b_data, BLK/4);
    set_ready(b);
    if(ide_queue) {
        ide_start(ide_queue);
    }
}

void hd_rw(struct buf* bp) {
    if(!(bp->b_flag & B_BUSY))
        PANIC("hd_rw: buf is not busy");
    if(bp->b_dev != 0)
        PANIC("hd_rw: error device number");
    bp->b_next = 0;
    struct buf* p = ide_queue;
    if( p == 0 ) {
        ide_queue = p;
    } else {
        while(p->b_next != 0)  p = p->b_next;
        p->b_next = bp;
    }
    if(ide_queue == bp) {
        ide_start(bp);
    }
}

#if 0
static void print_identify_info(u16* hdinfo) {
	int i;
    unsigned k;
	char s[64];

	struct iden_info_ascii {
		int idx;
		int len;
		char * desc;
	} iinfo[] = {{10, 20, "HD SN"}, /* Serial number in ASCII */
                 {27, 40, "HD Model"} /* Model number in ASCII */ };

	for (k = 0; k < sizeof(iinfo)/sizeof(iinfo[0]); k++) {
		char * p = (char*)&hdinfo[iinfo[k].idx];
		for (i = 0; i < iinfo[k].len/2; i++) {
			s[i*2+1] = *p++;
			s[i*2] = *p++;
		}
		s[i*2] = 0;
		printk("%s: %s\n", iinfo[k].desc, s);
	}

	int capabilities = hdinfo[49];
	printk("LBA supported: %s\n",
	       (capabilities & 0x0200) ? "Yes" : "No");

	int cmd_set_supported = hdinfo[83];
	printk("LBA48 supported: %s\n",
	       (cmd_set_supported & 0x0400) ? "Yes" : "No");

	int sectors = ((int)hdinfo[61] << 16) + hdinfo[60];
	printk("SECTORS: %d HD size: %d MB\n", sectors, sectors * 512 / 1000000);
}
#endif


void init_hd() {
    void* bios = (void*)0x90080;
    /* get the number of divers, from the BIOS data area */
    hd_inf[0].cyl   = *(u16*)bios;
    hd_inf[0].head  = *(u8*)(2+bios);
    hd_inf[0].wpcom = *(u16*)(5+bios);
    hd_inf[0].ctl   = *(u8*)(8+bios);
    hd_inf[0].lzone = *(u16*)(12+bios);
    hd_inf[0].sect  = *(u8*)(14+bios);

    u32 hd_size = (hd_inf[0].head * hd_inf[0].sect * hd_inf[0].cyl);
    printk("hd_size: %d KB\n", hd_size/1024);

#if 0
    printk("heads:%d\ncyl:%d\nwpcom:%d\nctl:%d\nlzone:%d\nsect:%d\n",
           hd_inf[0].head, hd_inf[0].cyl, hd_inf[0].wpcom,
           hd_inf[0].ctl, hd_inf[0].lzone, hd_inf[0].sect);
#endif
    
    irq_install_handler(14, (isq_t)(&hd_interupt_handler));

    struct hd_cmd cmd;
    cmd.device = MAKE_DEVICE_REG(0, 0, 0);
    do_hd_cmd(&cmd);
    waitfor_ready(0);
}

void init_ide() {
    printk("init_ide ...\n");
    init_hd();
    irq_install_handler(14, (isq_t)(&hd_interupt_handler));
    //waitfor_ready(0);
    ide_queue = 0;
}
