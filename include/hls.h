/*
 * Copyright (C) 2015, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* infra
///
/// @copyright Copyright (C) 2015-2020, Syntacore Ltd. All Rights Reserved.
/// @author mn-sc
///
/// @brief Hart Local Storage defs and funcs

#ifndef SCR_INFRA_HLS_H
#define SCR_INFRA_HLS_H

#include "platform_config.h"

#define HLS_HART_STATE_HALT 0
#define HLS_HART_STATE_RUN 1

/* hart-local stack size */
#if PLF_L2_STACK_8CORE
#define HLSHIFT (PGSHIFT - 1)
#define HLSIZE  (1UL << HLSHIFT)
#else 
#define HLSHIFT PGSHIFT
#define HLSIZE  (1UL << HLSHIFT)
#endif

#ifndef __ASSEMBLER__

#include "arch.h"
#include "csr.h"
#include "sbi.h"
#include "mmu.h"

#include <stdlib.h>
#include <stdbool.h>
#include "stringify.h"
#include "sbi_vendor.h"

typedef struct {
    unsigned long ipi_n;
    unsigned long cpuid;
    unsigned long hartid;
    unsigned long state;
    unsigned long active;
#ifdef PLF_CACHE_CFG
    unsigned l1i_info;
    unsigned l1d_info;
#endif // PLF_CACHE_CFG
#if PLF_L2CTL_BASE
    unsigned l2_descr;
#endif // PLF_L2CTL_BASE
} hls_t;

typedef struct hart_storage {
#ifndef __riscv_32e
    unsigned long xreg[32];
#else // ! __riscv_32e
    unsigned long xreg[16];
#endif // !__riscv_32e
    hls_t hls;
} hart_storage;

extern __thread hart_storage hart_local_storage;
extern char __C_STACK_TOP__, __TLS0_BASE__;
extern char _tdata_begin[], _tdata_end[], _tbss_end[], TLS_SIZE[];

#define HLS() (&(hart_local_storage.hls))

static inline unsigned long __attribute__((const)) hart_stack_page_top(unsigned hartn)
{
#if __riscv_xlen == 32
    unsigned long pn;

    asm ("lui  %0, %%hi(__C_STACK_TOP__);" /* top stack page */
         "srli %0, %0, " __xstringify(HLSHIFT) ";"
         "sub  %0, %0, %1;"
         : "=r" (pn), "+r" (hartn) : "1" (hartn) :);

    return pn;
#else  // __riscv_xlen == 32
    return (uintptr_t)&__C_STACK_TOP__ / HLSIZE - hartn;
#endif // __riscv_xlen == 32
}

static inline void * __attribute__((const)) hart_tls(unsigned hartn)
{
#if __riscv_xlen == 32
    void *addr;

    asm ("lui  %0, %%hi(__TLS0_BASE__);"
         "addi %0, %0, %%lo(__TLS0_BASE__);"
         "slli %1, %1, " __xstringify(HLSHIFT) ";"
         "sub  %0, %0, %1;"
         : "=r" (addr), "+r" (hartn) : "1" (hartn) :);

    return addr;
#else  // __riscv_xlen == 32
    return &__TLS0_BASE__ - hartn * HLSIZE;
#endif // __riscv_xlen == 32
}

/* logical HART number */
static inline unsigned long __attribute__((const)) logical_hart(void)
{
#if __riscv_xlen == 32
    unsigned long hart_pos;

    asm ("lui  %0, %%hi(__C_STACK_TOP__);" /* top stack page */
         "sub  %0, %0, sp;"
         "srli %0, %0, " __xstringify(HLSHIFT) ";"
         : "=r" (hart_pos) ::);

    return hart_pos;
#else  // __riscv_xlen == 32
    register uintptr_t sp asm ("sp");

    return ((uintptr_t)&__C_STACK_TOP__ - sp) / HLSIZE;
#endif  // __riscv_xlen == 32
}

static inline hls_t * __attribute__((const)) hart_hls(unsigned hartn)
{
    return (hls_t*)((void*)HLS() + (logical_hart() - hartn) * HLSIZE);
}

#endif // !__ASSEMBLER__
#endif // SCR_INFRA_HLS_H
