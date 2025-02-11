#ifndef __SPINLOCK__
#define __SPINLOCK__

// Mutual exclusion lock for SBI debug console output functions
typedef struct spinlock {
	unsigned int locked;
	unsigned int cpu;
} spinlock_t;


#define __SPIN_LOCK_UNLOCKED	\
	(spinlock_t) { 0, 0 }

#define SPIN_LOCK_INITIALIZER	\
	__SPIN_LOCK_UNLOCKED


void
spin_lock(spinlock_t *lock);

void
spin_unlock(spinlock_t *lock);

#endif /* __SPINLOCK__ */
