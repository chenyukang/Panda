#!/bin/sh



#build boot bin
#nasm -o boot.bin boot.S;

BOOT="./boot"
KERNEL="./kernel"
INCLUDE="./include"
NASM="nasm -f elf"
GCC="gcc -c -W -Wall -I./include/ -fno-stack-protector -fno-builtin"
OBJDIR="./objs"
TOOL="./tool"

do_clean() {
    echo "clean up"
    rm -rf a.img bochsout.txt $OBJDIR/*.bin $OBJDIR/*.[oO];
}

do_compile() {
    echo "building boot"
    $NASM $BOOT/boot.s  -o $OBJDIR/boot.O;
    $NASM $BOOT/setup.s -o $OBJDIR/setup.O;
    $NASM $BOOT/head.s  -o $OBJDIR/head.O;

    echo "building kernel"
    flist=`ls $KERNEL/`
    for f in $flist
    do

	cmd="$GCC $KERNEL/$f -o $OBJDIR/${f/.c/.o}"
	#echo $cmd
	`$cmd`;
    done
#     $GCC  $KERNEL/main.c   -o $OBJS/main.o;
#     $GCC  $KERNEL/screen.c -o $OBJS/screen.o;
#     $GCC  $KERNEL/irq.c    -o $OBJS/irq.o;
#     $GCC  $KERNEL/timer.c  -o $OBJS/timer.o;
#     $GCC  $KERNEL/asm.c    -o $OBJS/asm.o;
#     $GCC  $KERNEL/gdt.c    -o $OBJS/gdt.o;
#     $GCC  $KERNEL/idt.c    -o $OBJS/idt.o;
#     $GCC  $KERNEL/kb.c     -o $OBJS/kb.o;
#     $GCC  $KERNEL/cpu.c    -o $OBJS/cpu.o;
#     $GCC  $KERNEL/string.c -o $OBJS/string.o;
#     $GCC  $KERNEL/page.c   -o $OBJS/page.o;
#     $GCC  $KERNEL/bitmap.c -o $OBJS/bitmap.o;

    echo "linking"
    cd  $OBJDIR;
    ld boot.O -o boot.bin -T ../$TOOL/boot.ld;
    ld setup.O -o setup.bin -T ../$TOOL/setup.ld;
    
    objs=`ls *.o`
    cmd="ld head.O $objs -o kernel.bin -T ../$TOOL/kernel.ld"
    echo $cmd
    `$cmd`;

#     ld head.o screen.o main.o gdt.o idt.o asm.o irq.o \
# 	timer.o kb.o cpu.o string.o page.o bitmap.o \
# 	-o kernel.bin -T ../$TOOL/kernel.ld;
    
    if [ $? -ne 0 ]
	then
	echo "compile error!"
    else
	echo "making a.img"
	cat boot.bin setup.bin kernel.bin > ../a.img
	cd ../;
    fi

}

do_all()
{
    do_clean;
    do_compile;

    if [ -f "a.img" ]
	then 
	bochs;
    fi
}

show_help()
{
    echo "-clean|x   : do clean"
    echo "-compile|c : do compile"
    echo "-all|a     : do everything, and run Bochs"
}

while [ $# -gt 0 ]
do
  case $1 in
    -all|-a) do_all; exit 0;;
    -clean|-x) do_clean; exit 0;;
    -compile|-c) do_compile; exit 0;;
  esac
  shift
done

show_help;


