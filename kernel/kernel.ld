/* x86_64 kernel linker script */

ENTRY(_start)

SECTIONS
{
    /* Load at 1MB - traditional x86 kernel load address */
    . = 0x100000;

    .text : {
        *(.text)
        *(.text.*)
    }

    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }

    .data : {
        *(.data)
        *(.data.*)
    }

    . = ALIGN(8);
    _bss_start = .;

    .bss : {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }

    _bss_end = .;

    /* Discard unwanted sections */
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.note.*)
    }
}
