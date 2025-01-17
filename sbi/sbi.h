#ifndef __SBI_H__
#define __SBI_H__

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

void sbi_ecall_console_puts(const char *str);

#endif /* __SBI_H__ */
