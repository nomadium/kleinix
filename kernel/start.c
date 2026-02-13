#include "sbi/sbi.h"
#include "param.h"
#include "riscv.h"
#include "klibc.h"
#include "cpu.h"
#include "fdt.h"

// entry.S needs one stack per CPU.
__attribute__ ((aligned (16))) char stack0[4096 * NCPU];

/* Initialized to -1 so it goes in .data, not .bss (which gets zeroed) */
volatile int boot_hart_id = -1;

extern void _entry(void);

#define OSNAME  "Kleinix"
#define VERSION "0.0.1"
#define BANNER                                    \
	" _     _        _       _\n"             \
	"| |   | |      (_)     (_)\n"            \
	"| |  _| | _____ _ ____  _ _   _\n"       \
	"| |_/ ) || ___ | |  _ \\| ( \\ / )\n"    \
	"|  _ (| || ____| | | | | |) X (\n"       \
	"|_| \\_)\\_)_____)_|_| |_|_(_/ \\_)\n\n"


// entry.S: boot cpu jumps here in supervisor mode on stack0.
void
start()
{
	int hart_id = hartid();

	sbi_console_init();
	sbi_puts(BANNER);
	sbi_printf("%s v%s\n", OSNAME, VERSION);
	sbi_identify();
	cpu_identify(hart_id);
	sbi_printf("cpu%d: Hello World!!!\n", hart_id);
	sbi_printf("boot_hart_id: %d\n", boot_hart_id);

	/* Print device tree */
	void *dtb = fdt_get_dtb();
	sbi_printf("\nDevice Tree (DTB at 0x%lx):\n", (unsigned long)dtb);
	sbi_puts("----------------------------------------\n");
	fdt_print(dtb);
	sbi_puts("----------------------------------------\n\n");

	sbi_non_boot_hart_start((unsigned long)_entry);
	// assert boot_hart_id > 0;
	// report boot_hart_id
	// main();
	sbi_printf("cpu%d: system will shutdown in a few secs...\n", hart_id);
	delay(10);
	sbi_system_shutdown();
	sbi_hart_hang(); // unreachable
}

// non-boot cpu(s) jump here in supervisor mode on stack0.
void
non_boot_start(void)
{
	int hart_id = hartid();
	cpu_identify(hart_id);
	sbi_printf("cpu%d: non_boot_cpu\n", hart_id);
	if (hart_id == 1) { // testing enabling interrups in 1 core
		intrsinit();
		timerinit();
	}
	delay(10);       // not reached for now
	sbi_hart_hang(); // not reached for now
}
