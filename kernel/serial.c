/*
 * Serial port (COM1) driver for x86_64
 * Uses legacy I/O port 0x3F8
 */

#include "serial.h"

#define COM1_PORT 0x3F8

/* Port offsets */
#define DATA_REG        0   /* Data register (read/write) */
#define INT_ENABLE_REG  1   /* Interrupt enable */
#define FIFO_CTRL_REG   2   /* FIFO control */
#define LINE_CTRL_REG   3   /* Line control */
#define MODEM_CTRL_REG  4   /* Modem control */
#define LINE_STATUS_REG 5   /* Line status */
#define MODEM_STATUS_REG 6  /* Modem status */

/* Line status bits */
#define LSR_TX_EMPTY    0x20

/* I/O port access */
static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void serial_init(void)
{
    /* Disable interrupts */
    outb(COM1_PORT + INT_ENABLE_REG, 0x00);
    
    /* Enable DLAB (set baud rate divisor) */
    outb(COM1_PORT + LINE_CTRL_REG, 0x80);
    
    /* Set divisor to 1 (115200 baud) */
    outb(COM1_PORT + DATA_REG, 0x01);      /* Low byte */
    outb(COM1_PORT + INT_ENABLE_REG, 0x00); /* High byte */
    
    /* 8 bits, no parity, one stop bit */
    outb(COM1_PORT + LINE_CTRL_REG, 0x03);
    
    /* Enable FIFO, clear them, with 14-byte threshold */
    outb(COM1_PORT + FIFO_CTRL_REG, 0xC7);
    
    /* IRQs enabled, RTS/DSR set */
    outb(COM1_PORT + MODEM_CTRL_REG, 0x0B);
}

static int serial_tx_empty(void)
{
    return inb(COM1_PORT + LINE_STATUS_REG) & LSR_TX_EMPTY;
}

void serial_putc(char c)
{
    /* Wait for transmit buffer to be empty */
    while (!serial_tx_empty());
    
    outb(COM1_PORT + DATA_REG, c);
}

void serial_puts(const char *s)
{
    while (*s) {
        if (*s == '\n')
            serial_putc('\r');
        serial_putc(*s++);
    }
}

/* Simple printf implementation */
static void print_uint(uint64_t n, int base, int width, char pad)
{
    char buf[32];
    int i = 0;
    
    if (n == 0) {
        buf[i++] = '0';
    } else {
        while (n > 0) {
            int digit = n % base;
            buf[i++] = digit < 10 ? '0' + digit : 'a' + digit - 10;
            n /= base;
        }
    }
    
    /* Padding */
    while (i < width) {
        buf[i++] = pad;
    }
    
    /* Print in reverse */
    while (i > 0) {
        serial_putc(buf[--i]);
    }
}

static void print_int(int64_t n, int width, char pad)
{
    if (n < 0) {
        serial_putc('-');
        n = -n;
        if (width > 0) width--;
    }
    print_uint(n, 10, width, pad);
}

void serial_printf(const char *fmt, ...)
{
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    
    while (*fmt) {
        if (*fmt != '%') {
            if (*fmt == '\n')
                serial_putc('\r');
            serial_putc(*fmt++);
            continue;
        }
        
        fmt++;  /* Skip '%' */
        
        /* Parse width */
        int width = 0;
        char pad = ' ';
        if (*fmt == '0') {
            pad = '0';
            fmt++;
        }
        while (*fmt >= '0' && *fmt <= '9') {
            width = width * 10 + (*fmt - '0');
            fmt++;
        }
        
        switch (*fmt) {
        case 'd':
        case 'i':
            print_int(__builtin_va_arg(args, int), width, pad);
            break;
        case 'u':
            print_uint(__builtin_va_arg(args, unsigned int), 10, width, pad);
            break;
        case 'x':
            print_uint(__builtin_va_arg(args, unsigned int), 16, width, pad);
            break;
        case 'l':
            fmt++;
            if (*fmt == 'x') {
                print_uint(__builtin_va_arg(args, uint64_t), 16, width, pad);
            } else if (*fmt == 'u' || *fmt == 'd') {
                print_uint(__builtin_va_arg(args, uint64_t), 10, width, pad);
            }
            break;
        case 'p':
            serial_puts("0x");
            print_uint(__builtin_va_arg(args, uint64_t), 16, 16, '0');
            break;
        case 's': {
            const char *s = __builtin_va_arg(args, const char *);
            serial_puts(s ? s : "(null)");
            break;
        }
        case 'c':
            serial_putc(__builtin_va_arg(args, int));
            break;
        case '%':
            serial_putc('%');
            break;
        default:
            serial_putc('%');
            serial_putc(*fmt);
            break;
        }
        fmt++;
    }
    
    __builtin_va_end(args);
}
