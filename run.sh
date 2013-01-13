#!/bin/bash
BOOT="./boot"
KERNEL="./kernel"
INCLUDE="./inc"
NASM="nasm -f elf -g"

# on Mac
GCC="/usr/local/gcc-4.5.2-for-linux32/bin/i586-pc-linux-gcc -Wall -g -nostdinc -fno-builtin -fno-stack-protector -finline-functions -finline-functions-called-once -I./inc/ "
ON_GCC="clang "
LD="/usr/local/gcc-4.5.2-for-linux32/bin/i586-pc-linux-ld "
OBJCPY="/usr/local/gcc-4.5.2-for-linux32/bin/i586-pc-linux-objcopy"

OBJDIR="./objs"
TOOL="./tool"
QEMU="/usr/local/Cellar/qemu/1.2.1/bin/qemu-system-i386 "

do_clean() {
    echo "clean up"
    rm -rf a.img bochsout.txt $OBJDIR/ ./tool/mkfs.exe;
}

do_compile() {
    if [ ! -d "$OBJDIR" ]; then
	mkdir $OBJDIR;
    fi
       
    echo "building boot"
    flist=`cd $BOOT/; ls *.s;`
    for f in $flist;
    do 
	cmd="$NASM $BOOT/$f -o $OBJDIR/${f/.s/.O}"
	echo $cmd; `$cmd`
    done

    echo "building kernel"
    flist=`cd $KERNEL/; ls *.s;`
    `cd ../`
    for f in $flist;
    do
	cmd="$NASM $KERNEL/$f -o $OBJDIR/${f/.s/.o}"
	echo $cmd ; `$cmd`
	if [ $? -ne 0 ]
	    then 
	    exit;
	fi
    done

    flist=`cd $KERNEL/; ls *.c;`
    `cd ../`
    for f in $flist;
    do
	cmd="$GCC -c $KERNEL/$f -o $OBJDIR/${f/.c/.o}"
	#echo $cmd; 
	`$cmd`;
	if [ $? -ne 0 ]
	    then 
	    exit;
	fi
    done
    

    do_link;
}

do_link() {
    echo "linking ..."
    cd  $OBJDIR;
    $LD boot.O -o boot.bin -T ../$TOOL/boot.ld;
    $LD setup.O -o setup.bin -T ../$TOOL/setup.ld;
    
    #head.O must puted at first
    objs=`ls *.o`
    cmd="$LD head.O $objs -o kernel.elf -T ../$TOOL/kernel.ld"
    echo $cmd; `$cmd`;

    cmd="$OBJCPY -R .pdr -R .comment -R .note -S -O binary kernel.elf kernel.bin"
    echo $cmd; `$cmd`;

    if [ $? -ne 0 ]
	then
	echo "link error!"
	exit
    else
	echo "making a.img"
	cmd="cat boot.bin setup.bin kernel.bin > ../a.img"
	echo $cmd; cat boot.bin setup.bin kernel.bin > ../a.img;
	cd ../;
    fi
}

do_prepare_hd() {
    `$ON_GCC ./tool/mkfs.c -o ./tool/mkfs.exe`
    if [ ! -f "hd.img" ]; then
    echo "making hard disk"
    #bximage hd.img -hd -mode=flat -size=10 -q;
    `./tool/mkfs.exe hd.img ./kernel.elf setup.bin`
    fi
}

do_all()
{
    do_clean && do_compile ;
    do_prepare_hd;
    if [ -f "a.img" ]
	then 
	`$QEMU -fda a.img -localtime`
	#`bochs -q`
    fi
}

do_commit() {
    cmd="git commit -a -m\"$log\""
    echo $cmd
    do_clean;
    git add .;
    git commit -a -m"$log"
    git push;
}

show_help()
{
    echo "-clean|x   : do clean"
    echo "-compile|c : do compile"
    echo "-all|a     : do above two, and run simulation"
    echo "-commit|u  : do git commit"
    echo "-line|l    : do count code line :)"
}

do_wc_line() {
    find ./ -name *.[chs] -or -name *.asm| xargs cat | wc -l;
}

while [ $# -gt 0 ]
do
  case $1 in
    -all|-a) do_all; exit 0;;
    -clean|-x) do_clean; exit 0;;
    -compile|-c) do_clean; do_compile; exit 0;;
    -commit |-u) shift; log=$1; do_commit; exit 0;;
    -line |-l) do_wc_line; exit 0;
  esac
  shift
done


do_all;
show_help;
