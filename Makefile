ARCH    = riscv64-unknown-elf
CC      = $(ARCH)-gcc
CFLAGS  = -nostartfiles -g
LD      = $(ARCH)-ld
OBJCOPY = $(ARCH)-objcopy
OPENSBI = /usr/lib/riscv64-linux-gnu/opensbi/generic/fw_jump.bin
UBOOT   = /usr/lib/u-boot/qemu-riscv64_smode/u-boot.bin

CFLAGS  += -mcmodel=medany

all: clean hello.img

hello.img: hello.elf
	$(OBJCOPY) -O binary hello.elf hello.img

hello.elf: hello.o start.o klibc.o sbi.o link.ld Makefile
	$(LD) -T link.ld --no-warn-rwx-segments -o hello.elf hello.o start.o klibc.o sbi.o

sbi.o: sbi.c klibc.h
	$(CC) $(CFLAGS) -c $< -o $@

klibc.o: klibc.c klibc.h
	$(CC) $(CFLAGS) -c $< -o $@

hello.o: hello.S
	$(CC) $(CFLAGS) -c $< -o $@

start.o: start.c param.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.o hello.elf hello.img build

QEMU_HW_FLAGS     = -M virt -m 256 -smp 2 -nographic -display none
QEMU_BOOT_FLAGS   = -bios $(OPENSBI) -kernel $(UBOOT)
QEMU_NET_HW_FLAGS = -device virtio-net-device,netdev=net -netdev user,id=net,tftp=build
QEMU_FLAGS        = $(QEMU_HW_FLAGS) $(QEMU_BOOT_FLAGS) $(QEMU_NET_HW_FLAGS)
run: hello.img
	mkdir -p build
	mkimage -f boot/hello.its build/hello.itb
	mkimage -A riscv -T script -C none -n 'Boot script' -d boot/uboot.cmd build/boot.scr.uimg
	qemu-system-riscv64 $(QEMU_FLAGS)
