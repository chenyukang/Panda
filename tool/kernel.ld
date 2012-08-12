ENTRY(kmain)
SECTIONS {
    .text 0x0 : 
    {
        code = .; _code = .; __code = .;
        *(.text)
	. = ALIGN(4096);	
    }
    .data : 
    {
        data = .; _data = .; __data = .;
        *(.data)
	. = ALIGN(4096);	
    }

    .bss :
    {
        sbss = .;
        *(COMMON)
        *(.bss)
	. = ALIGN(4096);
        ebss = .;
    }

    __kimg_end__ = .;
}
