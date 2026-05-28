
ENTRY(kmain)
PHDRS {
    text PT_LOAD FLAGS(5);
    data PT_LOAD FLAGS(6);
}

SECTIONS {
    .text 0x0 : 
    {
        code = .; _code = .; __code = .;
        *(.text);
        . = ALIGN(0x1000);	
    } :text
    .data : 
    {
        data = .; _data = .; __data = .;
        *(.rodata .rodata.*);
        *(.data .data.*);
        . = ALIGN(0x1000);	
    } :data

    .bss :
    {
        __bss__ = .;
        *(.bss .bss.* COMMON);
        . = ALIGN(0x1000);
    } :data

    /DISCARD/ : {
        *(.note .note.* .comment .eh_frame .llvm_addrsig)
    }

    __kimg_end__ = .;
}
