.section .text
.global _entry
_entry:
    # Main U-Boot use case is to boot Linux kernels.
    # Linux kernel in RISC-V expects to have the hartid of the current core in a0.
    # So by using this bootloader and this de-facto standard, we can rely on that as well.
    # See: https://www.kernel.org/doc/html/next/riscv/boot.html

    # save hart id in tp
    mv   tp, a0

    # check/save boot hart id
    la   t0, boot_hart_id        # load boot_hart_id value
    lw   t1, 0(t0)               # from memory
    bge  t1, zero, sstack        # if >= 0, system already booted, go to stack set up

    # First hart: zero out .bss section
    la   t0, _bss_start
    la   t1, _bss_end
bss_clear:
    bge  t0, t1, bss_done
    sd   zero, 0(t0)
    addi t0, t0, 8
    j    bss_clear
bss_done:

    # Now store boot hart id
    la   t0, boot_hart_id
    sw   tp, 0(t0)

sstack:
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

    # non-boot cpu(s) jump to non_boot_start() in start.c
    la   t0, boot_hart_id        # load boot_hard_id value
    lw   t1, 0(t0)               # from memory again
    bne  tp, t1, non_boot_start  # if this is not a boot hart, jump to non_boot_start

    # boot cpu jumps to start() in start.c
    call start

spin:
    j spin
