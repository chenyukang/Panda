/*******************************************************************************
 *
 *      @name   : hd.h
 *
 *      @author : Yukang Chen (moorekang@gmail.com)
 *      @date   : 2012-05-28 21:53:54
 *
 *      @brief  :
 *
 *******************************************************************************/


#if !defined(HD_H)
#define HD_H


#include "buf.h"

/* Hard Drive */
#define SECTOR_SIZE		512
#define SECTOR_BITS		(SECTOR_SIZE * 8)
#define SECTOR_SIZE_SHIFT	9


#define HD_CMD  0x1F7
#define HD_STAT 0x1F7

/* commands for HD_CMD */
#define HD_CMD_READ    0x20
#define HD_CMD_WRITE   0x30

/* flags for HD_STAT */
#define HD_BSY	  0x80 //busy
#define HD_DRDY	  0x40 //ready
#define HD_DF     0x20 //fault
#define HD_ERR	  0x01 //error

#define REG_DATA  0x1F0 

struct hd_cmd {
	u8	feature;
	u8	count;
	u8	lba_low;
	u8	lba_mid;
	u8	lba_high;
	u8	device;
	u8	command;
};

struct ide_desc {
    struct buf* m_first;
    struct buf* m_last;
    struct buf* m_io_head;
    struct buf* m_io_tail;
    int         m_active;
};
        

#define MAKE_DEVICE_REG(drv, lba, lba_high)     \
    (((lba << 6) |                              \
    ((drv << 4) |                               \
     (lba_high & 0xF ) | 0xA0)))
    

void ide_init(void);
void hd_rw(struct buf* bp);

#endif

