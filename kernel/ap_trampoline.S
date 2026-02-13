/*
 * AP (Application Processor) Trampoline Code
 *
 * APs start execution in 16-bit real mode at the address specified
 * in the SIPI vector. This code transitions the AP from:
 *   Real Mode (16-bit) -> Protected Mode (32-bit) -> Long Mode (64-bit)
 *
 * This code must be position-independent and will be copied to
 * low memory (0x8000) before use.
 */

.section .text
.code16

.global ap_trampoline_start
.global ap_trampoline_end

ap_trampoline_start:
    cli
    cld
    
    /* Set up segments */
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    
    /* Calculate our base address (SIPI vector << 12) */
    movw %cs, %ax
    movw %ax, %ds
    shlw $4, %ax
    
    /* Load GDT */
    lgdtl (gdt_desc - ap_trampoline_start)
    
    /* Enable protected mode */
    movl %cr0, %eax
    orl $1, %eax
    movl %eax, %cr0
    
    /* Jump to 32-bit code */
    ljmpl $0x08, $(ap_trampoline_32 - ap_trampoline_start + 0x8000)

.code32
ap_trampoline_32:
    /* Set up 32-bit segments */
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    
    /* Enable PAE */
    movl %cr4, %eax
    orl $(1 << 5), %eax    /* PAE bit */
    movl %eax, %cr4
    
    /* Load CR3 from shared data */
    movl (ap_data_cr3 - ap_trampoline_start + 0x8000), %eax
    movl %eax, %cr3
    
    /* Enable long mode in EFER MSR */
    movl $0xC0000080, %ecx  /* EFER MSR */
    rdmsr
    orl $(1 << 8), %eax     /* LME bit */
    wrmsr
    
    /* Enable paging (and thus activate long mode) */
    movl %cr0, %eax
    orl $(1 << 31), %eax    /* PG bit */
    movl %eax, %cr0
    
    /* Jump to 64-bit code */
    ljmpl $0x18, $(ap_trampoline_64 - ap_trampoline_start + 0x8000)

.code64
ap_trampoline_64:
    /* Set up 64-bit segments */
    movw $0x20, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    xorw %ax, %ax
    movw %ax, %fs
    movw %ax, %gs
    
    /* Load stack pointer from shared data */
    movq (ap_data_stack - ap_trampoline_start + 0x8000), %rsp
    
    /* Load entry point and jump to it */
    movq (ap_data_entry - ap_trampoline_start + 0x8000), %rax
    jmpq *%rax

/* Align GDT to 8 bytes */
.align 8
gdt_start:
    /* Null descriptor */
    .quad 0
    
    /* 32-bit code segment (selector 0x08) */
    .word 0xFFFF        /* Limit 0:15 */
    .word 0x0000        /* Base 0:15 */
    .byte 0x00          /* Base 16:23 */
    .byte 0x9A          /* Access: present, ring 0, code, readable */
    .byte 0xCF          /* Flags: 4KB granularity, 32-bit */
    .byte 0x00          /* Base 24:31 */
    
    /* 32-bit data segment (selector 0x10) */
    .word 0xFFFF
    .word 0x0000
    .byte 0x00
    .byte 0x92          /* Access: present, ring 0, data, writable */
    .byte 0xCF
    .byte 0x00
    
    /* 64-bit code segment (selector 0x18) */
    .word 0x0000
    .word 0x0000
    .byte 0x00
    .byte 0x9A          /* Access: present, ring 0, code, readable */
    .byte 0x20          /* Flags: long mode */
    .byte 0x00
    
    /* 64-bit data segment (selector 0x20) */
    .word 0x0000
    .word 0x0000
    .byte 0x00
    .byte 0x92          /* Access: present, ring 0, data, writable */
    .byte 0x00
    .byte 0x00
gdt_end:

.align 4
gdt_desc:
    .word gdt_end - gdt_start - 1   /* Limit */
    .long gdt_start - ap_trampoline_start + 0x8000  /* Base */

/*
 * Shared data area (at fixed offset 0x100 from trampoline start)
 * This matches struct ap_startup_data in smp.c
 */
.align 8
.org 0x100
ap_data_cr3:
    .quad 0             /* CR3 - filled by BSP */
ap_data_stack:
    .quad 0             /* Stack pointer - filled by BSP */
ap_data_entry:
    .quad 0             /* Entry point - filled by BSP */

ap_trampoline_end:
