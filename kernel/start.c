/*
 * Kleinix x86_64 kernel main entry point
 */

#include "types.h"
#include "serial.h"
#include "acpi.h"

/* Boot info structure from EFI loader */
struct boot_info {
    uint64_t magic;
    uint64_t mem_map_addr;
    uint64_t mem_map_size;
    uint64_t mem_map_desc_size;
    uint64_t framebuffer_addr;
    uint64_t framebuffer_width;
    uint64_t framebuffer_height;
    uint64_t framebuffer_pitch;
    uint64_t acpi_rsdp;
    uint64_t num_cpus;
};

#define BOOT_INFO_MAGIC 0x424F4F54494E464FULL

#define OSNAME  "Kleinix"
#define VERSION "0.0.1"
#define BANNER \
    " _     _        _       _\n" \
    "| |   | |      (_)     (_)\n" \
    "| |  _| | _____ _ ____  _ _   _\n" \
    "| |_/ ) || ___ | |  _ \\| ( \\ / )\n" \
    "|  _ (| || ____| | | | | |) X (\n" \
    "|_| \\_)\\_)_____)_|_| |_|_(_/ \\_)\n\n"

/* Simple delay loop */
static void delay(int seconds)
{
    /* Very rough delay - about 1 second per iteration on QEMU */
    for (int s = 0; s < seconds; s++) {
        for (volatile int i = 0; i < 100000000; i++);
    }
}

/* Called from entry.S */
void start(struct boot_info *info)
{
    int num_cpus;
    
    /* Initialize serial console first */
    serial_init();
    
    /* Print banner */
    serial_puts(BANNER);
    serial_printf("%s v%s (x86_64)\n\n", OSNAME, VERSION);
    
    /* Verify boot info */
    if (!info || info->magic != BOOT_INFO_MAGIC) {
        serial_printf("Error: Invalid boot info (magic=0x%lx)\n", 
                      info ? info->magic : 0);
        goto halt;
    }
    
    serial_printf("Boot info at %p\n", info);
    serial_printf("  Memory map: %d bytes at 0x%lx\n", 
                  (int)info->mem_map_size, info->mem_map_addr);
    serial_printf("  ACPI RSDP: 0x%lx\n", info->acpi_rsdp);
    serial_puts("\n");
    
    /* Initialize ACPI and enumerate CPUs */
    num_cpus = acpi_init(info->acpi_rsdp);
    
    if (num_cpus > 0) {
        serial_printf("\nFound %d CPU(s)\n", num_cpus);
    } else {
        serial_printf("\nNo CPUs found via ACPI\n");
    }
    
    serial_printf("\nHello World from Kleinix on x86_64!\n");
    serial_printf("\nSystem will shutdown in 3 seconds...\n");
    
    delay(3);
    
    /* Shutdown via ACPI */
    acpi_shutdown();
    
halt:
    serial_printf("\nSystem halted.\n");
    while (1) {
        __asm__ volatile("hlt");
    }
}
