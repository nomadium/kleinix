/*
 * Minimal ACPI parser for CPU enumeration and shutdown
 */

#include "acpi.h"
#include "serial.h"

static int num_cpus = 0;
static uint16_t pm1a_cnt_blk = 0;
static uint16_t slp_typa = 0;

/* I/O port access */
static inline void outw(uint16_t port, uint16_t val)
{
    __asm__ volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

/* Compare memory */
static int memcmp(const void *s1, const void *s2, size_t n)
{
    const uint8_t *p1 = s1, *p2 = s2;
    while (n--) {
        if (*p1 != *p2)
            return *p1 - *p2;
        p1++;
        p2++;
    }
    return 0;
}

/* Parse MADT to count CPUs */
static void parse_madt(struct acpi_madt *madt)
{
    uint8_t *ptr = (uint8_t *)(madt + 1);
    uint8_t *end = (uint8_t *)madt + madt->header.length;
    
    serial_printf("  Local APIC at 0x%x\n", madt->local_apic_addr);
    
    while (ptr < end) {
        struct madt_entry_header *entry = (struct madt_entry_header *)ptr;
        
        if (entry->type == MADT_TYPE_LOCAL_APIC) {
            struct madt_local_apic *lapic = (struct madt_local_apic *)entry;
            
            if (lapic->flags & MADT_LAPIC_ENABLED) {
                serial_printf("  CPU %d: APIC ID %d\n", num_cpus, lapic->apic_id);
                num_cpus++;
            }
        }
        
        ptr += entry->length;
        if (entry->length == 0) break;  /* Safety check */
    }
}

/* Parse FADT for shutdown info */
static void parse_fadt(struct acpi_fadt *fadt)
{
    pm1a_cnt_blk = fadt->pm1a_cnt_blk;
    serial_printf("  PM1a control block at 0x%x\n", pm1a_cnt_blk);
    
    /* Get DSDT to find \_S5 sleep type */
    /* For QEMU, S5 sleep type is typically 0 or 5 */
    /* We'll use a common default that works with QEMU/OVMF */
    slp_typa = 0;  /* Will be OR'd with SLP_EN bit */
}

/* Parse XSDT/RSDT */
static void parse_xsdt(uint64_t xsdt_addr)
{
    struct acpi_header *xsdt = (struct acpi_header *)xsdt_addr;
    int entries;
    int entry_size;
    int is_xsdt;
    
    if (memcmp(xsdt->signature, "XSDT", 4) == 0) {
        is_xsdt = 1;
        entry_size = 8;
    } else if (memcmp(xsdt->signature, "RSDT", 4) == 0) {
        is_xsdt = 0;
        entry_size = 4;
    } else {
        serial_printf("ACPI: Invalid RSDT/XSDT signature\n");
        return;
    }
    
    entries = (xsdt->length - sizeof(struct acpi_header)) / entry_size;
    serial_printf("ACPI: Found %s with %d entries\n", 
                  is_xsdt ? "XSDT" : "RSDT", entries);
    
    uint8_t *entry_ptr = (uint8_t *)(xsdt + 1);
    
    for (int i = 0; i < entries; i++) {
        uint64_t table_addr;
        
        if (is_xsdt) {
            table_addr = *(uint64_t *)entry_ptr;
        } else {
            table_addr = *(uint32_t *)entry_ptr;
        }
        
        struct acpi_header *table = (struct acpi_header *)table_addr;
        
        if (memcmp(table->signature, "APIC", 4) == 0) {
            serial_printf("ACPI: Found MADT\n");
            parse_madt((struct acpi_madt *)table);
        } else if (memcmp(table->signature, "FACP", 4) == 0) {
            serial_printf("ACPI: Found FADT\n");
            parse_fadt((struct acpi_fadt *)table);
        }
        
        entry_ptr += entry_size;
    }
}

int acpi_init(uint64_t rsdp_addr)
{
    if (rsdp_addr == 0) {
        serial_printf("ACPI: No RSDP provided\n");
        return -1;
    }
    
    struct acpi_rsdp *rsdp = (struct acpi_rsdp *)rsdp_addr;
    
    /* Verify RSDP signature */
    if (memcmp(rsdp->signature, "RSD PTR ", 8) != 0) {
        serial_printf("ACPI: Invalid RSDP signature\n");
        return -1;
    }
    
    serial_printf("ACPI: RSDP at 0x%lx, revision %d\n", 
                  rsdp_addr, rsdp->revision);
    
    /* Use XSDT for ACPI 2.0+, RSDT for 1.0 */
    if (rsdp->revision >= 2 && rsdp->xsdt_addr != 0) {
        parse_xsdt(rsdp->xsdt_addr);
    } else {
        parse_xsdt(rsdp->rsdt_addr);
    }
    
    return num_cpus;
}

int acpi_get_num_cpus(void)
{
    return num_cpus;
}

void acpi_shutdown(void)
{
    serial_printf("ACPI: Initiating shutdown...\n");
    
    if (pm1a_cnt_blk == 0) {
        serial_printf("ACPI: No PM1a control block, cannot shutdown\n");
        return;
    }
    
    /* Write SLP_TYPa | SLP_EN to PM1a_CNT */
    /* SLP_EN is bit 13, SLP_TYP is bits 10-12 */
    /* For S5 (shutdown), SLP_TYP varies by system, try 0 first */
    uint16_t val = (slp_typa << 10) | (1 << 13);
    outw(pm1a_cnt_blk, val);
    
    /* If that didn't work, try other common S5 values */
    for (int i = 0; i <= 7; i++) {
        val = (i << 10) | (1 << 13);
        outw(pm1a_cnt_blk, val);
    }
    
    /* Last resort: try QEMU-specific shutdown port */
    outw(0x604, 0x2000);
    
    serial_printf("ACPI: Shutdown failed\n");
}
