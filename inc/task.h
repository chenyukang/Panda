/*******************************************************************************
 *
 *      @name   : TASK_H
 *
 *      @author : Yukang Chen (moorekang@gmail.com)
 *      @date   : 2012-06-02 01:06:25
 *
 *      @brief  : protype for task
 *
 *******************************************************************************/

#if !defined(TASK_H)
#define TASK_H

#include <page.h>
#include <mutex.h>
#include <spinlock.h>
#include <inode.h>

#define NOFILE 124

struct cpu_state {
    s32    eip;
    s32    esp;
    s32    ebx; // - callee registers
    s32    ecx;
    s32    edx;
    s32    esi;
    s32    edi;
    s32    ebp;
    u32   __sigmask;
};

struct tss {
    u32      link, esp0;
    u16      ss0, rsv_1;
    u32      esp1;
    u16      ss1, rsv_2;
    u32      esp2;
    u16      ss2, rsv_3;
    u32      cr3, eip, eflags;
    u32      eax, ecx, edx, ebx, esp, ebp, esi, edi;
    u16      es, rsv_4;
    u16      cs, rsv_5;
    u16      ss, rsv_6;
    u16      ds, rsv_7;
    u16      fs, rsv_8;
    u16      gs, rsv_9;
    u16      ldt,rsv_10;
    u16      trap, iomb;
} __attribute__((packed));

struct proc_stack {
    u32 ds, es, fs, gs;
    u32 edi, esi, ebp, esp;
    u32 ebx, edx, ecx, eax;
    u32 int_num, error_code;
    u32 eip, cs, eflags;
    u32 esp0, ss0;
};

enum task_status {
    CREATED = 0,
    WAIT,
    RUNNING,
    ZOMBIE,
    EXITING
};

struct proc_heap{
    void* heap_base;
    void* heap_top;
};
    
struct task {
    struct proc_stack* kstack;
    struct pde* pg_dir;
    u32 privilege;
    void* kstack_base;
    
    struct proc_heap heap;
    
    u32 pid;        /* process id */
    u32 ppid;       /* parent id */
    s32 priority;   /* process priority */

    s32 exit_code;  /* exit code process exit */
    u32 esp, ebp;   /* stack and base pointers */
    u32 eip;        /* instruction pointer */
    u32 stack_base;
    
    enum task_status stat;
    struct cpu_state cpu_s;
    //struct pde* pg_dir;
    //struct _tss_ tss;
    struct task* next;
    void*  ofile[NOFILE];
    struct inode *cwd;           // Current directory
    void *chan;
    char name[64];  /* process name*/
};

typedef struct task task_t;

struct task_table {
    struct mutex lock;
    struct task* head;
};

void init_multi_task();
void sched();
struct task* spawn(void* func);

char* get_current_name();

int fork();
int getpid();

void sleep(void* change, struct spinlock* lock);
void wakeup(void* change);
#endif
