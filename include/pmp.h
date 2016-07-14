/*
 * Copyright (C) 2023, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* infra
///
/// @copyright Copyright (C) 2023, Syntacore Ltd. All Rights Reserved.
/// @author gdi-sc and is-sc
///
/// @brief PMP defs and funcs

#ifndef SCR_INFRA_PMP_H
#define SCR_INFRA_PMP_H

#define PMP_A_SHIFT 3     // bit shift to get A value

#include <hal/drivers/pmp.h>

#if PLF_PMP_SUPPORT
#ifndef __ASSEMBLER__

#include <stddef.h>
#include <stdint.h>

static inline void pmp_decode_napot(uint64_t region, uint64_t *addr, uint64_t *range)
{
    /*
       aaaa...aaa0   8-byte NAPOT range
       aaaa...aa01   16-byte NAPOT range
       aaaa...a011   32-byte NAPOT range
       ...
       aa01...1111   2^XLEN-byte NAPOT range
       a011...1111   2^(XLEN+1)-byte NAPOT range
       0111...1111   2^(XLEN+2)-byte NAPOT range
       1111...1111   Reserved
    */
    uint64_t a = (region << 2) | 0x3;
    *addr = a & (a + 1);
    *range = (a | (a + 1)) - *addr + 1;
}

#else // #ifndef __ASSEMBLER__

#define PMP_ALL (PMP_NAPOT | PMP_RWX)
#define PMP_STRONG_ALL (PMP_ALL | SCR_PMP_CTRL_MT_STRONG)

.macro pmp_set_region csr, addr, mask
    li t0, \mask;
    not t0, t0;
    srli t0, t0, 3;
    li t1, \addr;
    srli t1, t1, PMP_SHIFT;
    or t0, t0, t1;
    csrw \csr, t0;
.endm // pmp_set_region

.macro pmp_set_region_config csr, offset, value:vararg
    csrr t0, \csr;
    li t1, (0xff << \offset);
    not t1, t1;
    and t0, t0, t1;
    li t1, (\value << \offset);
    or t0, t0, t1;
    csrw csr, t0;
.endm // pmp_set_region_config

.macro pmp_early_init
    pmp_reset_init;
    fence.i; fence;
#ifdef PLF_MMCFG_BASE
    pmp_set_region CSR_PMPADDR1, PLF_MMCFG_BASE, ~(PLF_MMCFG_SIZE-1);
    pmp_set_region_config CSR_PMPCFG0, 8, (PMP_NAPOT | SCR_PMP_CTRL_MT_CONFIG );
#endif // PLF_MMCFG_BASE
#ifdef PLF_MMIO_BASE
    pmp_set_region CSR_PMPADDR2, PLF_MMIO_BASE, ~(PLF_MMIO_SIZE-1);
    pmp_set_region_config CSR_PMPCFG0, 16, PMP_STRONG_ALL;
#endif // PLF_MMIO_BASE
    fence.i; fence;
#ifdef PLF_CLUSTER_CFG_BASE
    pmp_set_region CSR_PMPADDR3, PLF_CLUSTER_CFG_BASE, ~(PLF_CLUSTER_CFG_SIZE-1);
    pmp_set_region_config CSR_PMPCFG0, 24, PMP_STRONG_ALL;
#endif // PLF_CLUSTER_CFG_BASE
    pmp_set_region CSR_PMPADDR4, PLF_MTIMER_BASE, ~(PLF_MMIO_SIZE-1);
    pmp_set_region_config CSR_PMPCFG0, 32, PMP_STRONG_ALL;
    pmp_set_region CSR_PMPADDR5, PLF_OCRAM_BASE, ~(PLF_OCRAM_SIZE-1);
    pmp_set_region_config CSR_PMPCFG0, 40, PMP_STRONG_ALL;
.endm // pmp_early_init

#endif // #ifndef __ASSEMBLER__
#endif // #if PLF_PMP_SUPPORT

#endif // SCR_INFRA_PMP_H
