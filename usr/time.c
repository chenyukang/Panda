
/*******************************************************************************
 *
 *      time.c
 *
 *      @brief
 *
 *      @author   Yukang Chen  @date  2013-05-07 15:37:06
 *
 *
 *******************************************************************************/

#include <time.h>
#include <string.h>
#include <syscall.h>

void print_time(struct tm* t) {
    printf("%d-%d-%d %d:%d:%d\n",
           t->tm_year, t->tm_mon, t->tm_day,
           t->tm_hour, t->tm_min, t->tm_sec);
}


int main() {
    struct tm t;
    time(&t);
    print_time(&t);
    return 0;
}
