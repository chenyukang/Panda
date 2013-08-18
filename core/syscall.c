#include <syscall.h>
#include <task.h>
#include <string.h>
#include <screen.h>
#include <exec.h>
#include <file.h>
#include <timer.h>
#include <sysfile.h>

typedef int (*sysc_func) (struct registers_t* r);

extern void stub_ret(void);
static sysc_func sys_routines[NSYSC];

int errno = 0;

int nosys(struct registers_t* regs) {
    return -1;
}

int sys_chdir(struct registers_t* regs) {
    char* path = (char*)regs->ebx;
    do_chdir(path);
    return 0;
}

/* maybe need dcache as Linux */
int sys_getcwd(struct registers_t* regs) {
    char* buf = (char*)regs->ebx;
    int  size = (int)regs->ecx;
    if(vm_verify((u32)buf, size) < 0) {
        return -1;
    }
    return do_getcwd(buf);
}

int sys_exec(struct registers_t* regs) {
    char *path  = (char*)regs->ebx;
    char **argv = (char**)regs->ecx;
    if(do_exec(path, argv) == -1) {
        do_exit(OPENERR);
        return -1;
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
    s32 pid   = (s32)regs->ebx;
    s32* stat = (s32*)regs->ecx;
    regs->eax = wait_p(pid, stat);
    return 0;
}

int sys_write(struct registers_t* regs) {
    int fd    = regs->ebx;
    int cnt   = regs->edx;
    char* buf = (char*)regs->ecx;

    if(vm_verify((u32)buf, cnt) < 0) {
        return -1;
    }
    return do_write(fd, buf, cnt);
}

int sys_uname(struct registers_t* regs) {
    struct utsname* p = (struct utsname*)regs->ebx;
    if(vm_verify((u32)p, sizeof(struct utsname)) < 0) {
        return -1;
    }
    strcpy(p->sysname, "Panda OS");
    strcpy(p->release, "debug");
    strcpy(p->version, "0.1");
    return 1;
}

int sys_stat(struct registers_t* regs) {
    char* path = (char*)regs->ebx;
    struct stat* stat = (struct stat*)regs->ecx;

    if(vm_verify((u32)stat, sizeof(struct stat)) < 0) {
        return -1;
    }
    return do_stat(path, stat);
}

int sys_time(struct registers_t* regs) {
    struct tm* p = (struct tm*)regs->ebx;
    if(vm_verify((u32)p, sizeof(struct tm)) < 0) {
        return -1;
    }
    *p = kern_time;
    return 1;
}

int sys_open(struct registers_t* regs) {
    char* path = (char*)regs->ebx;
    u32   mode = regs->ecx;
    u32   flag = regs->edx;
    return do_open(path, mode, flag);
}

int sys_close(struct registers_t* regs) {
    u32 fd = (u32)regs->ebx;
    return do_close(fd);
}

int sys_sleep(struct registers_t* regs) {
    u32 n = (u32)regs->ebx;
    u32 sec = get_seconds();
    while((get_seconds() - sec) < n) {
        do_sleep(&timer, &timer.lock);
    }
    return 0;
}

int sys_read(struct registers_t* regs) {
    char* buf = (char*)regs->ecx;
    u32 fd    = regs->ebx;
    u32 cnt   = regs->edx;
    s32 ret;
    if(vm_verify((u32)buf, cnt) < 0) {
        return -1;
    }
    ret = do_read(fd, buf, cnt);
    regs->eax = ret;
    return ret;
}

int sys_getpid(struct registers_t* regs) {
    int pid = current->pid;
    regs->eax = pid;
    return pid;
}

int sys_getppid(struct registers_t* regs) {
    int ppid = current->ppid;
    regs->eax = ppid;
    return ppid;
}

int sys_sbrk(struct registers_t* regs) {
    u32 size = (u32)regs->ebx;
    return growtask(size);
}

int sys_procs(struct registers_t* regs) {
    char* buf =(char*)regs->ebx;
    u32  size = (u32)regs->ecx;
    if(vm_verify((u32)buf, size) < 0) {
        return -1;
    }
    return task_debug_s(buf, size);
}

int sys_halt(struct registers_t* regs) {
    xhalt();
    regs->eax = 0;
    return 0;
}

void do_syscall(struct registers_t* regs) {
    s32 ret;
    sysc_func func = 0;

    if (regs->eax > NSYSC) {
        PANIC("bad syscall");
    }
    current->p_error = 0;
    func = sys_routines[regs->eax];
    if (func == NULL)
        func = &nosys;
    ret = (*func)(regs);
    regs->eax = ret;
}

void sysc_init() {
    irq_install(0x80, (isq_t)(&do_syscall));
    sys_routines[NR_setup] = &nosys;
    sys_routines[NR_fork]  = &sys_fork;
    sys_routines[NR_getpid] = &sys_getpid;
    sys_routines[NR_getppid] = &sys_getppid;
    sys_routines[NR_exec]  = &sys_exec;
    sys_routines[NR_write] = &sys_write;
    sys_routines[NR_read]  = &sys_read;
    sys_routines[NR_kexit]  = &sys_exit;
    sys_routines[NR_wait]  = &sys_wait;
    sys_routines[NR_uname] = &sys_uname;
    sys_routines[NR_time]  = &sys_time;
    sys_routines[NR_open]  = &sys_open;
    sys_routines[NR_close] = &sys_close;
    sys_routines[NR_stat]  = &sys_stat;
    sys_routines[NR_chdir] = &sys_chdir;
    sys_routines[NR_getcwd] = &sys_getcwd;
    sys_routines[NR_sleep] = &sys_sleep;
    sys_routines[NR_sbrk] = &sys_sbrk;
    sys_routines[NR_procs] = &sys_procs;
    sys_routines[NR_halt] = &sys_halt;
}
