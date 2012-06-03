
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
#include <page.h>
#include <string.h>

volatile task_t* current_task;

volatile task_t* task_list;

extern page_dir_t* current_page_dir;
extern u32         init_esp_start;
static u32         next_valid_pid = 0;

void move_stack(void* new_esp_start, u32 size);

void init_task() {
    puts("init task ...\n");
    asm volatile("cli");
    move_stack((void*)0xE0000000, 0x2000);
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

void move_stack(void* new_esp_start, u32 size) {
    u32 i;
    u32 old_esp, old_ebp;
    u32 new_esp, new_ebp;
    u32 offset, dir_phys_addr;
    u32 start = (u32)new_esp_start;
    for( i = start; i >= start-size; i-=0x1000) {
        page_t* addr = get_page(current_page_dir, i, 1);
        set_page_frame(addr, 0, 1);
    }

    // flush TLB
    asm volatile("mov %%cr3, %0" : "=r" (dir_phys_addr));
    asm volatile("mov %0, %%cr3"  :: "r" (dir_phys_addr));
    
    asm volatile("mov %%esp, %0" : "=r" (old_esp));
    asm volatile("mov %%ebp, %0" : "=r" (old_ebp));

    offset  = (u32)new_esp_start -  init_esp_start;
    new_esp = old_esp + offset;
    new_ebp = old_ebp + offset;

    memcpy((void*)new_esp, (void*)old_esp, init_esp_start-old_esp);
    asm volatile("mov %0, %%esp" :: "r" (new_esp));
    asm volatile("mov %0, %%esp" :: "r" (new_ebp));
}
