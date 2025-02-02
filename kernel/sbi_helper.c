#include "sbi/sbi.h"
#include "cpu.h"
#include "param.h"

enum sbi_imp {
	OpenSBI = 1,
};

static const char *sbi_imp_name[] = {
	[OpenSBI]    "OpenSBI",
};

static struct sbi_console_device console_dev = {
	.name         = "sbi_console",
	.console_putc = NULL,
	.console_puts = NULL,
	.console_getc = NULL,
};

void
sbi_console_init(void)
{
	struct sbiret ret;
	struct sbi_console_device *_console_dev;
	const char *warn;

	_console_dev = &console_dev;

	ret = sbi_probe_extension(SBI_EXT_DBCN);
	if (ret.value) {
		_console_dev->console_puts = &sbi_debug_console_puts;
		_console_dev->console_getc = &sbi_debug_console_getchar;
	}

	_console_dev->console_putc = &sbi_legacy_console_putchar;

	if (!_console_dev->console_getc)
		_console_dev->console_getc = &sbi_legacy_console_getchar;

	sbi_console_set_device(_console_dev);

	if (!_console_dev->console_puts) {
		warn = "sbi: warning: deprecated sbi_console_putchar extension in use.\n";
		sbi_puts(warn);
	}
}

void
sbi_identify(void)
{
	struct sbiret ret;
	unsigned long impl_id;
	int impl_major, impl_minor;
	int spec_major, spec_minor;
	char *impl_info_fmt, *spec_info_fmt;
	const char *impl_name;

	ret = sbi_get_impl_id();
	impl_id = ret.value;
	ret = sbi_get_impl_version();
	impl_major = ret.value >> 16;
	impl_minor = ret.value & 0xFFFF;
	if (impl_id == OpenSBI) {
		impl_name = sbi_imp_name[impl_id];
		impl_info_fmt = "SBI: %s v%d.%d";
		sbi_printf(impl_info_fmt, impl_name, impl_major, impl_minor);
	} else {
		/* There are multiple known SBI implementations but only OpenSBI
		 * has been tested. Rework this bit if/when other impls are tested.
		 * https://github.com/riscv-non-isa/riscv-sbi-doc/releases/download/v2.0/riscv-sbi.pdf
		 * 4.9. SBI Implementation IDs */
		impl_info_fmt = "SBI: Unknown implementation";
		sbi_puts(impl_info_fmt);
	}

	spec_info_fmt = ", SBI Specification Version %d.%d\n";
	ret = sbi_get_spec_version();
	spec_minor = ret.value & 0xFFFFFF;
	spec_major = (ret.value >> 24) & 0x7F;
	sbi_printf(spec_info_fmt, spec_major, spec_minor);
}

void
sbi_non_boot_hart_start(unsigned long entry_point)
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

void __attribute__((noreturn))
sbi_hart_hang(void)
{
	while (1)
		asm volatile("wfi");
	__builtin_unreachable();
}

struct sbiret
sbi_set_timer(uint64_t stime_value)
{
	struct sbiret ret;
	long r;

	ret = sbi_probe_extension(SBI_EXT_TIME);
	if (!ret.value) {
		if ((r = sbi_legacy_set_timer(stime_value)))
			ret.error = r;
		ret.value = 0;
		return ret;
	}

	ret = sbi_timer_set_timer(stime_value);
	return ret;
}
