
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
#include <asm.h>
#include <string.h>

struct task procs[126];

task_t* current_task;

volatile task_t* task_list;
volatile task_t* wait_list;

extern u32 read_eip();

extern struct pde* cu_pg_dir;

extern u32         init_esp_start;
u32                next_valid_pid = 0;

int getpid(void) {
    kassert(current_task);
    return current_task->pid;
}

char* get_current_name() {
    kassert(current_task);
    return (char*)current_task->name;
}

void init_task() {
    puts("init task ...\n");
    current_task = &procs[0];
#if 0
    move_stack(current_task, (void*)0xE0000000);

    current_task->pid = next_valid_pid++;
    current_task->esp = current_task->ebp = 0;
    current_task->eip = 0;
    current_task->pg_dir = (u32)cu_pg_dir;
    current_task->next = 0;
    strcpy((char*)current_task->name, "kernel");
#endif
    task_list = current_task;
}

void move_stack(task_t* task, void* new_esp_start) {
    u32 i;
    u32 old_esp, old_ebp;
    u32 new_esp, new_ebp;
    u32 offset;
    u32 size = 0x1000;
    struct page* pg = alloc_page();
    task->stack_base = ((u32)pg)<<10;

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
    asm volatile("mov %0, %%ebp" :: "r" (new_ebp));
}


#if 0
int fork() {
    cli();
    task_t *parent, *new_task, *t;
    u32 eip;
    
    page_dir_t* dir = copy_page_dir(current_task->page_dir);
    parent = ( task_t*)current_task;
    new_task = (task_t*)kmalloc(sizeof(task_t));

repeat:
    if(++next_valid_pid < 0) next_valid_pid = 1;
    for(t=(task_t*)task_list; t->next ; t=t->next){
        if(t->pid == next_valid_pid) {
            ++next_valid_pid;
            goto repeat;
        }
    }
    
    new_task->pid = next_valid_pid++;
    new_task->esp = new_task->ebp = 0;
    new_task->eip = 0;
    new_task->next = 0;
    new_task->page_dir = dir;
    strcpy(new_task->name, "proc");

    t = (task_t*)task_list;
    while(t->next != NULL)
        t = t->next;
    t->next = new_task;

    eip = read_eip();
    if(current_task == parent) { //this is parent
        u32 esp, ebp;
        asm volatile("mov %%esp, %0" : "=r"(esp));
        asm volatile("mov %%ebp, %0" : "=r"(ebp));
        new_task->esp = esp;
        new_task->ebp = ebp;
        new_task->eip = eip;
        printk("set new_task: %x\n", eip);
        sti();
        return new_task->pid;
    }
    else {
        printk("child return\n");
        return 0;
    }
}

void switch_task() {
#if 0
    printk("from %d(%s) %x ==> Switch to\n", getpid(),
           get_current_name(), current_task);
#endif

    if(current_task == 0)
        return;
    kassert(current_task);

    u32 eip;
    u32 ebp, esp;

    asm volatile("mov %%esp, %0" : "=r"(esp));
    asm volatile("mov %%ebp, %0" : "=r"(ebp));

    eip = read_eip();
    if(eip == 0x12345)
        return;

    current_task->eip = eip;
    current_task->esp = esp;
    current_task->ebp = ebp;

    //find the next task to run
    task_t* next = current_task->next;
    if(next == 0)
        next = (task_t*)task_list;

    if(current_task == next)
        return;
    
    current_task = next;
    current_page_dir = current_task->page_dir;

    printk("real switch (%s): %x \neip:%x ebp:%x esp:%x addr:%x\n",
           (char*)current_task->name,current_task,
           current_task->eip, ebp, esp, current_task->page_dir->physicalAddr);

    asm volatile("         \
                cli;       \
        mov %0, %%ecx;     \
        mov %1, %%esp;     \
        mov %2, %%ebp;       \
        mov %3, %%cr3;       \
        mov $0x12345, %%eax; \
        sti;                 \
        jmp *%%ecx           "
                 :: "r"(current_task->eip), "r"(current_task->esp),
                  "r"(current_task->ebp), "r"(current_page_dir->physicalAddr));
}

#endif
