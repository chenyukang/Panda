ENTRY(kmain)
SECTIONS {
    .text 0x0 : 
    {
        code = .; _code = .; __code = .;
        *(.text)
	. = ALIGN(0x1000);	
    }
    .data : 
    {
        data = .; _data = .; __data = .;
        *(.data)
	. = ALIGN(0x1000);	
    }

    .bss :
    {
        sbss = .;
        *(COMMON)
        *(.bss)
	. = ALIGN(0x1000);
        ebss = .;
    }

    __kimg_end__ = .;
}
