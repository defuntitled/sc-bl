/*
 * Copyright (C) 2016, Syntacore Ltd.
 * All Rights Reserved.
 */

#ifndef SCR_VM_H
#define SCR_VM_H

#include "platform_config.h"
#include <string.h>
#include <stdint.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLAMP(a, lo, hi) MIN(MAX(a, lo), hi)
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#define ROUNDUP(a, b) ((((a)-1)/(b)+1)*(b))
#define ROUNDDOWN(a, b) ((a)/(b)*(b))

#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

#define EXTRACT_FIELD(val, which) (((val) & (which)) / ((which) & ~((which)-1)))
#define INSERT_FIELD(val, which, fieldval) (((val) & ~(which)) | ((fieldval) * ((which) & ~((which)-1))))

#define mb() __sync_synchronize()

typedef struct {
    long gpr[32];
    long status;
    long epc;
    long badvaddr;
    long cause;
    long insn;
} trapframe_t;

extern uintptr_t mem_size;

#if PLF_SMP_SUPPORT
extern volatile unsigned num_harts;
#if !PLF_SMP_ICCM_SUPPORT
extern volatile unsigned slot_bits_shift;
#endif
#else
#define num_harts (1)
#endif // PLF_SMP_SUPPORT

int printk(const char *fmt, ...) __attribute__((format (printf, 1, 2)));

void machine_init(void);

typedef uintptr_t pte_t;
void __attribute__((noreturn))
bad_trap(void);
void __attribute__((noreturn))
bad_trap_handler(uintptr_t mcause, uintptr_t* regs, uintptr_t cycle, uintptr_t instr);

#endif // SCR_VM_H
