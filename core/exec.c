#include <mm.h>
#include <inode.h>
#include <string.h>
#include <aout.h>
#include <buf.h>
#include <vm.h>
#include <task.h>
#include <blk.h>
#include <asm.h>

extern void enter_user(u32, u32);

static char** store_argv(char* path, char** args) {
    char **res;
    u32 argc , i;

    argc = 1;
    if(args != NULL)
        for(; args[argc-1] != 0; argc++);

    res = (char**)alloc_mem();
    res[0] = (char*)alloc_mem();
    strncpy(res[0], path, PAGE_SIZE-1);
    for(i=1; i<argc; i++) {
        res[i] = (char*)alloc_mem();
        strncpy(res[i], args[i-1], PAGE_SIZE-1);
        res[i][PAGE_SIZE-1] = '\0';
    }
    res[argc] = NULL;
    return res;
}

static void free_argv(char** args) {
    u32 i;
    for(i=0; args[i] != NULL; i++) {
        free_mem(args[i]);
    }
    free_mem(args);
}

u32 ustack_push(u32* esp, char* buf, u32 len) {
    u32 tmp = *esp;
    if(vm_verify(tmp-=len, len) < 0) {
        PANIC("ustack_push: bad mem");
        return 0;
    }
    memcpy((char*)tmp, buf, len);
    return (*esp = tmp);
}

u32 ustack_push_argv(u32* esp, char** args) {
    s32 arglen, argc, i;
    char* s;
    char** uargv;

    arglen = argc = 0;
    kassert(args);
    for(i=0; (s = args[i]) != NULL; i++) {
        arglen += (strlen(s) + 1);
        argc++;
    }
    arglen += sizeof(char*) * argc;
    if(vm_verify(*esp - arglen, arglen) < 0) {
        return -1;
    }
    uargv = (char**)(*esp - arglen);
    for(i=argc-1; i>=0; i--) {
        s = args[i];
        ustack_push(esp, s, strlen(s) + 1);
        uargv[i] = (char*) (*esp);
    }
    *esp = (u32)uargv;
    ustack_push(esp, (char*)&uargv, sizeof(u32));
    return argc;
}

int do_exec(char* path, char** argv) {
    struct inode* ip;
    struct header head;
    struct vm* vm;
    u32 esp, argc;
    char** store;

    ip = inode_name(path);
    if(ip == 0) {
        return -1;
    }

    ilock(ip);

    if(readi(ip, (char*)&head, 0, sizeof(head)) < sizeof(head)){
        goto error;
    }
    if(head.a_magic != NMAGIC ) {
        goto error;
    }
    store = store_argv(path, argv);
    vm = &current->p_vm;
    memset(current->name, 0, sizeof(current->name));
    strcpy(current->name, path);
    vm_renew(vm, &head, ip);

    esp = VM_STACK;
    argc = ustack_push_argv(&esp, store);
    if(argc < 0) {
        PANIC("push argv error");
    }
    ustack_push(&esp, (char*)&argc, sizeof(u32));
    free_argv(store);

    idrop(ip);
    enter_user(vm->vm_entry, esp);
    //never return for succ
    return 0;

error:
    idrop(ip);
    return -1;
}
