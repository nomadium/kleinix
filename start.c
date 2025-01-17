#include "sbi/sbi.h"
#include "param.h"

// entry.S needs one stack per CPU.
__attribute__ ((aligned (16))) char stack0[4096 * NCPU];

// entry.S jumps here in supervisor mode on stack0.
void
start()
{
	sbi_ecall_console_puts("Hello World!!!\n");
	// main();
	for (;;) ;
}
