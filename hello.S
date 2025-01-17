.section .text
.global _entry
_entry:
    # Main U-Boot use case is to boot Linux kernels.
    # Linux kernel in RISC-V expects to have the hartid of the current core in a0.
    # So by using this bootloader and this de-facto standard, we can rely on that as well.
    # See: https://www.kernel.org/doc/html/next/riscv/boot.html

    # save hart id in tp
    mv   tp, a0

    # set up a stack for C.
    # stack0 is declared in start.c,
    # with a 4096-byte stack per CPU.
    # sp = stack0 + (hartid * 4096)
    la   sp, stack0
    li   a0, 1024*4
    mv   a1, tp
    addi a1, a1, 1
    mul  a0, a0, a1
    add  sp, sp, a0
    # jump to start() in start.c
    call start

spin:
    j spin
