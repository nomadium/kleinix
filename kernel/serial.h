#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "types.h"

/* Initialize serial port COM1 */
void serial_init(void);

/* Write a character to serial */
void serial_putc(char c);

/* Write a string to serial */
void serial_puts(const char *s);

/* Printf-like formatted output */
void serial_printf(const char *fmt, ...);

#endif /* __SERIAL_H__ */
