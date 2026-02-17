#include "uart.h"
#include <stdint.h>

#define UART_BASE 0x10000000UL
#define UART_RHR 0x00
#define UART_THR 0x00
#define UART_DLL 0x00
#define UART_IER 0x01
#define UART_DLM 0x01
#define UART_FCR 0x02
#define UART_LCR 0x03
#define UART_LSR 0x05

#define LSR_RX_READY   (1 << 0)
#define LSR_TX_IDLE    (1 << 5)

static inline
void mmio_write8(uint64_t addr, uint8_t value)
{
	*(volatile uint8_t *)addr = value;
}

static inline
uint8_t mmio_read8(uint64_t addr)
{
	return *(volatile uint8_t *)addr;
}

void
uart_putc(char c)
{
	while ((mmio_read8(UART_BASE + UART_LSR) & LSR_TX_IDLE) == 0);
	mmio_write8(UART_BASE + UART_THR, c);
}

void
uart_putc_sync(char c)
{
	if (c == '\n')
		uart_putc('\r');
	uart_putc(c);
}

void
uart_puts(const char *fmt)
{
	while (*fmt)
		uart_putc_sync(*fmt++);
}

char
uart_getc(void)
{
	/* Wait until data is available */
	while ((mmio_read8(UART_BASE + UART_LSR) & LSR_RX_READY) == 0);

	return mmio_read8(UART_BASE + UART_RHR);
}

int
uart_getc_nonblock(void)
{
	if (mmio_read8(UART_BASE + UART_LSR) & LSR_RX_READY)
		return mmio_read8(UART_BASE + UART_RHR);
	return -1;
}

/*
void
uart_gets(char *s, int maxwidth, char endchar)
{
	int ch;
	char *retval = s;

	while ((ch = uart_getc()) != endchar && ch >= 0 && maxwidth > 1) {
		*retval = (char)ch;
		retval++;
		maxwidth--;
		uart_putc((char)ch);
	}
	*retval = '\0';
}
*/

void
uart_init(void)
{
	/* Disable interrupts */
	mmio_write8(UART_BASE + UART_IER, 0x00);

	/* Enable DLAB */
	mmio_write8(UART_BASE + UART_LCR, 0x80);

	/* Set baud rate divisor
	 * 115200 baud with 22.729 MHz clock -> divisor = 1
	 */
	mmio_write8(UART_BASE + UART_DLL, 0x01);
	mmio_write8(UART_BASE + UART_DLM, 0x00);

	/* 8 bits, no parity, one stop bit */
	mmio_write8(UART_BASE + UART_LCR, 0x03);

	/* Enable FIFO, clear RX/TX queues */
	mmio_write8(UART_BASE + UART_FCR, 0x07);
}
