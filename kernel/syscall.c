#include <syscall.h>
#include <task.h>
#include <string.h>
#include <screen.h>
#include <exec.h>
#include <sysfile.h>


typedef int (*sysc_func) (struct registers_t* r);
extern void stub_ret(void);
static sysc_func sys_routines[NSYSC];

int errno = 0;

int nosys(struct registers_t* regs) {
    return -1;
}

int sys_exec(struct registers_t* regs) {
    char *path = (char*)regs->ebx;
    char **argv = (char**)regs->ecx;
    int r = do_exec(path, argv);
    if(r == -1) {
        //cli();
        //do_exit(1);
        return -1; //remove warnning message
    } else {
        return 0;
    }
}

int sys_fork(struct registers_t* regs) {
    struct task* t;
    struct registers_t* r = 0;
    t = spawn(&stub_ret);
    r = (struct registers_t*)((u32)t + PAGE_SIZE) - 1;
    *r = *regs;
    r->eax = 0;  //child return this 0
    t->p_context.esp = (u32)r;
    t->p_trap = r;
    t->stat = RUNNABLE;
    return t->pid;
}

int sys_exit(struct registers_t* regs) {
    s32 code = (s32)regs->ebx;
    return do_exit(code);
}

int sys_wait(struct registers_t* regs) {
    s32 pid = regs->ebx;
    s32* stat = (s32*)regs->ecx;
    printk("(%d)begin wait\n", current_task->pid);
    regs->eax = wait_p(pid, stat);
    return 0;
}

int sys_write(struct registers_t* regs) {
    char v = regs->ebx;
    putch(v);
    return 1;
}

int sys_uname(struct registers_t* regs) {
    struct utsname* p = (struct utsname*)regs->ebx;
    if(vm_verify((u32)p, sizeof(struct utsname)) < 0) {
        return -1;
    }
    strcpy(p->sysname, "Panda OS");
    strcpy(p->release, "debug");
    strcpy(p->version, "0.11");
    return 1;
}

int sys_time(struct registers_t* regs) {
    struct tm* p = (struct tm*)regs->ebx;
    if(vm_verify((u32)p, sizeof(struct tm)) < 0) {
        return -1;
    }
    *p = kern_time;
    return 1;
}
    
int sys_read(struct registers_t* regs) {
    char* buf = (char*)regs->ecx;
    u32 fd    = regs->ebx;
    u32 cnt   = regs->edx;
    u32 ret;
    if(vm_verify((u32)buf, cnt) < 0) {
        return -1;
    }
    ret = do_read(fd, buf, cnt);
    regs->eax = ret;
    return ret;
}

void do_syscall(struct registers_t* regs){
    int ret;
    sysc_func func = 0;

    if (regs->eax > NSYSC) {
        PANIC("bad syscall");
    }
    current_task->p_error = 0;
    func = sys_routines[regs->eax];
    if (func == NULL)
        func = &nosys;
    ret = (*func)(regs);
    regs->eax = ret;
#if 1
    //
    if(ret < 0 ) {
        //regs->eax = -current_task->p_error; //fix me
        regs->eax = ret;
    }
    else
        regs->eax = ret;
#endif
}

void syscall_init() {
    puts("syscall_init ... \n");
    irq_install_handler(0x80, (isq_t)(&do_syscall));
    sys_routines[NR_setup] = &nosys;
    sys_routines[NR_fork]  = &sys_fork;
    sys_routines[NR_exec]  = &sys_exec;
    sys_routines[NR_write] = &sys_write;
    sys_routines[NR_read]  = &sys_read;
    sys_routines[NR_exitc] = &sys_exit;
    sys_routines[NR_wait]  = &sys_wait;
    sys_routines[NR_uname] = &sys_uname;
    sys_routines[NR_time]  = &sys_time;
}
