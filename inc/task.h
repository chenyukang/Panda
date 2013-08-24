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

#include <mm.h>
#include <spinlock.h>
#include <inode.h>
#include <vm.h>

#define NOFILE 124

struct tss_desc {
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

struct jmp_buf {
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


struct proc_stack {
    u32 ds, es, fs, gs;
    u32 edi, esi, ebp, esp;
    u32 ebx, edx, ecx, eax;
    u32 int_num, error_code;
    u32 eip, cs, eflags;
    u32 esp0, ss0;
};

enum task_status {
    UNUSED = 0,
    NEW,
    WAIT,
    RUNNING,
    RUNNABLE,
    ZOMBIE,
    EXITING
};


struct task {
    struct task* next;

    u32          pid;        /* process id */
    u32          ppid;       /* parent id */
    s32          priority;   /* process priority */
    u32          r_time;

    s32          exit_code;  /* exit code process exit */
    u32          p_error;
    u32          eip;        /* instruction pointer */

    enum task_status    stat;
    struct inode*       cwd;                   /* Current directory */
    void*               ofile[NOFILE];
    void*               chan;
    char                name[64];              /* process name*/
    char                cwd_path[64];
    struct vm           p_vm;
    struct jmp_buf      p_context;            /* switch to here to run process */
    struct registers_t* p_trap;               /* trapframe for syscall */
};

typedef struct task task_t;
extern struct task* current;

void task_init();
void sched();
void yield();

s32  do_exit(s32 ret);
s32  wait_p(s32 pid, s32* stat);
struct task* spawn(void* func);

s32 growtask(u32 size);

void do_sleep(void* change, struct spinlock* lock);
void do_wakeup(void* change);

void task_debug();
int  task_debug_s(char* buf, u32 size);

#endif
