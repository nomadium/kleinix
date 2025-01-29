#include "sbi/sbi.h"
#include "riscv.h"
#include "param.h"

int hartid()
{
	return r_tp();
}

int cpuid()
{
	return r_tp();
}

void start_non_boot_harts(unsigned long entry_point)
{
	int i, hart_id;
	const char *warn, *error;
	struct sbiret ret;

	ret = sbi_probe_extension(SBI_EXT_HSM);
	if (!ret.value) {
		warn = "sbi: warning: HSM extension is not available.\n";
		error = "sbi: error: Failed to start non-boot harts.\n";
		sbi_puts(warn);
		sbi_puts(error);
		return;
	}

	hart_id = hartid();
	// XXX: look at devicetree data instead of attempting start
	// on non-existent harts...
	for (i = 0; i < NCPU; i++) {
		if (hart_id == i) continue;
		ret = sbi_hart_start(i, entry_point, 0);
	}
}

void
delay(int n)
{
	int c = 1000000000 * n;
	do { asm(""); } while (c--);
}

void
cpu_identify(int hart_id)
{
	unsigned long mvendorid, marchid, mimpid;
	struct sbiret ret;
	const char *cpu_id_fmt;
	cpu_id_fmt = "cpu%d: vendor %d arch %d imp %d\n";

	ret = sbi_get_mvendorid();
	mvendorid = ret.value;

	ret = sbi_get_marchid();
	marchid = ret.value;

	ret = sbi_get_mimpid();
	mimpid = ret.value;

	sbi_printf(cpu_id_fmt, hart_id, mvendorid, marchid, mimpid);
}
