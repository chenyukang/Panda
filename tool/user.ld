OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

SECTIONS {
    . = 0x8000000;
    __base__ = .;
    .header : {
        LONG(0x6400CC)
        LONG(__data__ - __base__)
        LONG(__bss__ - __data__)
        LONG(__bss_end__ - __bss__)
        LONG(0)
        LONG(_start)
        LONG(0)
        LONG(0)
    }
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
        __bss_end__ = .;
    }
    /DISCARD/ : {
        *(.note .note.* .gnu.* .dynsym .comment .pdr .init .plt .fini .interp .stab .stabstr .eh_frame)
    }
}
