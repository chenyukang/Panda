
/*******************************************************************************
 *
 *      @name   : task.c
 *
 *      @author : Yukang Chen (moorekang@gmail.com)
 *      @date   : 2012-06-02 01:20:55
 *
 *      @brief  : multi-tasking implement 
 *
 *******************************************************************************/

#include <types.h>
#include <task.h>
#include <kheap.h>
#include <page.h>
#include <asm.h>
#include <string.h>

#define PROC_NUM 126

extern struct pde pg_dir0;
struct task init_task;
struct task task_demo;
struct task* procs[PROC_NUM];

task_t* current_task = 0;
struct task_table task_table;

volatile task_t* task_list;
volatile task_t* wait_list;

extern u32 read_eip();
extern struct pde* cu_pg_dir;

extern u32   init_esp_start;
u32          next_pid_nr = 0;

struct tss ktss;

int getpid(void) {
    kassert(current_task);
    return current_task->pid;
}

char* get_current_name() {
    kassert(current_task);
    return (char*)current_task->name;
}

static void
init_task_table() {
    mutex_init(&(task_table.lock));
    mutex_lock(task_table.lock);

    task_table.head = &(init_task);
    mutex_unlock(task_table.lock);
}

static void
init_proc() {
    struct task* t = current_task = procs[0] = &init_task;
    next_pid_nr = 1;

    strcpy(t->name, "init");
    t->pid  = 0;
    t->ppid = 0;
    t->pg_dir = &pg_dir0;
    t->stat = WAIT;
    //t->tss.ss0 = (2<<3);
    //t->tss.esp0 = 0x1000;
    t->eip = 0;
    t->ebp = 0;
    t->next = 0;
    //move_stack(t, (void*)0x0040000);
    task_list = current_task;
}


void init_multi_task() {
    init_proc();
    init_task_table();
}


#if 0
void init_task() {
    puts("init task ...\n");
    current_task = &procs[0];

    move_stack(current_task, (void*)0xE0000000);

    current_task->pid = next_valid_pid++;
    current_task->esp = current_task->ebp = 0;
    current_task->eip = 0;
    current_task->pg_dir = (u32)cu_pg_dir;
    current_task->next = 0;
    strcpy((char*)current_task->name, "init");

    task_list = current_task;
}
#endif

void move_stack(task_t* task, void* new_esp_start) {
    u32 i;
    u32 old_esp, old_ebp;
    u32 new_esp, new_ebp;
    u32 offset;
    u32 size = 0x1000;
    //struct page* pg = alloc_page();
    //flush_pgd(task->pg_dir);
    
    //task->stack_base = ((u32)pg)<<10; //stack_base is top of this page

    asm volatile("mov %%esp, %0" : "=r" (old_esp));
    asm volatile("mov %%ebp, %0" : "=r" (old_ebp));

    offset  = (u32)new_esp_start -  init_esp_start; //copy stack
    new_esp = old_esp + offset;
    new_ebp = old_ebp + offset;
    //size = init_esp_start - old_esp;
    printk("begin to copy memory:%x %x %x\n", old_esp, old_ebp, init_esp_start);
    memcpy((void*)new_esp, (void*)old_esp,  init_esp_start-old_esp);
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
    printk("now new_esp: %x\n", new_esp);
    printk("finsh move_stack\n");
}

static u32
find_next_pid() {
    u32 ret = 0;
    task_t* t;
repeat:
    if(++next_pid_nr < 0) next_pid_nr = 1;
    for(t=(task_t*)task_list; t->next ; t=t->next){
        if(t->pid == next_pid_nr) {
            ++next_pid_nr;
            goto repeat;
        }
    }
    ret = next_pid_nr++;
    return ret;
}

int fork() {
    cli();
    task_t *parent, *new_task, *t;
    u32 eip;

    //struct pde* new_pgdir = (struct pde*)alloc_page();
    //copy_pgd(current_task->pg_dir, new_pgdir);
    parent = ( task_t*)current_task;
    new_task = (task_t*)(&task_demo);

    new_task->pid = find_next_pid();
    new_task->esp = new_task->ebp = 0;
    new_task->eip = 0;
    new_task->next = 0;
    new_task->pg_dir = current_task->pg_dir;
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
        sti();
        return new_task->pid;
    }
    else {
        return 0; //child return
    }
}

void switch_task() {
#if 0
    printk("from %d(%s) %x ==> Switch to\n", getpid(),
           get_current_name(), current_task);
#endif

    if(current_task == 0)
        return;

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
    cu_pg_dir = current_task->pg_dir;

    eip = current_task->eip;
    esp = current_task->esp;
    ebp = current_task->ebp;
    
#if 1
    printk("real switch (%s): %x \neip:%x ebp:%x esp:%x addr:%x\n",
           (char*)current_task->name, current_task,
           current_task->eip, ebp, esp, current_task->pg_dir);
#endif
    asm volatile("         \
                cli;       \
        mov %0, %%ecx;     \
        mov %1, %%esp;     \
        mov %2, %%ebp;       \
        mov %3, %%cr3;       \
        mov $0x12345, %%eax; \
        sti;                 \
        jmp *%%ecx           "
                 : : "r"(eip), "r"(esp),
                 "r"(ebp), "r"((u32)cu_pg_dir));
    flush_pgd(cu_pg_dir);
}
