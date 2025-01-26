#include <stdarg.h>
#include "klibc.h"

// function to print to the console during early boot.
// XXX: should this be here? probably no...
extern void (*sbi_console_puts)(const char *);

size_t strlen(const char *str)
{
	size_t ret = 0;

	while (*str != '\0') {
		ret++;
		str++;
	}

	return ret;
}

// XXX: rework this fugly puts and printf
// implementations. Maybe just reuse the one from OpenSBI?
static char *
tostr(int n, char *buf)
{
	int i, neg = 0, digit;
	char temp;

	if (!buf) return buf;

	i = 0;

	if (n == 0) {
		buf[i++] = '0';
		buf[i] = '\0';
		return buf;
	}

	if (n < 0) {
	       neg = 1;
	       n = -n;
	}

	while (n > 0) {
		digit = n % 10;
		buf[i++] = '0' + digit;
		n /= 10;
	}

	if (neg) buf[i++] = '-';
	buf[i] = '\0';

	int start = 0, end = i - 1;
	while (start < end) {
		temp = buf[start];
		buf[start] = buf[end];
		buf[end] = temp;
		start++;
		end--;
	}

	return buf;
}

int
puts(const char *s)
{
	sbi_console_puts(s);
	return 0;
}

int
printf(const char *format, ...)
{
	char buf[128], *str;
	const char *ptrfmt;
	int i, num;
	va_list args;

	if (!format) return -1;

	// XXX: poor man's memset, fix
	for (i = 0; i < 128; i++) buf[i] = '\0';
	i = 0;

	ptrfmt = format;
	va_start(args, format);

	while (i < 128 && *ptrfmt) {
		if (*ptrfmt == '%') {
			ptrfmt++;
			if (*ptrfmt == '%') {
				buf[i] = '%';
				i++;
			} else if (*ptrfmt == 's') {
				str = va_arg(args, char *);
				int j;
				for (j = 0; j < strlen(str); j++) {
					buf[i] = str[j];
					i++;
				}
			} else if (*ptrfmt == 'd') {
				num = va_arg(args, int);
				int j;
				char buf2[16];
				for (j = 0; j < 16; j++) buf2[j] = 0;
				tostr(num, buf2);
				for (j = 0; j < strlen(buf2); j++) {
					buf[i] = buf2[j];
					i++;
				}
			} else {
				// not implemented
				return -1;
			}
		} else {
			buf[i] = *ptrfmt;
			i++;
		}
		ptrfmt++;
	}

	va_end(args);

	sbi_console_puts(buf);

	return i;
}
