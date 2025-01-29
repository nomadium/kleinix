#ifndef __SBI_H__
#define __SBI_H__

#include "../types.h"
#include "sbi_ecall_interface.h"
#include "sbi_console.h"

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

/* SBI calls */

struct sbiret
sbi_probe_extension(long extension_id);

struct sbiret
sbi_debug_console_write(unsigned long num_bytes,
		unsigned long base_addr_lo, unsigned long base_addr_hi);

unsigned long
sbi_debug_console_puts(const char *str, unsigned long num_bytes);

void
sbi_legacy_console_putchar(char ch);

struct sbiret
sbi_debug_console_read(unsigned long num_bytes,
		unsigned long base_addr_lo, unsigned long base_addr_hi);

int
sbi_debug_console_getchar(void);

int
sbi_legacy_console_getchar(void);

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

/* Other SBI related tasks */

void
sbi_console_init(void);

void
sbi_identify(void);

void
sbi_non_boot_hart_start(unsigned long entry_point);

void __attribute__((noreturn))
sbi_hart_hang(void);

#endif /* __SBI_H__ */
