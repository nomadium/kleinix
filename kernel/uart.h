#ifndef __UART_H__
#define __UART_H__

void
uart_init(void);

void
uart_putc(char c);

void
uart_putc_sync(char c);

void
uart_puts(const char *str);

char
uart_getc(void);

int
uart_getc_nonblock(void);

#endif /* __UART_H__ */
