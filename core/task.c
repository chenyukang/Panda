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
#include <mm.h>
#include <asm.h>
#include <string.h>
#include <gdt.h>
#include <file.h>

#define PROC_NUM 52

struct {
    struct spinlock lock;
    struct task* procs[PROC_NUM];
} proc_table;

struct tss_desc tss;
extern struct pde  pg_dir0;
struct task*       current = 0;
extern u32         init_esp_start;

extern void _do_swtch(struct jmp_buf* from, struct jmp_buf* to);

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

static u32 nextpid() {
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
    struct task* t ;
    t = current = alloc_proc();
    t->pid = nextpid();
    t->ppid = 0;
    t->p_vm.vm_pgd = &pg_dir0;
    t->stat = RUNNABLE;
    t->next = 0;
    t->r_time = 0;
    t->cwd = inode_name("/home");
    strcpy(t->cwd_path, "/home");
    tss.ss0  = KERN_DS;
    tss.esp0 = (u32)t + PAGE_SIZE;
    memset(t->name, 0, sizeof(t->name));
    strcpy(t->name, "kernel");
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
    parent = current;
    new_task = alloc_proc();
    new_task->stat = NEW;
    new_task->pid = nextpid();
    new_task->ppid = parent->pid;
    new_task->cwd = idup(parent->cwd);
    memset(new_task->cwd_path, 0, sizeof(new_task->cwd_path));
    strcpy(new_task->cwd_path, parent->cwd_path);
    vm_clone(&new_task->p_vm);
    new_task->p_context = parent->p_context;
    new_task->p_context.eip = (u32)func;
    new_task->p_context.esp = (u32)new_task+PAGE_SIZE;
    new_task->r_time = 0;
    sti();
    return new_task;
}

void swtch_to(struct task *to) {
    cli();
    struct task *from = current;
    tss.esp0 = (u32)to + PAGE_SIZE;
    to->r_time++;
    current = to;
    flush_pgd(to->p_vm.vm_pgd);
    to->stat = RUNNING;
    sti();
    _do_swtch(&(from->p_context), &(to->p_context));
}

void yield() {
    acquire_lock(&proc_table.lock);
    current->stat = RUNNABLE;
    release_lock(&proc_table.lock);
    sched();
}

/* find the proc with smallest r_time to switch */
void sched() {
    int i, min;
    struct task* next;
    struct task* t;

    t = next = 0;
    min = -1;
    for(i=0; i<PROC_NUM; i++) {
        t = proc_table.procs[i];
        if(t != NULL &&
           (t->stat != ZOMBIE && t->stat != WAIT)) {
            t->stat = RUNNABLE;
        }
    }
    for(i=0; i<PROC_NUM; i++)  {
        t = proc_table.procs[i];
        if(t == 0 || t == current ) continue;
        if(t->stat == ZOMBIE || t->stat == WAIT) continue;
        if(min == -1 || t->r_time <= min) {
            min = t->r_time;
            next = t;
        }
    }

    if(next) {
        swtch_to(next);
    }
}

void do_sleep(void* change, struct spinlock* lock) {
    cli();
    current->chan = change;
    current->stat = WAIT;
    sti();
    sched();
}

void do_wakeup(void* change) {
    struct task* p;
    u32 i;

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

    //do NOT close 0,1,2
    for(i=3; i<NOFILE; i++) {
        fp = current->ofile[i];
        if(fp) {
            file_close(fp);
        }
        current->ofile[i] = 0;
    }
    vm_clear(&current->p_vm);
    free_mem(current->p_vm.vm_pgd);
    current->chan = 0;
    current->stat = ZOMBIE;
    current->exit_code = ret;
    parent = find_task(current->ppid);
    do_wakeup(parent);
    return 0;
}

s32 growtask(u32 size) {
    cli();
    u32 used = current->p_vm.vm_used_heap;
#if 0
    printk("size: %d\n", size);
    printk("used + size: %d\n", used+ size);
#endif
    kassert(used + size < PAGE_SIZE*20);
    u32 start = (current->p_vm.vm_heap.v_base);
    u32 addr = start + used;
    current->p_vm.vm_used_heap = SZ_ROUND_UP(used + size);
    sti();
    return addr;
}

s32 wait_p(s32 pid, s32* stat) {
    struct task* p;
    u32 i;
    if(vm_verify((u32)stat, sizeof(s32)) < 0) {
        return -1;
    }

try_find:
    for(i=0; i<PROC_NUM; i++) {
        p = proc_table.procs[i];
        if(p == 0 || p == current) continue;
        if(p->pid != pid && pid != -1) continue;
        if(p->stat == ZOMBIE) {
            *stat = p->exit_code;
            free_mem(p);
            proc_table.procs[i] = 0;
            return pid;
        }
    }
    do_sleep(current, NULL);
    goto try_find;
    return 0;
}

void task_debug() {
    char buf[1024];
    task_debug_s(buf, 1024);
    printk("%s\n", buf);
}

int  task_debug_s(char* buf, u32 size) {
    struct task* p;
    char t[1024];
    u32 i, k, max;
    max = 25;
    memset(t, 0, sizeof(t));
    memset(buf, 0, sizeof(buf));
    for(i=0; i<PROC_NUM; i++) {
        p = proc_table.procs[i];
        if(p) {
            sprintk(t, "task[%d]: %s ", p->pid, p->name);
            if(strlen(t) < max) {
                for(k=strlen(t); k < max; k++) {
                    t[k] = ' ';
                }
            }
            t[max] = 0;
            strcat(buf, t);
            switch(p->stat) {
            case NEW:      sprintk(t, "NEW\n");      break;
            case WAIT:     sprintk(t, "WAIT\n");     break;
            case RUNNING:  sprintk(t, "RUN\n");      break;
            case ZOMBIE:   sprintk(t, "ZOMBIE\n");   break;
            case RUNNABLE: sprintk(t, "RUNNABLE\n"); break;
            default:                                 break;
            }
            strcat(buf, t);
        }
    }
    return 0;
}
