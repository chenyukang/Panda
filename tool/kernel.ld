OUTPUT_FORMAT("binary")
ENTRY(Main)
SECTIONS {
    .text 0x0 : 
    {
        *(.text)
    }
    .rodata : 
    {
	*(.rodata)
    }
    .data : 
    {
        *(.data)
    }

    .bss :
    {
        sbss = .;
        *(COMMON)
        *(.bss)
        ebss = .;
    }
}
