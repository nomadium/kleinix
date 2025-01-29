K = kernel

# entry obj must go first
OBJS = \
  $K/entry.o       \
  $K/cpu.o         \
  $K/klibc.o       \
  $K/sbi.o         \
  $K/sbi_console.o \
  $K/start.o

TOOLPREFIX = riscv64-unknown-elf-
CC         = $(TOOLPREFIX)gcc
AS         = $(TOOLPREFIX)gas
LD         = $(TOOLPREFIX)ld
OBJCOPY    = $(TOOLPREFIX)objcopy
OBJDUMP    = $(TOOLPREFIX)objdump

CFLAGS  = -Wall -Werror -O -fno-omit-frame-pointer -ggdb -gdwarf-2
CFLAGS += -MD
CFLAGS += -mcmodel=medany
CFLAGS += -nostartfiles -fno-common -nostdlib
CFLAGS += -fno-builtin-printf

LDFLAGS = -z max-page-size=4096

all: clean $K/kleinix.img

$K/kleinix.elf: $(OBJS) $K/kernel.ld
	$(LD) $(LDFLAGS) -T $K/kernel.ld -o $@ $(OBJS)
	$(OBJDUMP) -S $@ > $K/kleinix.asm
	$(OBJDUMP) -t $@ | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $K/kleinix.sym

$K/kleinix.img: $K/kleinix.elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf */*.o */*.sym */*.d */*.asm $K/kleinix.elf $K/kleinix.img build

-include kernel/*.d


OPENSBI = /usr/lib/riscv64-linux-gnu/opensbi/generic/fw_jump.bin
UBOOT   = /usr/lib/u-boot/qemu-riscv64_smode/u-boot.bin

ifndef CPUS
CPUS := 4
endif

QEMU              = qemu-system-riscv64
QEMU_HW_FLAGS     = -M virt -m 256 -smp $(CPUS) -nographic -display none
QEMU_BOOT_FLAGS   = -bios $(OPENSBI) -kernel $(UBOOT)
QEMU_NET_HW_FLAGS = -device virtio-net-device,netdev=net -netdev user,id=net,tftp=build
QEMU_FLAGS        = $(QEMU_HW_FLAGS) $(QEMU_BOOT_FLAGS) $(QEMU_NET_HW_FLAGS)
run: $K/kleinix.img
	mkdir -p build
	mkimage -f boot/kleinix.its build/kleinix.itb
	mkimage -A riscv -T script -C none -n 'Boot script' -d boot/uboot.cmd build/boot.scr.uimg
	$(QEMU) $(QEMU_FLAGS)
