/*
 * Copyright (C) 2023, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* infra
///
/// @copyright Copyright (C) 2023, Syntacore Ltd. All Rights Reserved.
///
///
/// @brief Generic utility functions.

#ifndef SCR_UTILS_H

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

#define readl(addr) (*(volatile uint32_t*) (addr))
#define writel(b, addr) ((*(volatile uint32_t*) (addr)) = (b))

#if __riscv_xlen == 64

#define readq(addr) (*(volatile uint64_t*) (addr))
#define writeq(b, addr) ((*(volatile uint64_t*) (addr)) = (b))

#endif

#endif // SCR_UTILS_H
