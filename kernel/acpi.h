#ifndef __ACPI_H__
#define __ACPI_H__

#include "types.h"

/* ACPI RSDP structure */
struct acpi_rsdp {
    char signature[8];      /* "RSD PTR " */
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_addr;
    /* ACPI 2.0+ fields */
    uint32_t length;
    uint64_t xsdt_addr;
    uint8_t ext_checksum;
    uint8_t reserved[3];
} __attribute__((packed));

/* ACPI table header */
struct acpi_header {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed));

/* MADT (Multiple APIC Description Table) */
struct acpi_madt {
    struct acpi_header header;
    uint32_t local_apic_addr;
    uint32_t flags;
    /* Variable length entries follow */
} __attribute__((packed));

/* MADT entry header */
struct madt_entry_header {
    uint8_t type;
    uint8_t length;
} __attribute__((packed));

/* MADT entry types */
#define MADT_TYPE_LOCAL_APIC     0
#define MADT_TYPE_IO_APIC        1
#define MADT_TYPE_INT_OVERRIDE   2
#define MADT_TYPE_LOCAL_APIC_X2  9

/* Local APIC entry */
struct madt_local_apic {
    struct madt_entry_header header;
    uint8_t processor_id;
    uint8_t apic_id;
    uint32_t flags;
} __attribute__((packed));

#define MADT_LAPIC_ENABLED      0x01

/* FADT (Fixed ACPI Description Table) */
struct acpi_fadt {
    struct acpi_header header;
    uint32_t firmware_ctrl;
    uint32_t dsdt;
    uint8_t reserved1;
    uint8_t preferred_pm_profile;
    uint16_t sci_int;
    uint32_t smi_cmd;
    uint8_t acpi_enable;
    uint8_t acpi_disable;
    uint8_t s4bios_req;
    uint8_t pstate_cnt;
    uint32_t pm1a_evt_blk;
    uint32_t pm1b_evt_blk;
    uint32_t pm1a_cnt_blk;
    uint32_t pm1b_cnt_blk;
    uint32_t pm2_cnt_blk;
    uint32_t pm_tmr_blk;
    uint32_t gpe0_blk;
    uint32_t gpe1_blk;
    uint8_t pm1_evt_len;
    uint8_t pm1_cnt_len;
    /* ... more fields, but we only need pm1a_cnt_blk for shutdown */
} __attribute__((packed));

/* Parse ACPI and return number of CPUs */
int acpi_init(uint64_t rsdp_addr);

/* Get number of CPUs found */
int acpi_get_num_cpus(void);

/* Shutdown via ACPI */
void acpi_shutdown(void);

#endif /* __ACPI_H__ */
