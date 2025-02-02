#ifndef __RISCV_H__
#define __RISCV_H__

#include "types.h"

// Supervisor Interrupt Enable
#define SSTATUS_SIE (1L << 1) // S-mode interrupts
#define SIE_SEIE (1L << 9)    // external
#define SIE_STIE (1L << 5)    // timer
#define SIE_SSIE (1L << 1)    // software

static inline uint64
r_tp()
{
	uint64 x;
	asm volatile("mv %0, tp" : "=r" (x));
	return x;
}

static inline uint64
rdtime()
{
	uint64 x;
	asm volatile("rdtime %0" : "=r" (x));
	return x;
}

static inline uint64
r_sie()
{
	uint64 x;
	asm volatile("csrr %0, sie" : "=r" (x) );
	return x;
}

static inline void
w_sie(uint64 x)
{
	asm volatile("csrw sie, %0" : : "r" (x));
}

// Supervisor trap-vector base address
// Low two bits are mode.
static inline void
w_stvec(uint64 x)
{
  asm volatile("csrw stvec, %0" : : "r" (x));
}

static inline uint64
r_stvec()
{
  uint64 x;
  asm volatile("csrr %0, stvec" : "=r" (x) );
  return x;
}

static inline void
w_sstatus(uint64 x)
{
  asm volatile("csrw sstatus, %0" : : "r" (x));
}

static inline uint64
r_sstatus()
{
  uint64 x;
  asm volatile("csrr %0, sstatus" : "=r" (x) );
  return x;
}

#endif /* __RISCV_H__ */
