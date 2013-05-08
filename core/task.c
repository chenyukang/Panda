
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

#define PROC_NUM 52

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

u32 getpid(void) {
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

static struct task* find_task(u32 pid ) {
    u32 i;
    for(i=0; i<PROC_NUM; i++) {
        if(proc_table.procs[i] == 0) continue;
        if(proc_table.procs[i]->pid == pid)
            return proc_table.procs[i];
    }
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

void task_init() {
    puts("[task] .... ");
    init_proctable();
    init_proc0();
    done();
}

struct task* spawn(void* func) {
    cli();
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
    sti();
    return new_task;
}

//#define DEBUG_PROC 1
#ifdef DEBUG_PROC
static int step = 0;
#endif
void swtch_to(struct task *to){
    acquire_lock(&proc_table.lock);
    struct task *from = current_task;
    tss.esp0 = (u32)to + PAGE_SIZE; 
    to->r_time++;
    current_task = to;
    release_lock(&proc_table.lock);
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

    kassert(current_task);
    min = -1;
    for(i=PROC_NUM-1; i>=0; i--) {
        t = proc_table.procs[i];
        if(t == 0) continue;
        if(t == current_task) continue;
        if(t->stat == ZOMBIE || t->stat == WAIT) continue;
        if(min == -1 || t->r_time <= min) {
            min = t->r_time;
            next = t;
        }
    }

    if(next) {
        kassert(next->stat != ZOMBIE);
        kassert(next->stat != WAIT);
        //printk("switch:%d\n", next->pid);
        swtch_to(next);
    }
}

void sleep(void* change, struct spinlock* lock) {
    if(current_task == 0)
        PANIC("sleep: no task");
#if 0
    if(lock == 0)
        PANIC("sleep: no lock");
#endif

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
            p->chan = 0;
            p->stat = RUNNABLE;
        }
    }
}

s32 do_exit(int ret) {
    u32 i;
    struct file* fp;
    struct task* parent;
    for(i=0; i<NOFILE; i++) {
        fp = current_task->ofile[i];
        if(fp) {
            //close_fp
        }
        current_task->ofile[i] = 0;
    }
    vm_clear(&current_task->p_vm);
    free_mem(current_task->p_vm.vm_pgd);
    current_task->chan = 0;
    current_task->stat = ZOMBIE;
    current_task->exit_code = ret;
    parent = find_task(current_task->ppid);
    wakeup(parent);
    return 0;
}

s32 wait_p(s32 pid, s32* stat) {
    struct task* p;
    u32 i;
    if(vm_verify((u32)stat, sizeof(s32)) < 0) {
        return -1;
    }
    //printk("wait_p:%d \n", current_task->pid);
try_find:
    for(i=0; i<PROC_NUM; i++) {
        p = proc_table.procs[i];
        if(!p) continue;
        if(p == current_task) continue;
        if(p->pid != pid && pid != -1) continue;
        if(p->stat == ZOMBIE) {
            *stat = p->exit_code;
            free_mem(p);
            proc_table.procs[i] = 0;
            //printk("return :%d %d\n", current_task->pid, p->pid);
            return pid;
        }
    }
    sleep(current_task, NULL);
    goto try_find;
    return 0;
}

void task_debug() {
    u32 i;
    struct task* p;
    for(i=0; i<PROC_NUM; i++) {
        p = proc_table.procs[i];
        if(p) {
            printk("task[%d]: %s ", p->pid, p->name);
            switch(p->stat) {
            case NEW: printk(" NEW\n"); break;
            case WAIT: printk(" WAIT\n"); break;
            case RUNNING: printk(" RUNNING\n"); break;
            case ZOMBIE: printk(" ZOMBIE\n"); break;
            default: break;
            }
            printk("\n");
        }
    }
}

