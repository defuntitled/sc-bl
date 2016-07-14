/*
 * Copyright (C) 2024, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* infra
///
/// @copyright Copyright (C) 2024, Syntacore Ltd. All Rights Reserved.
///
/// @brief Check accessibility of MMIO registers

#ifndef SCR_REG_DETECT_H
#define SCR_REG_DETECT_H

int mmio_read_allowed(uintptr_t addr, unsigned long *pmcause);

#endif // SCR_REG_DETECT_H
