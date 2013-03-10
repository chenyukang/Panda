# Initial process execs /init.

#include "../inc/syscall.h"
#include "../inc/traps.h"


# exec(init, argv)
.globl start
start:
  pushl $argv
  pushl $init
  pushl $0  // where caller pc would be
  movl $SYS_exec, %eax  // exec(argv, argc), where argv="/init"
  int $T_SYSCALL

#for(;;) 
exit:
  jmp exit

# char init[] = "/init\0";
init:
  .string "/init\0"

# char *argv[] = { init, 0 };
.p2align 2
argv:
  .long init
  .long 0
