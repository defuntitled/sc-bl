/*
 * Copyright (C) 2024, Syntacore Ltd.
 * All Rights Reserved.
 */

/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2020 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Atish Patra <atish.patra@wdc.com>
 */

/// Syntacore SCR* infra
///
/// @copyright Copyright (C) 2024, Syntacore Ltd. All rights reserved.
///
/// @brief Check accessibility of MMIO registers

#include <stdint.h>

#include "reg_detect.h"

extern void __mmio_detect_trap(void);

int mmio_read_allowed(uintptr_t addr, unsigned long *pmcause)
{
    register uintptr_t mtvec = (uintptr_t)&__mmio_detect_trap;
    register uintptr_t info asm("a3");
    register uintptr_t t1 asm("a4");
    register uintptr_t mstatus = 0;
    unsigned long mcause = -1UL;

    info = (uintptr_t)&mcause;

    asm volatile(
        "csrrw %[mtvec], mtvec, %[mtvec]\n"
        "csrr %[mstatus], mstatus\n"
        ".option push\n"
        // match with mepc increment by 4 in __mmio_detect_trap:
        // set norvc to make sure that faulting ld/lw is 4 bytes long
        ".option norvc\n"
#if __riscv_xlen == 32
        "lw %[t1], 0(%[addr])\n"
#else  // __riscv_xlen == 32
        "ld %[t1], 0(%[addr])\n"
#endif // __riscv_xlen == 32
        ".option pop\n"
        "csrw mstatus, %[mstatus]\n"
        "csrw mtvec, %[mtvec]\n"
        : [mstatus] "+&r"(mstatus), [mtvec] "+&r"(mtvec), [info] "+&r"(info), [t1] "+&r"(t1)
        : [addr] "r" (addr)
        : "memory");

    if (pmcause && (mcause != -1UL)) {
        *pmcause = mcause;
    }

    return (mcause == (-1UL));
}
