SECTIONS
{
  . = 0x80200000; /* XXX: this is not portable, fix */
  .text : { *(.text) }
  .rodata : { *(.rodata*) }
  .data : { *(.data) *(.sdata*) }
  . = ALIGN(8);
  _bss_start = .;
  .bss  : { *(.bss) *(.sbss*) }
  _bss_end = .;
}
