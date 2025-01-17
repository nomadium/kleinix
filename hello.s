.equ UART, 0x10000000 # UART output register (QEMU virt risc-v board)


.global _start

_start:
    # Main U-Boot use case is to boot Linux kernels.
    # Linux kernel in RISC-V expects to have the hartid of the current core in a0.
    # So by using this bootloader and this de-facto standard, we can rely on that as well.
    # See: https://www.kernel.org/doc/html/next/riscv/boot.html

    # run only one instance
    mv      tp, a0          # save hart id in tp register
    bnez    tp, forever     # other harts will just spin forever

    la      a0, hello
    call    uart_print
    la      a0, bye
    call    uart_print

forever:
    wfi
    j       forever

# void uart_print(char *s)
uart_print:
    li      t0, UART        # load UART output register address
    mv      t1, a0          # copy string parameter

_loop:
    lb      t2, 0(t1)       # load next byte in string at t1 into t2
    beqz    t2, _print_end  # bail out if byte is equal to '\0'
    sb      t2, 0(t0)       # write byte to UART register
    addi    t1, t1, 1       # increase t1
    j       _loop           # repeat

_print_end:
    ret


.section .data

hello:
  .string "hello world!\n"

bye:
  .string "bye...!\n"
