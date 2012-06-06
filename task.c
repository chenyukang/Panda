
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
volatile task_t* wait_list;

extern u32 read_eip();
extern page_dir_t* current_page_dir;
extern u32         init_esp_start;
u32         next_valid_pid = 0;

void move_stack(void* new_esp_start, u32 size);


int getpid(void) {
    //kassert(current_task);
    printk("current pid: %x\n", current_task);
    return current_task->id;
}
    
void init_task() {
    puts("init task ...\n");
    asm volatile("cli");
    move_stack((void*)0xE0000000, 0x2000);
    current_task =
        (task_t*)kmalloc(sizeof(task_t));
    current_task->id = next_valid_pid++;
    current_task->esp = current_task->ebp = 0;
    current_task->eip = 0;
    current_task->page_dir = current_page_dir;
    current_task->next = 0;
    strcpy((char*)current_task->name, "kernel");
    wait_list = current_task;
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
    for(i=(u32)new_esp_start; i>(u32)(new_esp_start-size); i-=4){
        u32 t = *(u32*)i;
        if((old_esp < t) && (t < init_esp_start)) {
            t += offset;
            u32* addr = (u32*)i;
            *addr = t;
        }
    }
    
    asm volatile("mov %0, %%esp" :: "r" (new_esp));
    asm volatile("mov %0, %%esp" :: "r" (new_ebp));
}


int fork() {

    printk("now in fork\n");
    task_t *parent, *new_task, *t;
    page_dir_t* dir;
    u32 eip, esp, ebp;
    
    parent = (task_t*)current_task;
    dir = copy_page_dir(current_task->page_dir);
    
    asm volatile("cli");
    new_task = (task_t*)kmalloc(sizeof(task_t));

#if 0
repeat:
    if(++next_valid_pid < 0) next_valid_pid = 1;
    for(t=(task_t*)task_list; t->next ; t=t->next){
        if(t->id == next_valid_pid) {
            ++next_valid_pid;
            goto repeat;
        }
    }
#endif
    next_valid_pid++;
    
    new_task->id = next_valid_pid;
    new_task->esp = new_task->ebp = 0;
    new_task->eip = 0;
    new_task->next = 0;
    new_task->page_dir = dir;

    /* put this at the end of task_list */
    t = (task_t*)task_list;
    while(t->next != NULL)
        t = t->next;
    t->next = new_task;

    printk("fork new_task:%x current_task:%x\n",
           new_task, current_task);

    eip = read_eip();

    if(current_task == parent) {
        asm volatile("mov %%esp, %0" : "=r"(esp));
        asm volatile("mov %%ebp, %0" : "=r"(ebp));
        new_task->esp = esp;
        new_task->ebp = ebp;
        new_task->eip = eip;
        asm volatile("sti");
        
        printk("now return child: %x \neip:%x ebp:%x esp:%x dir:%x addr:%x\n",
               new_task, new_task->eip, new_task->ebp, new_task->esp,
               new_task->page_dir, 
               new_task->page_dir->physicalAddr);
        
        return new_task->id;
    } else {
        printk("\nchild return\n");
        return 0;
    }
}

void switch_task() {
    printk("from %d Switch to\n", getpid());
    if(current_task == 0) {
        return;
    }

    u32 esp, ebp, eip;
    asm volatile("mov %%esp, %0" : "=r"(esp));
    asm volatile("mov %%ebp, %0" : "=r"(ebp));

    eip = read_eip();

    if(eip == 0x12345)
        return;

    current_task->eip = eip;
    current_task->esp = esp;
    current_task->ebp = ebp;

    current_task = current_task->next;

    if(!current_task)
        current_task = task_list;

    eip = current_task->eip;
    esp = current_task->esp;
    ebp = current_task->ebp;

    current_page_dir = current_task->page_dir;
    printk("before real switch: %x \neip:%x ebp:%x esp:%x addr:%x\n",
           current_task, eip, ebp,esp, current_task->page_dir->physicalAddr);
    asm volatile("         \
                cli;       \
      mov %0, %%ecx;       \
      mov %1, %%esp;       \
      mov %2, %%ebp;       \
      mov %3, %%cr3;       \
      mov $0x12345, %%eax; \
      sti;                 \
      jmp *%%ecx           "
      :: "r"(eip), "r"(esp), "r"(ebp), "r"(current_page_dir->physicalAddr));
    printk("finish switch: %x\n", current_task);
}

