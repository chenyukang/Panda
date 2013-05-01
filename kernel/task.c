
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
#include <gdt.h>

#define PROC_NUM 126

struct {
    struct spinlock lock;
    struct task* procs[PROC_NUM];
} proc_table;

struct tss_desc tss;

extern struct pde  pg_dir0;
extern struct pde* cu_pg_dir;
struct task*       current_task = 0;
extern u32         init_esp_start;

extern void _do_swtch(struct jmp_buf* from, struct jmp_buf* to);

int getpid(void) {
    kassert(current_task);
    return current_task->pid;
}

char* get_current_name() {
    kassert(current_task);
    return (char*)current_task->name;
}

struct task* alloc_proc() {
    u32 i;
    acquire_lock(&proc_table.lock);
#if 0
    for(i=0; i<PROC_NUM; i++) {
        if(proc_table.procs[i].stat == UNUSED) {
            release_lock(&proc_table.lock);
            return &proc_table.procs[i];
        }
    }
#endif
    for(i=0; i<PROC_NUM; i++) {
        if(proc_table.procs[i] == 0) {
            proc_table.procs[i] = (struct task*)alloc_mem();
            release_lock(&proc_table.lock);
            return proc_table.procs[i];
        }
    }
    release_lock(&proc_table.lock);
    return 0;
}

static u32 next_pid() {
    u32 i, id = 0;
repeat:
    id++;
    for(i=0; i<PROC_NUM; i++) {
        if(proc_table.procs[i] == 0) continue;
        if(proc_table.procs[i]->pid == id)
            goto repeat;
    }
    return id;
}

static void init_proc0() {
    struct task* t = current_task = alloc_proc();
    t->pid = next_pid();
    t->ppid = 0;
    t->p_vm.vm_pgd = &pg_dir0;
    t->stat = RUNNABLE;
    t->next = 0;
    t->r_time = 0;
    t->cwd = inode_name("/");
    tss.ss0  = KERN_DS;
    tss.esp0 = (u32)t + PAGE_SIZE;
    strcpy(t->name, "init");
}

static void init_proctable() {
    init_lock(&proc_table.lock, "proc_table");
    memset(proc_table.procs, 0, sizeof(proc_table.procs));
}

void init_multi_task() {
    init_proctable();
    init_proc0();
}

void forkret() {
    static int first = 1;
    if(first) {
        first = 0;
    }
}

struct task* spawn(void* func) {
    struct task *parent, *new_task;
    parent = current_task;
    new_task = alloc_proc();
    new_task->stat = NEW;
    new_task->pid = next_pid();
    new_task->ppid = parent->pid;
    vm_clone(&new_task->p_vm);
    new_task->p_context = parent->p_context;
    new_task->p_context.eip = (u32)func;
    new_task->p_context.esp = (u32)new_task+PAGE_SIZE;
    new_task->r_time = 0;
    if(new_task->pid == 2)
        strcpy(new_task->name, "proc2");
    else if(new_task->pid == 3)
        strcpy(new_task->name, "proc3");
    return new_task;
}

//#define DEBUG_PROC 1

#ifdef DEBUG_PROC
static int step = 0;
#endif
void swtch_to(struct task *to){
    struct task *from = current_task;
    tss.esp0 = (u32)to + PAGE_SIZE; 
    from->stat = RUNNABLE;
    to->stat   = RUNNING;
    to->r_time++;
    current_task = to;
    flush_pgd(to->p_vm.vm_pgd);
#ifdef DEBUG_PROC
    printk("switch %d from: %s to %s\n", step++, from->name, to->name);
#endif
    _do_swtch(&(from->p_context), &(to->p_context));
}

void sched() {
    int i, min;
    struct task* next = 0;
    struct task* t;
    
    if(current_task == 0)
        return;
    min = -1;
    for(i=PROC_NUM-1; i>=0; i--) {
        t = proc_table.procs[i];
        if(t == 0) continue;
        if(t->stat != RUNNABLE || t == current_task ) 
            continue;
        else {
            if(min == -1 || t->r_time <= min) {
                min = t->r_time;
                next = t;
            }
        }
    }

    if(next) {
        swtch_to(next);
    }
}

void sleep(void* change, struct spinlock* lock) {
    if(current_task == 0)
        PANIC("sleep: no task");
    if(lock == 0)
        PANIC("sleep: no lock");

    cli();
    current_task->chan = change;
    current_task->stat = WAIT;
    sti();
    sched();
}

void wakeup(void* change) {
    u32 i;
    struct task* p;

    if(change == 0)
        PANIC("wakeup: change error");
    for(i=0; i<PROC_NUM; i++) {
        p = proc_table.procs[i];
        if(p == 0) continue;
        if(p->stat == WAIT && p->chan == change) {
            p->stat = RUNNABLE;
        }
    }
}
