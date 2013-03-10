
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
#define PAGE 0x1000

extern struct pde pg_dir0;
struct tss tss;
struct task init_task;
struct task task_demo;
struct task* procs[PROC_NUM];

static u8 kstack0[PAGE] __attribute__((aligned(PAGE)));

task_t* current_task = 0;
struct task_table task_table;

volatile task_t* task_list;
volatile task_t* wait_list;

extern u32 read_eip();
extern void _do_swtch(struct cpu_state* f, struct cpu_state* t);
extern struct pde* cu_pg_dir;

extern u32   init_esp_start;
u32          next_pid_nr = 0;


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

#if 1
static void
init_proc0() {
    struct task* t = current_task = procs[0] = (struct task*)(u32)(kstack0);
    next_pid_nr = 1;
    strcpy(t->name, "init");
    t->pid = 0;
    t->ppid = 0;
    t->pg_dir = &pg_dir0;
    t->stat = WAIT;
    t->next = 0;
    t->cwd  = inode_name("/");
    tss.ss0 = 2<<3;
    tss.esp0 = (u32)t + PAGE;
    task_list = current_task;
}
#endif

void init_multi_task() {
    memset(procs, 0, sizeof(procs));
    init_proc0();
    init_task_table();
}

static u32 find_next_pid() {
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


struct task* spawn(void* func) {
    cli();
    struct task *parent, *new_task;
    parent = current_task;
    new_task = &task_demo;

    new_task->pid = find_next_pid();
    new_task->ppid = parent->pid;
    new_task->pg_dir = current_task->pg_dir;
    strcpy(new_task->name, "proc");

    struct task* t = (task_t*)task_list;
    while(t->next != NULL)
        t = t->next;
    t->next = new_task;
    new_task->cpu_s = parent->cpu_s;
    new_task->cpu_s.eip = (u32)(func);
    new_task->cpu_s.esp = (u32)new_task + PAGE;
    new_task->stat = WAIT;
    sti();
    return new_task;
}

void swtch_to(struct task *to){
    struct task *from;
    tss.esp0 = (u32)to + PAGE; 
    from = current_task;
    current_task = to;
    cu_pg_dir = to->pg_dir;
    flush_pgd(to->pg_dir);
    //printk("switch from: %s to %s\n", from->name, to->name);
    _do_swtch(&(from->cpu_s), &(to->cpu_s));
}

void sched() {
    if(current_task == 0)
        return;
    struct task* next = current_task->next;
    
    while(!(next->stat == CREATED ||
            next->stat == RUNNING))
        next = next->next;
    
    if(next != current_task) {
        swtch_to(next);
    }
}

void sleep(void* change, struct spinlock* lock) {
    if(current_task == 0) {
        PANIC("sleep: no task");
    }
    if(lock == 0) {
        PANIC("sleep: no lock");
    }
    current_task->chan = change;
    current_task->stat = WAIT;
    sched();
}

void wakeup(void* change) {
    kassert(change);
    struct task* t;
    t = current_task;
}



