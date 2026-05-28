OUTPUT_FORMAT("elf32-i386")
ENTRY(start)
SECTIONS {
    .text 0x7c00 : {
        *(.text)
        *(.rodata .rodata.*)
    }
    .data : {
        *(.data)
    }
}
