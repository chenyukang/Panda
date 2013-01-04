
/*******************************************************************************
 *
 *      @name   : MUTEX_H
 *
 *      @author : Yukang Chen (moorekang@gmail.com)
 *      @date   : 2012-08-22 07:50:40
 *
 *      @brief  :
 *
 *******************************************************************************/

#if !defined(MUTEX_H)
#define MUTEX_H

#include <types.h>

struct mutex {
	u32 foo;
};

void mutex_init( struct mutex * m);

// save the CPU flags and disable interrupts locally 
#define mutex_lock( m )                             \
	u32 flags;                                      \
	ASM( "pushfl" ::: "memory" );                   \
	ASM( "popl %0" : "=g" ( flags ) :: "memory" );  \
	cli();                                          \


// restore the flags 
#define mutex_unlock( m )                           \
    ASM( "pushl %0" :: "g" ( flags ): "memory" );   \
    ASM( "popfl" ::: "memory" );                    \

#endif

