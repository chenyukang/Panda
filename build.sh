#!/bin/sh



#build boot bin
#nasm -o boot.bin boot.S;

BOOT="./boot"
KERNEL="./kernel"
INCLUDE="./include"
NASM="nasm -f elf"
GCC="gcc -c -W -Wall -I./include/ -fno-stack-protector -fno-builtin"
OBJS="./objs"
TOOL="./tool"

rm -rf a.img $OBJS/*.o;

echo "building boot"
$NASM $BOOT/boot.s  -o $OBJS/boot.o;
$NASM $BOOT/setup.s -o $OBJS/setup.o;
$NASM $BOOT/head.s  -o $OBJS/head.o;

echo "building kernel"
$GCC  $KERNEL/main.c   -o $OBJS/main.o;
$GCC  $KERNEL/screen.c -o $OBJS/screen.o;
$GCC  $KERNEL/irq.c    -o $OBJS/irq.o;
$GCC  $KERNEL/timer.c  -o $OBJS/timer.o;
$GCC  $KERNEL/asm.c    -o $OBJS/asm.o;
$GCC  $KERNEL/gdt.c    -o $OBJS/gdt.o;
$GCC  $KERNEL/idt.c    -o $OBJS/idt.o;
$GCC  $KERNEL/kb.c     -o $OBJS/kb.o;
$GCC  $KERNEL/cpu.c    -o $OBJS/cpu.o;
$GCC  $KERNEL/string.c -o $OBJS/string.o;

echo "linking"
cd  $OBJS;
ld boot.o -o boot.bin -T ../$TOOL/boot.ld;
ld setup.o -o setup.bin -T ../$TOOL/setup.ld;

#ld head.o main.o screen.o irq.o timer.o asm.o \
#   gdt.o idt.o -o kernel.bin -T ../$TOOL/kernel.ld;

ld head.o main.o screen.o gdt.o idt.o asm.o irq.o \
   timer.o kb.o cpu.o string.o \
   -o kernel.bin -T ../$TOOL/kernel.ld;

cat boot.bin setup.bin kernel.bin > ../a.img
cd ../;

#bochs;

#rm *.o;


