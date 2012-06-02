#!/bin/bash

BOOT="./boot"
KERNEL="./kernel"
INCLUDE="./include"
NASM="nasm -f elf"
GCC="gcc -c -W -Wall -I./include/ -fno-stack-protector -fno-builtin"
OBJDIR="./objs"
TOOL="./tool"

do_clean() {
    echo "clean up"
    rm -rf a.img hd.img bochsout.txt $OBJDIR/*.bin $OBJDIR/*.[oO];
}

do_compile() {
    echo "building boot"
    $NASM $BOOT/boot.s  -o $OBJDIR/boot.O;
    $NASM $BOOT/setup.s -o $OBJDIR/setup.O;
    $NASM $BOOT/head.s  -o $OBJDIR/head.O;

    echo "building kernel"
    flist=`cd $KERNEL/; ls *.c;`
    `cd ../`
    for f in $flist;
    do
	cmd="$GCC $KERNEL/$f -o $OBJDIR/${f/.c/.o}"
	echo $cmd
	`$cmd`;
    done

    do_link;
}

do_link() {
    echo "linking ..."
    cd  $OBJDIR;
    ld boot.O -o boot.bin -T ../$TOOL/boot.ld;
    ld setup.O -o setup.bin -T ../$TOOL/setup.ld;
    
    #head.O must puted at first
    objs=`ls *.o`
    cmd="ld head.O $objs -o kernel.bin -T ../$TOOL/kernel.ld"
    #echo $cmd
    `$cmd`;

    if [ $? -ne 0 ]
	then
	echo "compile error!"
    else
	echo "making a.img"
	cat boot.bin setup.bin kernel.bin > ../a.img
	cd ../;
    fi
}

do_prepare_hd() {
    echo "making hard disk"
    bximage hd.img -hd -mode=flat -size=10 -q;
}

do_all()
{
    do_clean;
    do_compile;
    do_prepare_hd;
    if [ -f "a.img" ]
	then 
	`qemu -fda a.img`
    fi
}

do_commit() {
    echo $log
    cmd="git commit -a -m\"$log\""
    echo $cmd
    do_clean;
    `git add .`;
    $cmd;
    `git push`;
}

show_help()
{
    echo "-clean|x   : do clean"
    echo "-compile|c : do compile"
    echo "-all|a     : do everything, and run Bochs"
    echo "-commit|u  : do commit"
}

while [ $# -gt 0 ]
do
  case $1 in
    -all|-a) do_all; exit 0;;
    -clean|-x) do_clean; exit 0;;
    -compile|-c) do_clean; do_compile; exit 0;;
    -commit |-u) shift; log=$1; do_commit; exit 0;
  esac
  shift
done

show_help;


