K = kernel

# entry obj must go first
OBJS = \
  $K/entry.o       \
  $K/cpu.o         \
  $K/klibc.o       \
  $K/sbi.o         \
  $K/sbi_console.o \
  $K/sbi_helper.o  \
  $K/spinlock.o    \
  $K/start.o       \
  $K/uart.o

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
	rm -rf */*.o */*.sym */*.d */*.asm $K/kleinix.elf $K/kleinix.img image

-include kernel/*.d
-include Makefile.local


OPENSBI ?= /usr/lib/riscv64-linux-gnu/opensbi/generic/fw_jump.bin
UBOOT   ?= /usr/lib/u-boot/qemu-riscv64_smode/u-boot.bin
CPUS    ?= 4

QEMU              = qemu-system-riscv64
QEMU_HW_FLAGS     = -M virt -m 256 -smp $(CPUS) -nographic -display none
QEMU_BOOT_FLAGS   = -bios $(OPENSBI) -kernel $(UBOOT)
QEMU_DSK_HW_FLAGS = -drive file=fat:rw:image,format=raw,id=hd0 \
		    -device virtio-blk-device,drive=hd0
QEMU_FLAGS        = $(QEMU_HW_FLAGS) $(QEMU_BOOT_FLAGS) $(QEMU_DSK_HW_FLAGS)
run: $K/kleinix.img
	mkdir -p image/EFI/BOOT
	cp kernel/kleinix.img image/kernel.bin
	cp boot/loader.efi image/EFI/BOOT/BOOTRISCV64.EFI
	$(QEMU) $(QEMU_FLAGS)
