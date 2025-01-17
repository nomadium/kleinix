.equ UART, 0x10000000 # UART output register (QEMU virt risc-v board)


.global _start

_start:
    # XXX: my current understanding is that OpenSBI and U-Boot use 1 hart only at boot time
    # XXX: however, U-Boot run in S-mode, so no access to M-mode registers (e.g. mhartid)
    # XXX: and the code below is disabled to avoid illegal instruction traps
    # XXX: figure out how to retrieve this instance hart id and how to start the other harts
    # run only one instance
    # csrr    t0, mhartid
    # bnez    t0, forever

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
