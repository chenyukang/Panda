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


struct _tss_ {
    u32      link, esp0;
    u16      ss0, __1;
    u32      esp1;
    u16      ss1, __2;
    u32      esp2;
    u16      ss2, __3;
    u32      cr3, eip, eflags;
    u32      eax, ecx, edx, ebx, esp, ebp, esi, edi;
    u16      es, __4;
    u16      cs, __5;
    u16      ss, __6;
    u16      ds, __7;
    u16      fs, __8;
    u16      gs, __9;
    u16      ldt,__10;
    u16      trap, iomb;
} __attribute__((packed));

enum _status {
    WAIT,
    RUNNING,
    ZOMBIE,
    EXITING
};
    
struct task {
    u32 pid;        /* process id */
    u32 ppid;       /* parent id */
    s32 priority;   /* process priority */
    s32 exit_code;  /* exit code process exit */
    u32 esp, ebp;   /* stack and base pointers */
    u32 eip;        /* instruction pointer */
    u32 stack_base; 
    u32 pg_dir;
    enum _status stat;
    struct _tss_ tss;
    struct task* next;
    char name[24];  /* process name*/
};

typedef struct task task_t;

void init_task();
void switch_task();
void move_stack(task_t* task, void* new_pos);
int fork();
int getpid();
char* get_current_name();

#endif
