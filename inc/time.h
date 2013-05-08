
/*******************************************************************************
 *
 *      @name   : TIME_H
 *
 *      @author : Yukang Chen (moorekang@gmail.com)
 *      @date   : 2012-07-09 21:58:04
 *
 *      @brief  :
 *
 *******************************************************************************/



#if !defined(TIME_H)
#define TIME_H

#include <string.h>
 
#define outb_p(value,port)                      \
  __asm__ ("outb %%al,%%dx\n"                   \
           "\tjmp 1f\n"                         \
           "1:\tjmp 1f\n"                       \
           "1:"::"a" (value),"d" (port))
 
#define inb_p(port) ({                                  \
          unsigned char _v;                             \
          __asm__ volatile ("inb %%dx,%%al\n"           \
                            "\tjmp 1f\n"                \
                            "1:\tjmp 1f\n"              \
                            "1:":"=a" (_v):"d" (port)); \
          _v;                                           \
      })
                       
typedef long time_t;

#define MINUTE 60
#define HOUR   (60*MINUTE)
#define DAY    (24*HOUR)
#define YEAR   (365*DAY)

struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_day;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
    int tm_centry;
};

time_t kern_setup_time;
struct tm kern_time;

#define CMOS_READ(addr) ({                      \
            outb_p(addr, 0x70);                 \
            inb_p(0x71);                        \
        })

#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)

long kernel_mktime(struct tm * tm);
void time_init(void);
void print_time_local(void);
void update_time();

#endif

