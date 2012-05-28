
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

void init_hd()
{
    /* get the number of divers, from the BIOS data area */
    u8* num_of_driver = (u8*)(0x475);
    printk("NrDrivers: %d\n", *num_of_driver);

    irq_install_handler(14, (isq_t)(&hd_interupt_handler));
}
