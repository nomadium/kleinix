#ifndef __SMP_H__
#define __SMP_H__

#include "types.h"

#define MAX_CPUS 16

/* CPU info structure */
struct cpu_info {
    uint8_t apic_id;
    uint8_t present;
    uint8_t started;
    uint8_t is_bsp;
};

extern struct cpu_info cpus[MAX_CPUS];
extern int num_cpus;
extern volatile int cpus_running;

/* Local APIC registers (memory-mapped at 0xFEE00000) */
#define LAPIC_BASE      0xFEE00000UL
#define LAPIC_ID        0x020   /* Local APIC ID */
#define LAPIC_EOI       0x0B0   /* End of Interrupt */
#define LAPIC_SVR       0x0F0   /* Spurious Interrupt Vector */
#define LAPIC_ICR_LO    0x300   /* Interrupt Command Register (low) */
#define LAPIC_ICR_HI    0x310   /* Interrupt Command Register (high) */

/* ICR delivery modes */
#define ICR_INIT        0x00000500
#define ICR_STARTUP     0x00000600
#define ICR_LEVEL_ASSERT 0x00004000
#define ICR_DEST_ALL_EX 0x000C0000  /* All excluding self */

/* Read from Local APIC register */
static inline uint32_t lapic_read(uint32_t reg) {
    return *(volatile uint32_t *)(LAPIC_BASE + reg);
}

/* Write to Local APIC register */
static inline void lapic_write(uint32_t reg, uint32_t val) {
    *(volatile uint32_t *)(LAPIC_BASE + reg) = val;
}

/* Get current CPU's APIC ID */
static inline uint32_t get_apic_id(void) {
    return lapic_read(LAPIC_ID) >> 24;
}

/* Initialize SMP - called by BSP */
void smp_init(void);

/* AP entry point - called by each AP after startup */
void ap_main(void);

/* Simple spinlock for synchronization */
typedef volatile uint32_t spinlock_t;

static inline void spin_lock(spinlock_t *lock) {
    while (__sync_lock_test_and_set(lock, 1)) {
        while (*lock) {
            __asm__ volatile("pause");
        }
    }
}

static inline void spin_unlock(spinlock_t *lock) {
    __sync_lock_release(lock);
}

/* Global print lock */
extern spinlock_t print_lock;

#endif /* __SMP_H__ */
