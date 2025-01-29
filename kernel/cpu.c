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
