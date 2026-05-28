OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

SECTIONS {
    . = 0x8000020;
    .text : {
        __text__ = .;
        ./objs/usr/lib/entry.O(.text) *(.text)
        *(.rodata .rodata.* .gnu.linkonce.*)
        . = ALIGN(0x1000);
        __text_end__ = .;
    }
    .data : {
        __data__ = .;
        *(.data .data.*)
        . = ALIGN(0x1000);
    }
    .bss : {
        __bss__ = .;
        *(.bss .bss.* COMMON)
        . = ALIGN(0x1000);
    }
    /DISCARD/ : {
        *(.note .note.* .gnu.* .dynsym .comment .pdr .init .plt .fini .interp .stab .stabstr .eh_frame)
    }
}
