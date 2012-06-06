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

struct task {
    u32 pid;        /* process id */
    u32 ppid;       /* parent id */
    s32 priority;   /* process priority */
    s32 exit_code;  /* exit code process exit */
    
    char name[24];  /* process name*/
    u32 esp, ebp;   /* stack and base pointers */
    u32 eip;        /* instruction pointer */
    page_dir_t* page_dir; /* page directory */
    struct task* next;
};

typedef struct task task_t;

void init_task();
void switch_task();
void move_stack(void* new_stack_pos, u32 size);
int fork();
int getpid();
char* get_current_name();

#endif

