#include "sbi/sbi.h"
#include "param.h"
#include "riscv.h"
#include "klibc.h"

// entry.S needs one stack per CPU.
__attribute__ ((aligned (16))) char stack0[4096 * NCPU];

// function to print to the console during early boot.
extern void (*sbi_console_puts)(const char *);

extern void _entry(void);

int boot_hart_id = -1;
void non_boot_start(void);

#define OSNAME  "Kleinix"
#define VERSION "0.0.1"
#define BANNER                                    \
	" _     _        _       _\n"             \
	"| |   | |      (_)     (_)\n"            \
	"| |  _| | _____ _ ____  _ _   _\n"       \
	"| |_/ ) || ___ | |  _ \\| ( \\ / )\n"    \
	"|  _ (| || ____| | | | | |) X (\n"       \
	"|_| \\_)\\_)_____)_|_| |_|_(_/ \\_)\n\n"

void sbi_start_non_boot_harts(void)
{
	int i, hart_id;
	const char *warn;
	struct sbiret ret;

	ret = sbi_probe_extension(SBI_EXT_HSM);
	if (!ret.value) {
		warn = "sbi: warning: HSM extension is not available.\n";
		sbi_console_puts(warn);
		return;
	}

	hart_id = r_tp();
	// XXX: look at devicetree data instead of attempting start
	// on non-existent harts...
	for (i = 0; i < NCPU; i++) {
		if (hart_id == i) continue;
		ret = sbi_hart_start(i, (unsigned long)_entry, 0);
	}
}

void
delay(int n)
{
	int c = 1000000000 * n;
	do {} while (c--);
}

void
cpu_identify(int hart_id)
{
	unsigned long mvendorid, marchid, mimpid;
	struct sbiret ret;
	const char *cpu_id_fmt;

	ret = sbi_get_mvendorid();
	mvendorid = ret.value;

	ret = sbi_get_marchid();
	marchid = ret.value;

	ret = sbi_get_mimpid();
	mimpid = ret.value;

	cpu_id_fmt = "cpu%d: vendor %d arch %d imp %d\n";
	printf(cpu_id_fmt, hart_id, mvendorid, marchid, mimpid);
}

// entry.S: boot cpu jumps here in supervisor mode on stack0.
void
start()
{
	int hart_id = r_tp();

	sbi_console_init();
	sbi_console_puts(BANNER);
	printf("%s v%s\n", OSNAME, VERSION);
	sbi_print_base_info();
	cpu_identify(hart_id);
	printf("cpu%d: Hello World!!!\n", hart_id);
	sbi_start_non_boot_harts();
	// assert boot_hart_id > 0;
	// report boot_hart_id
	// main();
	printf("cpu%d: system will shutdown in a few secs...", hart_id);
	delay(3);
	sbi_system_shutdown();
	for (;;) ;
}

// non-boot cpu(s) jump here in supervisor mode on stack0.
void
non_boot_start(void)
{
	int hart_id = r_tp();
	cpu_identify(hart_id);
	printf("cpu%d: non_boot_cpu\n", hart_id);
	for (;;) ;
}
