/*
 * SMP (Symmetric Multi-Processing) support for x86_64
 *
 * Wakes up Application Processors (APs) using INIT-SIPI-SIPI sequence.
 * APs start in real mode and must transition to long mode.
 */

#include "smp.h"
#include "serial.h"

/* CPU information */
struct cpu_info cpus[MAX_CPUS];
int num_cpus = 0;
volatile int cpus_running = 0;

/* Print lock for synchronized output */
spinlock_t print_lock = 0;

/* Delay loop - roughly calibrated for QEMU */
static void delay_us(int us) {
    for (volatile int i = 0; i < us * 100; i++) {
        __asm__ volatile("pause");
    }
}

/*
 * AP trampoline code location.
 * Must be in low memory (below 1MB) and page-aligned.
 * We'll copy our trampoline code to 0x8000.
 */
#define AP_TRAMPOLINE_ADDR  0x8000

/* External symbols for AP trampoline */
extern uint8_t ap_trampoline_start[];
extern uint8_t ap_trampoline_end[];

/* Per-CPU stacks (16KB each) */
#define AP_STACK_SIZE 16384
static uint8_t ap_stacks[MAX_CPUS][AP_STACK_SIZE] __attribute__((aligned(16)));

/* Shared data for AP startup - must be at known location */
struct ap_startup_data {
    uint64_t cr3;           /* Page table root */
    uint64_t stack_top;     /* Stack pointer for this AP */
    uint64_t entry;         /* Entry point (ap_entry) */
    uint64_t gdt_ptr;       /* GDT pointer */
} __attribute__((packed));

static struct ap_startup_data *ap_data = (struct ap_startup_data *)(AP_TRAMPOLINE_ADDR + 0x100);

/* Memory copy */
static void memcpy(void *dst, const void *src, size_t n) {
    uint8_t *d = dst;
    const uint8_t *s = src;
    while (n--) *d++ = *s++;
}

/* AP C entry point - called from ap_trampoline after switching to long mode */
void ap_entry(void) {
    uint32_t apic_id = get_apic_id();
    int cpu_idx = -1;
    
    /* Find our CPU index */
    for (int i = 0; i < num_cpus; i++) {
        if (cpus[i].apic_id == apic_id) {
            cpu_idx = i;
            break;
        }
    }
    
    if (cpu_idx >= 0) {
        cpus[cpu_idx].started = 1;
    }
    
    /* Increment running CPU count atomically */
    __sync_fetch_and_add(&cpus_running, 1);
    
    /* Print hello message (synchronized) */
    spin_lock(&print_lock);
    serial_printf("Hello from CPU #%d (APIC ID %d)\n", cpu_idx, apic_id);
    spin_unlock(&print_lock);
    
    /* Halt this AP - just spin forever */
    while (1) {
        __asm__ volatile("hlt");
    }
}

/* Get current CR3 (page table root) */
static uint64_t get_cr3(void) {
    uint64_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

/*
 * Start an AP by sending INIT-SIPI-SIPI sequence
 */
static void start_ap(int cpu_idx) {
    uint8_t apic_id = cpus[cpu_idx].apic_id;
    uint32_t icr_hi = (uint32_t)apic_id << 24;
    
    /* Set up AP's stack */
    ap_data->stack_top = (uint64_t)&ap_stacks[cpu_idx][AP_STACK_SIZE];
    
    /* Send INIT IPI */
    lapic_write(LAPIC_ICR_HI, icr_hi);
    lapic_write(LAPIC_ICR_LO, ICR_INIT | ICR_LEVEL_ASSERT);
    
    /* Wait for delivery */
    delay_us(10000);  /* 10ms */
    
    /* Send STARTUP IPI (vector = physical address >> 12) */
    uint32_t startup_vector = AP_TRAMPOLINE_ADDR >> 12;
    
    lapic_write(LAPIC_ICR_HI, icr_hi);
    lapic_write(LAPIC_ICR_LO, ICR_STARTUP | startup_vector);
    
    delay_us(200);  /* 200us */
    
    /* Send second STARTUP IPI (some CPUs need this) */
    lapic_write(LAPIC_ICR_HI, icr_hi);
    lapic_write(LAPIC_ICR_LO, ICR_STARTUP | startup_vector);
    
    delay_us(200);
}

/*
 * Initialize SMP - wake up all APs
 */
void smp_init(void) {
    uint32_t bsp_apic_id = get_apic_id();
    size_t trampoline_size;
    
    serial_printf("\nSMP: BSP APIC ID is %d\n", bsp_apic_id);
    
    /* Mark BSP */
    for (int i = 0; i < num_cpus; i++) {
        if (cpus[i].apic_id == bsp_apic_id) {
            cpus[i].is_bsp = 1;
            cpus[i].started = 1;
            break;
        }
    }
    
    /* BSP counts as running */
    cpus_running = 1;
    
    if (num_cpus <= 1) {
        serial_printf("SMP: Only one CPU, nothing to do\n");
        return;
    }
    
    /* Copy AP trampoline code to low memory */
    trampoline_size = (size_t)(ap_trampoline_end - ap_trampoline_start);
    serial_printf("SMP: Copying %d byte trampoline to 0x%x\n", 
                  (int)trampoline_size, AP_TRAMPOLINE_ADDR);
    memcpy((void *)AP_TRAMPOLINE_ADDR, ap_trampoline_start, trampoline_size);
    
    /* Set up shared AP startup data */
    ap_data->cr3 = get_cr3();
    ap_data->entry = (uint64_t)ap_entry;
    
    serial_printf("SMP: CR3 = 0x%lx, entry = 0x%lx\n", ap_data->cr3, ap_data->entry);
    
    /* Start each AP */
    for (int i = 0; i < num_cpus; i++) {
        if (cpus[i].is_bsp) continue;
        
        serial_printf("SMP: Starting CPU %d (APIC ID %d)...\n", i, cpus[i].apic_id);
        start_ap(i);
        
        /* Wait a bit for AP to start */
        delay_us(50000);  /* 50ms */
    }
    
    /* Wait for all APs to report in */
    serial_printf("SMP: Waiting for APs to start...\n");
    int timeout = 100;  /* 1 second timeout */
    while (cpus_running < num_cpus && timeout > 0) {
        delay_us(10000);  /* 10ms */
        timeout--;
    }
    
    serial_printf("SMP: %d of %d CPUs running\n", cpus_running, num_cpus);
}
