
/*******************************************************************************
 *
 *      @name   : task.c
 *
 *      @author : Yukang Chen (moorekang@gmail.com)
 *      @date   : 2012-06-02 01:20:55
 *
 *      @brief  : task implement 
 *
 *******************************************************************************/

#include <types.h>
#include <task.h>
#include <kheap.h>
#include <string.h>

volatile task_t* current_task;

volatile task_t* task_list;

extern page_dir_t* current_page_dir;
static u32 next_valid_pid = 0;

void init_task() {
    printk("init task...\n");
    asm volatile("cli");
    current_task = task_list =
        (task_t*)kmalloc(sizeof(task_t));
    current_task->id = next_valid_pid++;
    current_task->esp = current_task->ebp = 0;
    current_task->eip = 0;
    current_task->page_dir = current_page_dir;
    current_task->next = 0;
    strcpy((char*)current_task->name, "kernel");
    asm volatile("sti");
}
