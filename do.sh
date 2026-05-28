#!/bin/bash
## NOTE: I tested on Mac/Qemu and Linux/Qemu,
## Bochs have some problem in loading and this haven't been solved.
BOOT="./boot"
KERNEL="./core"
INCLUDE="./inc"
NASM="${NASM:-nasm} -f elf"
CFLAGS="-Wall -m32 -O -nostdinc -fno-builtin -fno-stack-protector -fcommon -mno-sse -mno-sse2 -mno-mmx -Wno-implicit-function-declaration -finline-functions -I./inc/ "
BOCHS="${BOCHS:-bochs}"
DEFAULT="None"

find_tool() {
    for tool in "$@"; do
        if command -v "$tool" >/dev/null 2>&1; then
            command -v "$tool"
            return 0
        fi
    done
    return 1
}

if [ "$(uname)" = "Linux" ]; then
    GCC="${GCC:-gcc}"
    ON_GCC="${ON_GCC:-gcc}"
    LD="${LD:-ld}"
    OBJCPY="${OBJCPY:-objcopy}"
    QEMU="${QEMU:-qemu-system-i386}"
    BOCHS_CONF="./.bochs_linux"
    DEFAULT="qemu"
else
    LLVM_PREFIX="$(brew --prefix llvm 2>/dev/null || true)"
    BINUTILS_PREFIX="$(brew --prefix binutils 2>/dev/null || true)"

    if [ -z "${GCC:-}" ]; then
        GCC="$(find_tool i686-elf-gcc i386-elf-gcc i586-pc-linux-gcc || true)"
        if [ -z "$GCC" ] && [ -n "$LLVM_PREFIX" ] && [ -x "$LLVM_PREFIX/bin/clang" ]; then
            GCC="$LLVM_PREFIX/bin/clang -target i386-unknown-elf"
        fi
    fi
    ON_GCC="${ON_GCC:-cc}"
    LD="${LD:-$(find_tool i686-elf-ld i386-elf-ld ld.lld || true)}"
    if [ -z "$LD" ] && [ -n "$LLVM_PREFIX" ] && [ -x "$LLVM_PREFIX/bin/ld.lld" ]; then
        LD="$LLVM_PREFIX/bin/ld.lld"
    fi
    OBJCPY="${OBJCPY:-$(find_tool i686-elf-objcopy i386-elf-objcopy objcopy gobjcopy || true)}"
    if [ -z "$OBJCPY" ] && [ -n "$BINUTILS_PREFIX" ] && [ -x "$BINUTILS_PREFIX/bin/objcopy" ]; then
        OBJCPY="$BINUTILS_PREFIX/bin/objcopy"
    fi
    QEMU="${QEMU:-$(find_tool qemu-system-i386 qemu-system-x86_64 || true)}"
    BOCHS_CONF="./.bochs_mac"
    DEFAULT="qemu"
fi

if [ -z "$GCC" ] || [ -z "$LD" ] || [ -z "$OBJCPY" ]; then
    echo "missing toolchain; need an i386 ELF compiler/linker/objcopy"
    echo "tried: i686-elf-gcc/i386-elf-gcc, Homebrew llvm clang+ld.lld, and binutils objcopy"
    exit 1
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
	$NASM $BOOT/$f -o $OBJDIR/${f/.s/.O}
    done

    echo "building kernel"
    flist=`cd $KERNEL/; ls *.s;`
    for f in $flist;
    do
	$NASM $KERNEL/$f -o $OBJDIR/${f/.s/.o}
	if [ $? -ne 0 ]
	then exit;
	fi
    done

    flist=`cd $KERNEL/; ls *.c;`
    for f in $flist;
    do
	$GCC $CFLAGS -c $KERNEL/$f -o $OBJDIR/${f/.c/.o}
	if [ $? -ne 0 ]
	then exit;
	fi
    done

    echo "building user"
    mkdir ./objs/usr;
    mkdir ./objs/usr/lib;
    users=`cd usr; ls *.c;`
    for f in $users;
    do
        $GCC -I./inc -m32 -fno-builtin -Wno-implicit-function-declaration -fno-stack-protector -fcommon -mno-sse -mno-sse2 -mno-mmx -nostdinc -c usr/$f -o $USEROBJDIR/${f/.c/.o}
        if [ $? -ne 0 ]
            then exit;
        fi
    done

    users=`cd objs/usr/; ls *.o;`
    usrc=`cd ./usr/lib; ls *.c`
    for f in $usrc;
    do
	$GCC -I./inc -DUSR -m32 -fno-builtin -Wno-implicit-function-declaration -fno-stack-protector -fcommon -mno-sse -mno-sse2 -mno-mmx -nostdinc -c ./usr/lib/$f -o $USEROBJDIR/lib/${f/.c/.o}
    done
    $NASM ./usr/lib/entry.s -o $USEROBJDIR/lib/entry.O
    for f in $users;
    do
        echo "build user obj: $f"
        $LD $USEROBJDIR/lib/entry.O $USEROBJDIR/lib/*.o $USEROBJDIR/$f -m elf_i386 -e _start -o $USEROBJDIR/${f/.o/}.elf -T $TOOL/user.ld || exit
        $OBJCPY -O binary $USEROBJDIR/${f/.o/}.elf $USEROBJDIR/${f/.o/}
        if [ $? -ne 0 ]
           then exit;
        fi
    done
    do_link;
}

do_link() {
    echo "linking ..."
    cd  $OBJDIR;
    $LD -m elf_i386 boot.O -o boot.elf -T ../$TOOL/boot.ld;
    $OBJCPY -O binary boot.elf boot.bin;
    $LD -m elf_i386 setup.O -o setup.elf -T ../$TOOL/setup.ld;
    $OBJCPY -O binary setup.elf setup.bin;

    #head.O must puted at first
    objs=`ls *.o`
    $LD -m elf_i386 -T ../$TOOL/kernel.ld -o kernel.elf head.O $objs;

    #$OBJDUMP -t kernel.bin | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > kernel.sym
    $OBJCPY -R .pdr -R .comment -R .note -S -O binary kernel.elf kernel.bin;

    if [ $? -ne 0 ]
    then
	    echo "link error!"
	    exit
    else
	    echo "making a.img"
	    cat boot.bin setup.bin kernel.bin > ../a.img
	    min_img_size=$((1440 * 1024))
	    img_size=$(wc -c < ../a.img)
	    if [ "$img_size" -lt "$min_img_size" ]; then
	        dd if=/dev/zero bs=1 count=0 seek=$min_img_size of=../a.img 2>/dev/null
	    fi
	    ls -lh kernel.bin;
	    cd ..;
    fi
}

do_prepare_hd() {
    $ON_GCC ./tool/mkfs.c -o ./tool/mkfs.exe || exit
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
	    if [ -z "$QEMU" ]; then
	        echo "qemu-system-i386/qemu-system-x86_64 not found"
	        exit 1
	    fi
	    $QEMU -boot a -drive file=a.img,format=raw,if=floppy -drive file=hd.img,format=raw,index=1,media=disk -rtc base=localtime -m 128;
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
