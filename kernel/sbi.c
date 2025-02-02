#include "klibc.h"
#include "sbi/sbi.h"
#include "sbi/sbi_ecall_interface.h"


/* Inspired by this example:
 * https://github.com/riscv-software-src/opensbi/blob/v1.5/firmware/payloads/test_main.c
 */
struct sbiret sbi_ecall(int ext,  int fid,  unsigned long arg0,
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

inline void
sbi_legacy_console_putchar(char ch)
{
	sbi_ecall(SBI_EXT_0_1_CONSOLE_PUTCHAR, 0,
			(unsigned long)ch, 0, 0, 0, 0, 0);
}

inline struct sbiret
sbi_debug_console_write(unsigned long num_bytes,
		unsigned long base_addr_lo, unsigned long base_addr_hi)
{
	return sbi_ecall(SBI_EXT_DBCN, SBI_EXT_DBCN_CONSOLE_WRITE,
			num_bytes, base_addr_lo, base_addr_hi, 0, 0, 0);
}

inline int
sbi_legacy_console_getchar(void)
{
	struct sbiret ret;

	ret = sbi_ecall(SBI_EXT_0_1_CONSOLE_GETCHAR, 0,
			0, 0, 0, 0, 0, 0);
	return ret.value;
}

inline struct sbiret
sbi_debug_console_read(unsigned long num_bytes,
		unsigned long base_addr_lo, unsigned long base_addr_hi)
{
	return sbi_ecall(SBI_EXT_DBCN, SBI_EXT_DBCN_CONSOLE_READ,
			num_bytes, base_addr_lo, base_addr_hi, 0, 0, 0);
}

int
sbi_debug_console_getchar(void)
{
	struct sbiret ret;
	char buf;
	ret = sbi_debug_console_read(1, (unsigned long)&buf, 0);
	return ret.value > 0 ? buf : -1;
}

unsigned long
sbi_debug_console_puts(const char *str, unsigned long num_bytes)
{
	struct sbiret ret;
	ret = sbi_debug_console_write(num_bytes, (unsigned long)str, 0);
	return ret.value;
}

inline struct sbiret
sbi_hart_start(unsigned long hartid,
		unsigned long start_addr, unsigned long opaque)
{
	return sbi_ecall(SBI_EXT_HSM, SBI_EXT_HSM_HART_START,
			hartid, start_addr, opaque, 0, 0, 0);
}

inline struct sbiret
sbi_system_reset(uint32_t reset_type, uint32_t reset_reason)
{
	return sbi_ecall(SBI_EXT_SRST, SBI_EXT_SRST_RESET,
			reset_type, reset_reason, 0, 0, 0, 0);
}

static inline void
sbi_legacy_shutdown(void)
{
	sbi_ecall(SBI_EXT_0_1_SHUTDOWN, 0, 0, 0, 0, 0, 0, 0);
}

void
sbi_system_shutdown(void)
{
	struct sbiret ret = sbi_probe_extension(SBI_EXT_SRST);

	if (ret.value)
		sbi_system_reset(SBI_SRST_RESET_TYPE_SHUTDOWN,
				SBI_SRST_RESET_REASON_NONE);

	sbi_legacy_shutdown();
}

inline struct sbiret
sbi_get_spec_version(void)
{
	return sbi_ecall(SBI_EXT_BASE, SBI_EXT_BASE_GET_SPEC_VERSION,
			0, 0, 0, 0, 0, 0);
}

inline struct sbiret
sbi_get_impl_id(void)
{
	return sbi_ecall(SBI_EXT_BASE, SBI_EXT_BASE_GET_IMP_ID,
			0, 0, 0, 0, 0, 0);
}

inline struct sbiret
sbi_get_impl_version(void)
{
	return sbi_ecall(SBI_EXT_BASE, SBI_EXT_BASE_GET_IMP_VERSION,
			0, 0, 0, 0, 0, 0);
}

inline struct sbiret
sbi_get_mvendorid(void)
{
	return sbi_ecall(SBI_EXT_BASE, SBI_EXT_BASE_GET_MVENDORID,
			0, 0, 0, 0, 0, 0);
}

inline struct sbiret
sbi_get_marchid(void)
{
	return sbi_ecall(SBI_EXT_BASE, SBI_EXT_BASE_GET_MARCHID,
			0, 0, 0, 0, 0, 0);
}

inline struct sbiret
sbi_get_mimpid(void)
{
	return sbi_ecall(SBI_EXT_BASE, SBI_EXT_BASE_GET_MIMPID,
			0, 0, 0, 0, 0, 0);
}

inline long
sbi_legacy_set_timer(uint64_t stime_value)
{
	struct sbiret ret;

	ret = sbi_ecall(SBI_EXT_0_1_SET_TIMER, 0,
			stime_value, 0, 0, 0, 0, 0);
	return ret.value;
}

inline struct sbiret
sbi_timer_set_timer(uint64_t stime_value)
{
	return sbi_ecall(SBI_EXT_TIME, SBI_EXT_TIME_SET_TIMER,
			stime_value, 0, 0, 0, 0, 0);
}
