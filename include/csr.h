/*
 * Copyright (C) 2015, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* infra
///
/// @copyright Copyright (C) 2015-2023, Syntacore Ltd. All Rights Reserved.
/// @author mn-sc
///
/// @brief Architecture specific CSR's defs and inlines

#ifndef SCR_INFRA_CSR_H
#define SCR_INFRA_CSR_H

#include <hal/csr.h>

// interrupts
#define INT_SM_SOFTWARE     1
#define INT_MM_SOFTWARE     3
#define INT_SM_TIMER        5
#define INT_MM_TIMER        7
#define INT_SM_EXTERNAL     9
#define INT_MM_EXTERNAL     11
#define INT_MM_MEMORY_FAULT 15
// Misaligned access fault
#define EXC_MIS_INSTR       0
#define EXC_MIS_LOAD        4
#define EXC_MIS_STORE       6
// Memory access fault
#define EXC_MEM_FETCH       1
#define EXC_MEM_LOAD        5
#define EXC_MEM_STORE       7
// System Calls
#define EXC_UM_ECALL        8
#define EXC_SM_ECALL        9
#define EXC_MM_ECALL        11
// Page Faults
#define EXC_PF_FETCH        12
#define EXC_PF_LOAD         13
#define EXC_PF_STORE        15
// custom exc: TLB miss
#define EXC_TLB_MISS        14

// Common CSRs not define in old gcc
#define CSR_VSSTATUS 0x200
#define CSR_VSTVEC   0x205
#define CSR_VSEPC    0x241
#define CSR_VSCAUSE  0x242
#define CSR_VSTVAL   0x243

#if __riscv_xlen == 32
#define CSR_MSTATUSH 0x310
#endif

#define CSR_MTINST   0x34A
#define CSR_MTVAL2   0x34B

#define CSR_HTVAL    0x643
#define CSR_HTINST   0x64A

// mstatus/h bits
#define CSR_MSTATUS_SIE_SHIFT  1
#define CSR_MSTATUS_MIE_SHIFT  3
#define CSR_MSTATUS_SPIE_SHIFT 5
#define CSR_MSTATUS_UBE_SHIFT  6
#define CSR_MSTATUS_MPIE_SHIFT 7
#define CSR_MSTATUS_SPP_SHIFT  8
#define CSR_MSTATUS_VS_SHIFT   9
#define CSR_MSTATUS_MPP_SHIFT  11
#define CSR_MSTATUS_FS_SHIFT   13
#define CSR_MSTATUS_XS_SHIFT   15
#define CSR_MSTATUS_MPRV_SHIFT 17
#define CSR_MSTATUS_SUM_SHIFT  18
#define CSR_MSTATUS_MXR_SHIFT  19
#define CSR_MSTATUS_TVM_SHIFT  20
#define CSR_MSTATUS_TW_SHIFT   21
#define CSR_MSTATUS_TSR_SHIFT  22

#if __riscv_xlen == 32
#define CSR_MSTATUSH_SBE_SHIFT  4
#define CSR_MSTATUSH_MBE_SHIFT  5
#define CSR_MSTATUSH_GVA_SHIFT  6
#define CSR_MSTATUSH_MPV_SHIFT  7
#else
#define CSR_MSTATUS_UXL_SHIFT  32
#define CSR_MSTATUS_SXL_SHIFT  34
#define CSR_MSTATUS_SBE_SHIFT  36
#define CSR_MSTATUS_MBE_SHIFT  37
#define CSR_MSTATUS_GVA_SHIFT  38
#define CSR_MSTATUS_MPV_SHIFT  39
#endif

// hstatus bits
#define CSR_HSTATUS_VSBE_SHIFT 5
#define CSR_HSTATUS_GVA_SHIFT  6
#define CSR_HSTATUS_SPV_SHIFT  7
#define CSR_HSTATUS_SPVP_SHIFT 8
#define CSR_HSTATUS_HU_SHIFT   9

#define CSR_MENVCFG 0x30A
#if __riscv_xlen == 32
#define CSR_MENVCFGH 0x31A
#endif

#if __riscv_xlen > 32
#define CSR_MENVCFG_STCE_SHIFT   63
#define CSR_MENVCFG_PBMTE_SHIFT  62
#else
#define CSR_MENVCFGH_STCE_SHIFT  31
#define CSR_MENVCFGH_PBMTE_SHIFT 30
#endif

#define CSR_MENVCFG_CBZE_SHIFT   7
#define CSR_MENVCFG_CBCFE_SHIFT  6
#define CSR_MENVCFG_CBIE_SHIFT   4
#define CSR_MENVCFG_CBIE_ILL     0x0UL
#define CSR_MENVCFG_CBIE_FLUSH   0x1UL
#define CSR_MENVCFG_CBIE_INV     0x3UL
#define CSR_MENVCFG_FIOM_SHIFT   0

#define CSR_MSTATEEN0                   0x30C
#define CSR_MSTATEEN1                   0x30D
#define CSR_MSTATEEN2                   0x30E
#define CSR_MSTATEEN3                   0x30F
#if __riscv_xlen == 32
#define CSR_MSTATEEN0H                  0x31C
#define CSR_MSTATEEN1H                  0x31D
#define CSR_MSTATEEN2H                  0x31E
#define CSR_MSTATEEN3H                  0x31F
#endif

#define CSR_MSTATEEN0_CS_SHIFT             0
#define CSR_MSTATEEN0_FCSR_SHIFT           1
#define CSR_MSTATEEN0_CONTEXT_SHIFT        57
#define CSR_MSTATEEN0_IMSIC_SHIFT          58
#define CSR_MSTATEEN0_AIA_SHIFT            59
#define CSR_MSTATEEN0_SVSLCT_SHIFT         60
#define CSR_MSTATEEN0_HSENVCFG_SHIFT       62
#define CSR_MSTATEEN_STATEN_SHIFT          63

#define rdtime() read_csr(time)
#define rdtimeh() read_csr(timeh)
#define rdcycle() read_csr(cycle)

#define FEAT_AUTH_CSR   (0xFF0)
#define FEAT_EN_CSR     (0xBE0)
#define L1D_PF_CTRL0    (0xBF0)

#define SCR_MISELECT_CSR                0x350
#define SCR_VCR_SCHEMA_CSR              0x351
#define SCR_CORE_TIMESTAMP_ID_CSR       0x352
#define SCR_CORE_BUILD_TARGET_ID_CSR    0x355
#define SCR_CORE_CFG_ID_CSR             0x356

#endif // SCR_INFRA_CSR_H
