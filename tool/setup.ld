OUTPUT_FORMAT("binary")
ENTRY(setup)
SECTIONS {
    .text 0x0100 : {
        *(.text)
        *(.rodata)
    }
    .data : {
        *(.data)
    }
}
