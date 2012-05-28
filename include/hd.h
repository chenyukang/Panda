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


void init_hd();

#endif

