
/*******************************************************************************
 *
 *      @name   : mutex.c
 *
 *      @author : Yukang Chen (moorekang@gmail.com)
 *      @date   : 2012-08-22 07:57:49
 *
 *      @brief  :
 *
 *******************************************************************************/

#include <mutex.h>

void mutex_init( struct mutex * m) {
    m->foo = 0;
}
