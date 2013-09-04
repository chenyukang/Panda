#!/bin/bash
## NOTE: I tested on Mac/Qemu and Linux/Qemu,
## Bochs have some problem in loading and this haven't been solved.
BOOT="./boot"
KERNEL="./core"
INCLUDE="./inc"
NASM="nasm -f elf "
CFLAGS="-Wall -m32 -O -nostdinc -fno-builtin -fno-stack-protector -finline-functions -finline-functions-called-once -I./inc/ "
BOCHS="bochs "
DEFAULT="None"

if [ `uname` = "Linux" ]; then
    GCC="gcc "
    ON_GCC="gcc"
    LD="ld"
    OBJCPY="objcopy"
    QEMU="qemu-system-i386 "
    BOCHS_CONF="./.bochs_linux"
    DEFAULT="qemu"
else
    #on Mac, I will run Qemu
    GCC="/usr/local/gcc-4.5.2-for-linux32/bin/i586-pc-linux-gcc "
    ON_GCC="clang "
    LD="/usr/local/gcc-4.5.2-for-linux32/bin/i586-pc-linux-ld "
    OBJCPY="/usr/local/gcc-4.5.2-for-linux32/bin/i586-pc-linux-objcopy"
    OBJDUMP="/usr/local/gcc-4.5.2-for-linux32/bin/i586-pc-linux-objdump"
    QEMU="/usr/local/Cellar/qemu/1.5.1/bin/qemu-system-i386"
    BOCHS_CONF="./.bochs_mac"
    DEFAULT="qemu"
fi

OBJDIR="./objs"
TOOL="./tool"
USEROBJDIR="./objs/usr"

do_clean() {
    echo "clean up"
    rm -rf a.img hd.img bochsout.txt $OBJDIR/ ./tool/mkfs.exe;
}

do_compile() {
    if [ ! -d "$OBJDIR" ]; then
	mkdir $OBJDIR;
    fi

    echo "building boot"
    flist=`cd $BOOT/; ls *.s;`
    for f in $flist;
    do
	`$NASM $BOOT/$f -o $OBJDIR/${f/.s/.O}`
    done

    echo "building kernel"
    flist=`cd $KERNEL/; ls *.s;`
    `cd ../`
    for f in $flist;
    do
	`$NASM $KERNEL/$f -o $OBJDIR/${f/.s/.o}`
	if [ $? -ne 0 ]
	then exit;
	fi
    done

    flist=`cd $KERNEL/; ls *.c;`
    `cd ../`
    for f in $flist;
    do
	`$GCC $CFLAGS -c $KERNEL/$f -o $OBJDIR/${f/.c/.o}`
	if [ $? -ne 0 ]
	then exit;
	fi
    done

    echo "building user"
    mkdir ./objs/usr;
    mkdir ./objs/usr/lib;
    users=`cd usr; ls *.c;`
    `cd ../`;
    for f in $users;
    do
        `$GCC -I./inc -m32 -fno-builtin -fno-stack-protector -nostdinc -c usr/$f -o $USEROBJDIR/${f/.c/.o}`
        if [ $? -ne 0 ]
            then exit;
        fi
    done

    users=`cd objs/usr/; ls *.o;`
    usrc=`cd ./usr/lib; ls *.c`
    for f in $usrc;
    do
	`$GCC -I./inc -DUSR -m32 -fno-builtin -fno-stack-protector -nostdinc -c ./usr/lib/$f -o $USEROBJDIR/lib/${f/.c/.o}`
    done
    `$NASM -f elf ./usr/lib/entry.s -o $USEROBJDIR/lib/entry.O`
    for f in $users;
    do
        `$LD $USEROBJDIR/lib/entry.O $USEROBJDIR/lib/*.o $USEROBJDIR/$f -m elf_i386 -e _start -o $USEROBJDIR/${f/.o/ } -T $TOOL/user.ld`
        if [ $? -ne 0 ]
           then exit;
        fi
    done
    do_link;
}

do_link() {
    echo "linking ..."
    cd  $OBJDIR;
    `$LD -m elf_i386 boot.O -o boot.bin -T ../$TOOL/boot.ld;`
    `$LD -m elf_i386 setup.O -o setup.bin -T ../$TOOL/setup.ld;`

    #head.O must puted at first
    objs=`ls *.o`
    `$LD -m elf_i386 -T ../$TOOL/kernel.ld -o kernel.elf head.O $objs;`

    #$OBJDUMP -t kernel.bin | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > kernel.sym
    `$OBJCPY -R .pdr -R .comment -R .note -S -O binary kernel.elf kernel.bin;`

    if [ $? -ne 0 ]
    then
	echo "link error!"
	exit
    else
	echo "making a.img"
	`cat boot.bin setup.bin kernel.bin > ../a.img`
	ls -lh kernel.bin;
	cd ../;
    fi
}

do_prepare_hd() {
    `$ON_GCC ./tool/mkfs.c -o ./tool/mkfs.exe`
    rm -rf hd.img;
    echo "making hard disk"
    cp ./tool/mkfs.exe objs/;
    cp README.md objs/usr/;
    cp usr/prog.scm objs/usr/;
    cd objs/usr/;
    #mkdir home;
    rm -rf *.o;
    ../mkfs.exe hd.img *;
    cd ../../;
    cp objs/usr/hd.img ./;
}

do_all() {
    do_clean && do_compile ;
    do_prepare_hd;
    if [ -f "a.img" ]
    then
        if [ $1 == "qemu" ]
        then
	    $QEMU -hdb hd.img -fda a.img -localtime -m 128;
        else
     	    $BOCHS -f $BOCHS_CONF -q;
        fi
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

show_help() {
    echo "-clean  | x   : do clean"
    echo "-compile| c   : do compile"
    echo "-all    | a   : do above two, and run simulation"
    echo "-commit | u   : do git commit"
    echo "-qemu   | q   : do all and run qemu"
    echo "-bochs  | b   : do all and run bochs"
    echo "-line   | l   : do count code line :)"
}

do_wc_line() {
    find ./ -name *.[chs] -or -name *.asm| xargs cat | wc -l;
}

while [ $# -gt 0 ]
do
    case $1 in
        -help|-h)    show_help;      exit 0;;
        -clean|-x)   do_clean;       exit 0;;
        -compile|-c) do_clean;       do_compile; exit 0;;
        -line |-l)   do_wc_line;     exit 0;;
        -qemu|-q)    do_all "qemu";  exit 0;;
        -bochs|-b)   do_all "bochs"; exit 0;;
        -all|-a)     do_all "qemu";  exit 0;;
        -commit |-u) shift; log=$1;  do_commit;  exit 0;
    esac
    shift
done

do_all $DEFAULT;
show_help;
