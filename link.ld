SECTIONS
{
  . = 0x84000000; /* XXX: this is not portable, fix */
  .text : { *(.text) }
  .data : { *(.data) }
  .bss  : { *(.bss) }
}