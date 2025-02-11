#include "sbi/sbi.h"
#include "sbi/sbi_console.h"
#include "spinlock.h"
#include "cpu.h"

unsigned int
holding(spinlock_t *lock);

void
spin_lock(spinlock_t *lock)
{
	if (lock->locked)
		sbi_panic("spin_lock");

	// On RISC-V, sync_lock_test_and_set turns into an atomic swap:
	//   a5 = 1
	//   s1 = &lock->locked
	//   amoswap.w.aq a5, a5, (s1)
	while (__sync_lock_test_and_set(&lock->locked, 1) != 0);
	// Tell the C compiler and the processor to not move loads or stores
	// past this point, to ensure that the critical section's memory
	// references happen strictly after the lock is acquired.
	// On RISC-V, this emits a fence instruction.
	__sync_synchronize();

	// Record info about lock acquisition for holding() and debugging.
	lock->cpu = cpuid();
}

void
spin_unlock(spinlock_t *lock)
{

	if (!holding(lock))
		sbi_panic("spin_unlock");

	lock->cpu = 0;

	// Tell the C compiler and the CPU to not move loads or stores
	// past this point, to ensure that all the stores in the critical
	// section are visible to other CPUs before the lock is released,
	// and that loads in the critical section occur strictly before
	// the lock is released.
	// On RISC-V, this emits a fence instruction.
	__sync_synchronize();
	// Release the lock, equivalent to lock->locked = 0.
	// This code doesn't use a C assignment, since the C standard
	// implies that an assignment might be implemented with
	// multiple store instructions.
	// On RISC-V, sync_lock_release turns into an atomic swap:
	//   s1 = &lock->locked
	//   amoswap.w zero, zero, (s1)
	__sync_lock_release(&lock->locked);
}

unsigned int
holding(spinlock_t *lock)
{
	int r;
	r = (lock->locked && lock->cpu == cpuid());
	return r;
}
