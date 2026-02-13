/*
 * Kleinix x86_64 kernel entry point
 *
 * Entry convention (from EFI loader):
 *   RDI = pointer to boot_info structure
 *
 * We're already in 64-bit long mode with paging enabled by UEFI.
 */

.section .text
.global _start
.extern start

_start:
    /* Save boot_info pointer */
    movq %rdi, %r15

    /* Set up a stack (16KB per CPU, start with CPU 0) */
    leaq stack_top(%rip), %rsp

    /* Clear direction flag */
    cld

    /* Clear BSS */
    leaq _bss_start(%rip), %rdi
    leaq _bss_end(%rip), %rcx
    subq %rdi, %rcx
    shrq $3, %rcx           /* Count in qwords */
    xorq %rax, %rax
    rep stosq

    /* Call C entry point with boot_info pointer */
    movq %r15, %rdi
    call start

    /* Should not return, but just in case... */
halt_loop:
    hlt
    jmp halt_loop

.section .bss
.align 16
stack_bottom:
    .space 16384            /* 16KB stack */
stack_top:
