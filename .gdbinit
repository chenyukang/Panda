file ./objs/kernel.elf
target remote localhost:1234
set disassembly-flavor intel
b kmain
