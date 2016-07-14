/*
 * Copyright (C) 2015, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* infra
///
/// @copyright Copyright (C) 2015-2022, Syntacore Ltd. All Rights Reserved.
/// @author mn-sc
///
/// @brief MMU defs and funcs

#ifndef SCR_INFRA_MMU_H
#define SCR_INFRA_MMU_H

#include "arch.h"
#include <hal/drivers/mpu.h>

#define PGSHIFT  12
#define PGSIZE   (1UL << PGSHIFT)
#define PGMASK   (PGSIZE - 1)

// MMU CSRs
#define CSR_MMU_BASE             0xbc0
#define CSR_MMU_PATTR            (CSR_MMU_BASE + 0)
#define CSR_MMU_VADDR            (CSR_MMU_BASE + 1)
#define CSR_MMU_UPDATE           (CSR_MMU_BASE + 2)
#define CSR_MMU_SCAN             (CSR_MMU_BASE + 3)

// constants
#define CSR_MMU_UPDATE_TLBI      (1UL << 0)
#define CSR_MMU_UPDATE_TLBD      (1UL << 1)
#define CSR_MMU_UPDATE_MPAGE     (1UL << 2)
#define CSR_MMU_UPDATE_GPAGE     (2UL << 2)
#define CSR_MMU_UPDATE_TPAGE     (3UL << 2)

#define CSR_MMU_SCAN_TYPE_OFF    31

// TLB-miss hw helper regs
#if __riscv_xlen == 32
#define CSR_TLB_PTE1_ADDR        0xfd0
#define CSR_TLB_PTE0_OFFS        0xfd1
#else // __riscv_xlen == 32
#define CSR_TLB_PTE2_ADDR        0xfd0
#define CSR_TLB_PTE1_OFFS        0xfd1
#define CSR_TLB_PTE0_OFFS        0xfd2
#endif // __riscv_xlen == 32

#endif // SCR_INFRA_MMU_H
