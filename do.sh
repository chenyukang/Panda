#!/bin/sh 
BOOT="./boot"
KERNEL="./core"
INCLUDE="./inc"
NASM="nasm -f elf -g"
CFLAGS="-Wall -nostdinc -fno-builtin -fno-stack-protector -finline-functions -finline-functions-called-once -I./inc/ "
BOCHS="bochs "
DEFAULT="None"
if [ `uname` = "Linux" ]; then
    GCC="gcc "
    ON_GCC="gcc"
    LD="ld"
    OBJCPY="objcopy"
    QEMU="qemu "
    BOCHS_CONF="./.bochs_linux"
    DEFAULT="bochs"
else #on Mac, I will run Qemu
    GCC="/usr/local/gcc-4.5.2-for-linux32/bin/i586-pc-linux-gcc "
    ON_GCC="clang "
    LD="/usr/local/gcc-4.5.2-for-linux32/bin/i586-pc-linux-ld "
    OBJCPY="/usr/local/gcc-4.5.2-for-linux32/bin/i586-pc-linux-objcopy"
    OBJDUMP="/usr/local/gcc-4.5.2-for-linux32/bin/i586-pc-linux-objdump"
    QEMU="/usr/local/Cellar/qemu/1.2.1/bin/qemu-system-i386"
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
	then exit;
	fi
    done


    flist=`cd $KERNEL/; ls *.c;`
    `cd ../`
    for f in $flist;
    do
	cmd="$GCC $CFLAGS -c $KERNEL/$f -o $OBJDIR/${f/.c/.o}"
	`$cmd`;
	if [ $? -ne 0 ]
	then exit;
	fi
    done

    echo "building user"
    mkdir ./objs/usr;

    users=`cd usr; ls *.c;` 
    `cd ../`;
    for f in $users;
    do 
        cmd="$GCC -I./inc -fno-builtin -fno-stack-protector -nostdinc -c usr/$f -o $USEROBJDIR/${f/.c/.o}"
        `$cmd`;
        if [ $? -ne 0 ] 
            then exit;
        fi
    done

    users=`cd objs/usr/; ls *.o;` 
    `cd ../`;
    $GCC -I./inc -DUSR -fno-builtin -fno-stack-protector -nostdinc -c ./usr/lib/string.c -o $USEROBJDIR/string.o;
    $GCC -I./inc -fno-builtin -fno-stack-protector -nostdinc -c ./usr/lib/clib.c -o $USEROBJDIR/clib.o;
    $NASM -f elf ./usr/lib/entry.s -o $USEROBJDIR/entry.o;
    for f in $users;
    do 
        cmd="$LD $USEROBJDIR/entry.o $USEROBJDIR/clib.o  $USEROBJDIR/string.o $USEROBJDIR/$f -e _start -o $USEROBJDIR/${f/.o/ } -T $TOOL/user.ld"
        #echo $cmd
        `$cmd`;
        if [ $? -ne 0 ] 
            then exit;
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
    #cmd="$LD head.O $objs -b binary initcode -o kernel.bin -T ../$TOOL/kernel.ld"
    $LD -m elf_i386 -T ../$TOOL/kernel.ld -o kernel.elf head.O $objs;

    #$OBJDUMP -t kernel.bin | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > kernel.sym
    $OBJCPY -R .pdr -R .comment -R .note -S -O binary kernel.elf kernel.bin;

    if [ $? -ne 0 ]
    then
	echo "link error!"
	exit
    else
	echo "making a.img"
	cmd="cat boot.bin setup.bin kernel.bin > ../a.img"
	echo $cmd; cat boot.bin setup.bin kernel.bin > ../a.img;
	ls -lh kernel.bin;
	cd ../;
    fi
}

do_prepare_hd() {
    `$ON_GCC ./tool/mkfs.c -o ./tool/mkfs.exe`
    rm -rf hd.img;
    echo "making hard disk"
    cp README.md ./tool/mkfs.exe objs/usr/;
    cd objs/usr/;
    mkdir home;
    rm -rf *.o;
    ./mkfs.exe hd.img *;
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
    echo "-clean|x   : do clean"
    echo "-compile|c : do compile"
    echo "-all|a     : do above two, and run simulation"
    echo "-commit|u  : do git commit"
    echo "-qemu|q    : do all and run qemu"
    echo "-bochs|b   : do all and run bochs"
    echo "-line|l    : do count code line :)"
}

do_wc_line() {
    find ./ -name *.[chs] -or -name *.asm| xargs cat | wc -l;
}

while [ $# -gt 0 ]
do
    case $1 in
        -help|-h)    show_help;      exit 0;;
        -all|-a)     do_all "qemu";  exit 0;;
        -clean|-x)   do_clean;       exit 0;;
        -compile|-c) do_clean;       do_compile; exit 0;;
        -line |-l)   do_wc_line;     exit 0;;
        -qemu|-q)    do_all "qemu";  exit 0;;
        -bochs|-b)   do_all "bochs"; exit 0;;
        -commit |-u) shift; log=$1;  do_commit;  exit 0;
    esac
    shift
done


do_all $DEFAULT;
show_help;