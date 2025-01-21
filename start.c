#include "sbi/sbi.h"
#include "param.h"

// entry.S needs one stack per CPU.
__attribute__ ((aligned (16))) char stack0[4096 * NCPU];

// function to print to the console during early boot.
extern void (*sbi_console_puts)(const char *);

#define OSNAME  "Kleinix"
#define VERSION "0.0.1"
#define BANNER                                    \
	" _     _        _       _\n"             \
	"| |   | |      (_)     (_)\n"            \
	"| |  _| | _____ _ ____  _ _   _\n"       \
	"| |_/ ) || ___ | |  _ \\| ( \\ / )\n"    \
	"|  _ (| || ____| | | | | |) X (\n"       \
	"|_| \\_)\\_)_____)_|_| |_|_(_/ \\_)\n\n"

// entry.S jumps here in supervisor mode on stack0.
void
start()
{
	sbi_console_init();
	sbi_console_puts(BANNER);
	sbi_console_puts(OSNAME " v" VERSION "\n");
	sbi_console_puts("Hello World!!!\n");
	// main();
	for (;;) ;
}
