K = kernel

# Kernel objects
OBJS = \
  $K/entry.o   \
  $K/serial.o  \
  $K/acpi.o    \
  $K/smp.o     \
  $K/ap_trampoline.o \
  $K/start.o

CC      = gcc
AS      = as
LD      = ld
OBJCOPY = objcopy
OBJDUMP = objdump

CFLAGS  = -Wall -Werror -O2 -g
CFLAGS += -ffreestanding -fno-builtin -nostdlib
CFLAGS += -fno-pic -fno-pie
CFLAGS += -mno-red-zone -mno-mmx -mno-sse -mno-sse2
CFLAGS += -mcmodel=kernel
CFLAGS += -MD

LDFLAGS = -nostdlib -z max-page-size=4096

all: $K/kleinix.img

$K/entry.o: $K/entry.S
	$(CC) -c -o $@ $<

$K/ap_trampoline.o: $K/ap_trampoline.S
	$(CC) -c -o $@ $<

$K/%.o: $K/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$K/kleinix.elf: $(OBJS) $K/kernel.ld
	$(LD) $(LDFLAGS) -T $K/kernel.ld -o $@ $(OBJS)
	$(OBJDUMP) -S $@ > $K/kleinix.asm
	$(OBJDUMP) -t $@ | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $K/kleinix.sym

$K/kleinix.img: $K/kleinix.elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $K/*.o $K/*.d $K/*.elf $K/*.img $K/*.asm $K/*.sym image

-include $K/*.d

# QEMU with OVMF
OVMF    = /usr/share/ovmf/OVMF.fd
LOADER  = loader.efi

run: $K/kleinix.img
	mkdir -p image/EFI/BOOT
	cp $K/kleinix.img image/kernel.bin
	cp $(LOADER) image/EFI/BOOT/BOOTX64.EFI
	qemu-system-x86_64 \
		-m 256M \
		-smp 4 \
		-bios $(OVMF) \
		-drive format=raw,file=fat:rw:image \
		-nographic \
		-no-reboot

.PHONY: all clean run
