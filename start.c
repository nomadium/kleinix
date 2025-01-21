#include "sbi/sbi.h"
#include "param.h"

// entry.S needs one stack per CPU.
__attribute__ ((aligned (16))) char stack0[4096 * NCPU];

// function to print to the console during early boot.
extern void (*sbi_console_puts)(const char *);


// entry.S jumps here in supervisor mode on stack0.
void
start()
{
	sbi_console_init();
	sbi_console_puts("Hello World!!!\n");
	// main();
	for (;;) ;
}
