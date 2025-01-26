#ifndef __SBI_H__
#define __SBI_H__

#include "sbi_ecall_interface.h"

// kernel/types.h defines this
// XXX: move this somewhere else
typedef unsigned int uint32_t;
typedef unsigned int uint32;

/* Inspired by this example:
 * https://github.com/riscv-software-src/opensbi/blob/v1.5/firmware/payloads/test_main.c
 */
struct sbiret {
	unsigned long error;
	unsigned long value;
};

struct sbiret sbi_ecall(int ext,  int fid,  unsigned long arg0,
			unsigned long arg1, unsigned long arg2,
			unsigned long arg3, unsigned long arg4,
			unsigned long arg5);

struct sbiret
sbi_probe_extension(long extension_id);

void
sbi_console_init(void);

struct sbiret
sbi_debug_console_write(unsigned long num_bytes,
		unsigned long base_addr_lo, unsigned long base_addr_hi);

struct sbiret
sbi_hart_start(unsigned long hartid,
		unsigned long start_addr, unsigned long opaque);

struct sbiret
sbi_system_reset(uint32_t reset_type, uint32_t reset_reason);

void
sbi_system_shutdown(void);

struct sbiret
sbi_get_spec_version(void);

struct sbiret
sbi_get_impl_id(void);

struct sbiret
sbi_get_impl_version(void);

struct sbiret
sbi_get_mvendorid(void);

struct sbiret
sbi_get_marchid(void);

struct sbiret
sbi_get_mimpid(void);

void
sbi_print_base_info(void);

#endif /* __SBI_H__ */
