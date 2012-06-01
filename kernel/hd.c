
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

struct hd_i_struct {
    unsigned int head;
    unsigned int sect;
    unsigned int cyl;
    unsigned int wpcom,lzone,ctl;
};

struct hd_i_struct hd_inf[] = {{0,0,0,0,0,0},
                               {0,0,0,0,0,0}};

static int wait_for_ready(void)
{
    int retries = 10000;
    while( --retries && (inb(HD_STAT) & 0xc0) != 0x40)
        ;
    return retries;
}

void hd_interupt_handler(void)
{
    wait_for_ready();
    return ;
}

void init_hd(void* bios)
{
    /* get the number of divers, from the BIOS data area */
    hd_inf[0].cyl = *(unsigned short*)bios;
    hd_inf[0].head = *(unsigned char*)(2+bios);
    hd_inf[0].wpcom = *(unsigned short*)(5+bios);
    hd_inf[0].ctl   = *(unsigned char*)(8+bios);
    hd_inf[0].lzone = *(unsigned short*)(12+bios);
    hd_inf[0].sect  = *(unsigned char*)(14+bios);
    unsigned int hd_size = (hd_inf[0].head * hd_inf[0].sect * hd_inf[0].cyl);
    printk(" hd_size: %d\n", hd_size);
    printk(" heads: %d\n cyl:%d\n wpcom:%d\n ctl:%d\n lzone:%d\n sect:%d\n",
           hd_inf[0].head, hd_inf[0].cyl, hd_inf[0].wpcom, hd_inf[0].ctl, hd_inf[0].lzone, hd_inf[0].sect);
    irq_install_handler(14, (isq_t)(&hd_interupt_handler));
}
