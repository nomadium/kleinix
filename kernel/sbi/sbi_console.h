/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#ifndef __SBI_CONSOLE_H__
#define __SBI_CONSOLE_H__

#include <stddef.h>
#include <stdbool.h>

struct sbi_console_device {
	/** Name of the console device */
	char name[32];

	/** Write a character to the console output */
	void (*console_putc)(char ch);

	/** Write a character string to the console output */
	unsigned long (*console_puts)(const char *str, unsigned long len);

	/** Read a character from the console input */
	int (*console_getc)(void);
};

#define __printf(a, b) __attribute__((format(printf, a, b)))

bool sbi_isprintable(char ch);

int sbi_getc(void);

void sbi_putc(char ch);

void sbi_puts(const char *str);

unsigned long sbi_nputs(const char *str, unsigned long len);

void sbi_gets(char *s, int maxwidth, char endchar);

unsigned long sbi_ngets(char *str, unsigned long len);

int __printf(2, 3) sbi_sprintf(char *out, const char *format, ...);

int __printf(3, 4) sbi_snprintf(char *out, u32 out_sz, const char *format, ...);

int __printf(1, 2) sbi_printf(const char *format, ...);

void __printf(1, 2) __attribute__((noreturn)) sbi_panic(const char *format, ...);

const struct sbi_console_device *sbi_console_get_device(void);

void sbi_console_set_device(const struct sbi_console_device *dev);

#endif
