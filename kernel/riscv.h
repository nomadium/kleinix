#ifndef __RISCV_H__
#define __RISCV_H__


// XXX: in xv6 kernel/types.h defines this
// probably we should fo the same
typedef unsigned long uint64;


static inline uint64
r_tp()
{
	uint64 x;
	asm volatile("mv %0, tp" : "=r" (x));
	return x;
}

#endif /* __RISCV_H__ */
