
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
        *(.rodata);
        *(.data);
        . = ALIGN(0x1000);	
    }

    .bss :
    {
        __bss__ = .;
        *(.bss);
        . = ALIGN(0x1000);
    }

    __kimg_end__ = .;
}
