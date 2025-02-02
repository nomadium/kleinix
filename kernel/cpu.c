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

static void
dummy_kernelvec(void)
{
	int _cpuid = cpuid();
	sbi_printf("cpu%d: interrupts are enabled\n", _cpuid);
	sbi_printf("cpu%d: hanging in interrupt handler\n", _cpuid);
	sbi_hart_hang();
}

void
intrsinit(void)
{
	// 4.1.2 supervisor trap vector
	w_stvec((uint64)dummy_kernelvec);

	// 4.1.1 enable all interrupts in S-mode.
	w_sstatus(r_sstatus() | SSTATUS_SIE);

	// 4.1.3 enabled S-mode interrupts, i.e. (external, timer, software)
	w_sie(r_sie() | SIE_SEIE | SIE_STIE | SIE_SSIE);
}

void
timerinit()
{
	// XXX: address magic number 1000000 (also check sbi_set_timer return)
	sbi_set_timer(rdtime() + 1000000);
}
