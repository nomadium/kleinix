#include "klibc.h"

size_t strlen(const char *str)
{
	size_t ret = 0;

	while (*str != '\0') {
		ret++;
		str++;
	}

	return ret;
}
