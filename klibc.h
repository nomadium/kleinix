#ifndef __KLIBC_H__
#define __KLIBC_H__

#include <stddef.h>

size_t strlen(const char *str);
int puts(const char *s);
int printf(const char *format, ...);


#endif /* __KLIBC_H__ */
