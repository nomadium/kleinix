#ifndef __CPU_H__
#define __CPU_H__

int
hartid();

int
cpuid();

void
start_non_boot_harts(unsigned long entry_point);

void
delay(int n);

void
cpu_identify(int hart_id);

#endif /* __CPU_H__ */
