
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

struct {
    struct spinlock lock;
    struct task procs[PROC_NUM];
} proc_table;

struct tss tss;
struct task* procs[PROC_NUM];

extern struct pde pg_dir0;
extern struct pde* cu_pg_dir;
extern u32         init_esp_start;

//static struct task* init_task = 0;
struct task* current_task = 0;

extern u32 read_eip();
extern void _do_swtch(struct cpu_state* fr, struct cpu_state* to);


int getpid(void) {
    kassert(current_task);
    return current_task->pid;
}

char* get_current_name() {
    kassert(current_task);
    return (char*)current_task->name;
}

struct task* alloc_proc() {
    acquire_lock(&proc_table.lock);
    u32 i;
    for(i=0; i<PROC_NUM; i++) {
        if(proc_table.procs[i].stat == UNUSED) {
            release_lock(&proc_table.lock);
            return &proc_table.procs[i];
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
        if(proc_table.procs[i].stat == UNUSED) continue;
        if(proc_table.procs[i].pid == id)
            goto repeat;
    }
    return id;
}

static void loopfunc() {
    printk("init\n");
    while(1) {
        //printk("init\n");
    }
}

static void init_proc0() {
    struct task* t = current_task = alloc_proc();
    t->pid = next_pid();
    t->ppid = 0;
    t->pg_dir = &pg_dir0;
    t->stat = RUNNABLE;
    t->next = 0;
    t->r_time = 0;
    t->cwd = inode_name("/");
    t->cpu_s.eip = (u32)(loopfunc);
    t->cpu_s.esp = (u32)t + PAGE;
    strcpy(t->name, "init");
    tss.ss0 = 2<<3;
    tss.esp0 = (u32)t + PAGE;
}

static void init_proctable() {
    init_lock(&proc_table.lock, "proc_table");
    acquire_lock(&proc_table.lock);
    memset(proc_table.procs, 0, sizeof(proc_table.procs));
    release_lock(&proc_table.lock);
}

void init_multi_task() {
    init_proctable();
    init_proc0();
}

struct task* spawn(void* func) {
    cli();
    struct task *parent, *new_task;
    parent = current_task;
    new_task = alloc_proc();
    new_task->pid = next_pid();
    new_task->ppid = parent->pid;
    new_task->pg_dir = current_task->pg_dir;
    new_task->cpu_s = parent->cpu_s;
    new_task->cpu_s.eip = (u32)(func);
    new_task->cpu_s.esp = (u32)new_task + PAGE;
    new_task->stat = RUNNABLE;
    new_task->r_time = 0;
    if(new_task->pid == 2)
        strcpy(new_task->name, "proc2");
    else
        strcpy(new_task->name, "proc3");
    sti();
    return new_task;
}

static int step = 0;
void swtch_to(struct task *to){
    struct task *from;
    tss.esp0 = (u32)to + PAGE; 
    from = current_task;
    from->stat = RUNNABLE;
    to->stat   = RUNNING;
    to->r_time++;
    current_task = to;
    cu_pg_dir = to->pg_dir;
    flush_pgd(to->pg_dir);
    printk("switch %d from: %s to %s\n", step++, from->name, to->name);
    _do_swtch(&(from->cpu_s), &(to->cpu_s));
}

void sched() {
    printk("sched\n");
    int i, min;
    struct task* next = 0;
    struct task* t;
    
    if(current_task == 0)
        return;
    min = -1;
    for(i=PROC_NUM-1; i>=0; i--) {
        t = &proc_table.procs[i];
        if(t->stat != RUNNABLE || t == current_task ||
           t->ppid == current_task->ppid)
            continue;
        else {
            if(min == -1 || t->r_time <= min) {
                min = t->r_time;
                next = t;
            }
        }
    }
    kassert(next);
    swtch_to(next);
}

void sleep(void* change, struct spinlock* lock) {
    if(current_task == 0)
        PANIC("sleep: no task");

    if(lock == 0)
        PANIC("sleep: no lock");

    acquire_lock(&proc_table.lock);
    current_task->chan = change;
    current_task->stat = WAIT;
    sched();
    release_lock(&proc_table.lock);
}

void wakeup(void* change) {
    struct task* p;
    u32 i;
    if(change == 0) PANIC("wakeup: change error");
    
    acquire_lock(&proc_table.lock);
    for(i=0; i<PROC_NUM; i++) {
        p = &proc_table.procs[i];
        if(p->stat == WAIT && p->chan == change)
            p->stat = RUNNABLE;
    }
    release_lock(&proc_table.lock);
}


#if 0
void init_userproc() {
    struct task* p;
    extern char _binary_initcode_start[];
    extern char _binary_initcode_size[];
    init_task = p = alloc_proc();
    p->pg_dir = (struct pde*)alloc_pde();
    copy_pgd(current_task->pd_dir, p->pg_dir);
    memset(p->tf, 0 , sizeof(*p->tf));
    p->cwd = nameid("/");
    p->stat = RUNNABLE;
}
#endif
