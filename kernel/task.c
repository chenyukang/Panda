
/*******************************************************************************
 *
 *      @name   : task.c
 *
 *      @author : Yukang Chen (moorekang@gmail.com)
 *      @date   : 2012-06-02 01:20:55
 *
 *      @brief  :
 *
 *******************************************************************************/

#include <types.h>
#include <task.h>
#include <kheap.h>
#include <string.h>

volatile task_t* current_task;

volatile task_t* task_list;

static u32 next_valid_pid = 0;

void init_task() {
    printk("init task...\n");
    asm volatile("cli");
    next_valid_pid++;
    asm volatile("sti");
}
