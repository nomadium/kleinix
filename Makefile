ARCH    = riscv64-unknown-elf
CC      = $(ARCH)-gcc
FLAGS   = -nostartfiles -g
LD      = $(ARCH)-ld
OBJCOPY = $(ARCH)-objcopy
OPENSBI = /usr/lib/riscv64-linux-gnu/opensbi/generic/fw_jump.bin
UBOOT   = /usr/lib/u-boot/qemu-riscv64_smode/u-boot.bin


all: clean hello.img

hello.img: hello.elf
	$(OBJCOPY) -O binary hello.elf hello.img

hello.elf: hello.o link.ld Makefile
	$(LD) -T link.ld --no-warn-rwx-segments -o hello.elf hello.o

hello.o: hello.s
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -rf *.o hello.elf hello.img build

QEMU_HW_FLAGS     = -M virt -m 256 -smp 2 -nographic -display none
QEMU_BOOT_FLAGS   = -bios $(OPENSBI) -kernel $(UBOOT)
QEMU_NET_HW_FLAGS = -device virtio-net-device,netdev=net -netdev user,id=net,tftp=build
QEMU_FLAGS        = $(QEMU_HW_FLAGS) $(QEMU_BOOT_FLAGS) $(QEMU_NET_HW_FLAGS)
run: hello.img
	mkdir -p build
	cp $< build
	mkimage -A riscv -T script -C none -n 'Boot script' -d uboot.cmd build/boot.scr.uimg
	qemu-system-riscv64 $(QEMU_FLAGS)
