/*
 * Copyright (C) 2015, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* infra
///
/// @copyright Copyright (C) 2015-2017, Syntacore Ltd. All Rights Reserved.
/// @author mn-sc
///
/// @brief Spinlock defs and inlines

#ifndef SCR_INFRA_SPINLOCK_H
#define SCR_INFRA_SPINLOCK_H

#include "platform_config.h"
#include "arch.h"
#include "cache.h"

#include <stdbool.h>

#if PLF_SMP_NO_ATOMIC_SPINLOCK && PLF_SMP_SUPPORT
/* 8-bit-per-hart mask size in xlen words */
#define PLF_HART8_MASK_SIZE ((((PLF_SMP_HART_NUM) * 8) + (__riscv_xlen - 1)) / __riscv_xlen)
#define PLF_HART8_XLEN (__riscv_xlen / 8)

typedef struct arch_spinlock {
	volatile unsigned long flags[PLF_HART8_MASK_SIZE];
} arch_spinlock_t;

#define SPINLOCK_INIT(i) {(0)}

void arch_spin_unlock(arch_spinlock_t *lock);
void arch_spin_lock(arch_spinlock_t *lock);

#elif defined(__riscv_atomic) && PLF_SMP_SUPPORT // PLF_SMP_NO_ATOMIC_SPINLOCK

typedef struct arch_spinlock {
	volatile unsigned int lock;
} arch_spinlock_t;

static inline bool arch_spin_is_locked(arch_spinlock_t *lock)
{
#if PLF_SMP_NON_COHERENT
    cache_invalidate((void*)&(lock->lock), sizeof(lock->lock));
#endif // PLF_SMP_NON_COHERENT
    return lock->lock != 0;
}

static inline void arch_spin_unlock_wait(arch_spinlock_t *lock)
{
    do {
        cpu_relax();
    } while (arch_spin_is_locked(lock));
}

#define SPINLOCK_INIT(i) {(i)}

static inline void arch_spin_unlock(arch_spinlock_t *lock)
{
	__asm__ __volatile__ (
		"amoswap.w.rl x0, x0, %0"
		: "=A" (lock->lock)
		:: "memory");
}

static inline int arch_spin_trylock(arch_spinlock_t *lock)
{
	int tmp = 1, busy;

	__asm__ __volatile__ (
		"amoswap.w.aq %0, %2, %1"
		: "=r" (busy), "+A" (lock->lock)
		: "r" (tmp)
		: "memory");

	return !busy;
}

static inline void arch_spin_lock(arch_spinlock_t *lock)
{
	while (1) {
		if (arch_spin_is_locked(lock))
			continue;

		if (arch_spin_trylock(lock))
			break;
	}
}

#else // PLF_SMP_SUPPORT && (PLF_SMP_NO_ATOMIC_SPINLOCK || __riscv_atomic)

typedef struct {
} arch_spinlock_t;

#define arch_spin_is_locked(x)	(0)
#define arch_spin_unlock_wait(x) \
		do { } while (0)

#define SPINLOCK_INIT(i) {}

static inline void arch_spin_unlock(arch_spinlock_t *lock)
{
    (void)lock;
}

static inline int arch_spin_trylock(arch_spinlock_t *lock)
{
    (void)lock;

    return 1;
}

static inline void arch_spin_lock(arch_spinlock_t *lock)
{
    (void)lock;
}
#endif // spinlock variants

// basic atomic support

typedef struct sc_atomic_struct {
    int counter;
} sc_atomic;

static inline int atomic_read(const sc_atomic *v)
{
#if PLF_SMP_NON_COHERENT
    cache_invalidate((void*)&(v->counter), sizeof(v->counter));
#endif // PLF_SMP_NON_COHERENT
	return *((volatile int *)(&(v->counter)));
}

static inline void atomic_set(sc_atomic *v, int i)
{
    *((volatile int *)(&(v->counter))) = i;
#if PLF_SMP_NON_COHERENT
    cache_flush((void*)&(v->counter), sizeof(v->counter));
#endif // PLF_SMP_NON_COHERENT
}

#if defined(__riscv_atomic)
static inline int atomic_add(int i, sc_atomic *v)
{
	int out;

	__asm__ __volatile__ (
		"amoadd.w %1, %2, %0"
		: "+A" (v->counter), "=r" (out)
		: "r" (i) : "memory");
	return out;
}
#elif PLF_SMP_SUPPORT // __riscv_atomic
// atomic ops use global locking
int atomic_add(int i, sc_atomic *v);
#else // __riscv_atomic || PLF_SMP_SUPPORT
static inline int atomic_add(int i, sc_atomic *v)
{
    int out = atomic_read(v);
    atomic_set(v, out + i);
    return out;
}
#endif // __riscv_atomic || PLF_SMP_SUPPORT
#endif // SCR_INFRA_SPINLOCK_H
