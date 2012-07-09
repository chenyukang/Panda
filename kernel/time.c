
/*******************************************************************************
 *
 *      @name   : time.c
 *
 *      @author : Yukang Chen (moorekang@gmail.com)
 *      @date   : 2012-07-09 22:02:26
 *
 *      @brief  :
 *
 *******************************************************************************/


#include <time.h>

/* interestingly, we assume leap-years */
static int month[12] = {
    0,
    DAY*(31),
    DAY*(31+29),
    DAY*(31+29+31),
    DAY*(31+29+31+30),
    DAY*(31+29+31+30+31),
    DAY*(31+29+31+30+31+30),
    DAY*(31+29+31+30+31+30+31),
    DAY*(31+29+31+30+31+30+31+31),
    DAY*(31+29+31+30+31+30+31+31+30),
    DAY*(31+29+31+30+31+30+31+31+30+31),
    DAY*(31+29+31+30+31+30+31+31+30+31+30)
};


long kernel_mktime(struct tm * tm) {
    long res;
    int year;
    year = tm->tm_year - 1970;
    /* magic offsets (y+1) needed to get leapyears right.*/
    res = YEAR*year + DAY*((year+1)/4);
    res += month[tm->tm_mon];
    /* and (y+2) here. If it wasn't a leap-year, we have to adjust */
    if (tm->tm_mon>1 && ((year+2)%4))
        res -= DAY;
    res += DAY*(tm->tm_mday-1);
    res += HOUR*tm->tm_hour;
    res += MINUTE*tm->tm_min;
    res += tm->tm_sec;
    return res;
}

void time_init(void) {
    struct tm time;
    unsigned centry;
    do {
        time.tm_sec  = CMOS_READ(0);
        time.tm_min  = CMOS_READ(2);
        time.tm_hour = CMOS_READ(4);
        time.tm_mday = CMOS_READ(7);
        time.tm_mon  = CMOS_READ(8);
        time.tm_year = CMOS_READ(9); //year 1980~2099
        time.tm_centry = CMOS_READ(0x32);
    } while (time.tm_sec != CMOS_READ(0));
    BCD_TO_BIN(time.tm_sec);
    BCD_TO_BIN(time.tm_min);
    BCD_TO_BIN(time.tm_hour);
    BCD_TO_BIN(time.tm_mday);
    BCD_TO_BIN(time.tm_mon);
    BCD_TO_BIN(time.tm_year);
    BCD_TO_BIN(time.tm_centry);
    
    time.tm_year = time.tm_centry*100 + time.tm_year;
    kern_setup_time = kernel_mktime(&time);
    kern_time = time;
    print_time_local();
}

void print_time_local(void) {
    printk("kernel: %x\n", kern_setup_time);
    printk("year:%d month:%d day:%d hour:%d min:%d sec:%d\n",
           kern_time.tm_year, kern_time.tm_mon, kern_time.tm_mday,
           kern_time.tm_hour+8, kern_time.tm_min, kern_time.tm_sec);
}    
