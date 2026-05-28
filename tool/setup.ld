OUTPUT_FORMAT("elf32-i386")
ENTRY(setup)
SECTIONS {
    .text 0x0100 : {
        *(.text)
        *(.rodata .rodata.*)
    }
    .data : {
        *(.data)
    }
}
