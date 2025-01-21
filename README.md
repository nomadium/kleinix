# kleinix
Toy OS for RISC-V (rv64) hardware.

Status: it's very early days (just boots in S-mode, prints a banner and stops).
Mostly tested on QEMU RISC-V virt "hardware".

## Requirements
### Tools:
- riscv64-unknown-elf-gcc
- riscv64-unknown-elf-ld
- riscv64-unknown-elf-objcopy

### Building:
make

### Execution:
qemu-system-riscv64

## Building
make all

## Running
make run

## Acknowledgements

Kleinix is heavily influenced and copies from:

- xv6 (https://github.com/mit-pdos/xv6-riscv)

Main current goals at the moment are: to build on xv6 and expand it,
experiment with OS concepts and port Kleinix to at least one real
hardware board (i.e. D1, JH7110 or any other).
