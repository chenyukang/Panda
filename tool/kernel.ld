
ENTRY(kmain)
SECTIONS {
    .text 0x0 : 
    {
        code = .; _code = .; __code = .;
        *(.text);
        . = ALIGN(0x1000);	
    }
    .data : 
    {
        data = .; _data = .; __data = .;
        *(.rodata .rodata.*);
        *(.data .data.*);
        . = ALIGN(0x1000);	
    }

    .bss :
    {
        __bss__ = .;
        *(.bss .bss.* COMMON);
        . = ALIGN(0x1000);
    }

    /DISCARD/ : {
        *(.note .note.* .comment .eh_frame .llvm_addrsig)
    }

    __kimg_end__ = .;
}
