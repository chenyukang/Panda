
/*******************************************************************************
 *
 *      @name   : blk.h
 *
 *      @author : Yukang Chen (moorekang@gmail.com)
 *      @date   : 2012-05-28 23:12:47
 *
 *      @brief  :
 *
 *******************************************************************************/

#if !defined(BLK_H)
#define BLK_H


#include <types.h>

#define NR_REQUEST 32
#define DEVICE_NAME "hard_disk"
#define MAJOR_NR 3

struct request {
    int dev;           //the dev number, -1 is for free status
    int cmd_type;      //write or read
    int error;         //erros happened
    size_t sector;     //sector for start
    size_t nr_sectors; //the number of w/r sectors
    char*  buffer;     //data buffer
    struct request* next;
};

extern struct request requests[NR_REQUEST];
struct request* current_req;

#endif

