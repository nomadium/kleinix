#include "klibc.h"
#include "sbi/sbi.h"
#include "sbi/sbi_ecall_interface.h"

void (*sbi_console_puts)(const char *);

/* Inspired by this example:
 * https://github.com/riscv-software-src/opensbi/blob/v1.5/firmware/payloads/test_main.c
 */
struct sbiret sbi_ecall(int ext, int fid, unsigned long arg0,
			unsigned long arg1, unsigned long arg2,
			unsigned long arg3, unsigned long arg4,
			unsigned long arg5)
{
	struct sbiret ret;

	register unsigned long a0 asm ("a0") = (unsigned long)(arg0);
	register unsigned long a1 asm ("a1") = (unsigned long)(arg1);
	register unsigned long a2 asm ("a2") = (unsigned long)(arg2);
	register unsigned long a3 asm ("a3") = (unsigned long)(arg3);
	register unsigned long a4 asm ("a4") = (unsigned long)(arg4);
	register unsigned long a5 asm ("a5") = (unsigned long)(arg5);
	register unsigned long a6 asm ("a6") = (unsigned long)(fid);
	register unsigned long a7 asm ("a7") = (unsigned long)(ext);
	asm volatile ("ecall"
		      : "+r" (a0), "+r" (a1)
		      : "r" (a2), "r" (a3), "r" (a4), "r" (a5), "r" (a6), "r" (a7)
		      : "memory");
	ret.error = a0;
	ret.value = a1;

	return ret;
}

inline struct sbiret
sbi_probe_extension(long extension_id)
{
	return sbi_ecall(SBI_EXT_BASE, SBI_EXT_BASE_PROBE_EXT,
			(unsigned long)extension_id, 0, 0, 0, 0, 0);
}

static inline long
sbi_legacy_console_putchar(int ch)
{
	struct sbiret ret;

	ret = sbi_ecall(SBI_EXT_0_1_CONSOLE_PUTCHAR, 0,
			(unsigned long)ch, 0, 0, 0, 0, 0);

	return ret.value;
}

static void
sbi_legacy_console_puts(const char *str)
{
	char c;
	while ((c = *str++))
		sbi_legacy_console_putchar(c);
}

inline struct sbiret
sbi_debug_console_write(unsigned long num_bytes,
		unsigned long base_addr_lo, unsigned long base_addr_hi)
{
	return sbi_ecall(SBI_EXT_DBCN, SBI_EXT_DBCN_CONSOLE_WRITE,
			num_bytes, base_addr_lo, base_addr_hi, 0, 0, 0);
}

static void
sbi_debug_console_puts(const char *str)
{
	sbi_debug_console_write(strlen(str), (unsigned long)str, 0);
}

void
sbi_console_init(void)
{
	struct sbiret ret = sbi_probe_extension(SBI_EXT_DBCN);
	const char *warn;

	if (ret.value) {
		sbi_console_puts = &sbi_debug_console_puts;
		return;
	}

	sbi_console_puts = &sbi_legacy_console_puts;

	warn = "sbi: warning: deprecated sbi_console_putchar extension in use.\n";
	sbi_console_puts(warn);

	return;
}
