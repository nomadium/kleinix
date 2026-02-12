SECTIONS
{
  . = 0x80200000; /* XXX: this is not portable, fix */
  .text : { *(.text) }
  .data : { *(.data) }
  .bss  : { *(.bss) }
}