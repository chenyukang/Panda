#include <syscall.h>
#include <task.h>
#include <string.h>

typedef int (*sysc_func) (struct registers_t* r);

int nosys(struct registers_t* r) {
    return -1;
}

static sysc_func sys_routines[NSYSC];

/* common handlers for all syscalls */
void do_syscall(struct registers_t* tf){
    int ret;
    sysc_func func = 0;
    
    if (tf->eax > NSYSC) {
        PANIC("bad syscall");
    }
    current_task->p_error = 0;
    func = sys_routines[tf->eax];
    
    if (func == NULL)
        func = &nosys;
    ret = (*func)(tf);
    //
    tf->eax = ret;
    tf->ebx = current_task->p_error;
}
