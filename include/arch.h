/*
 * Copyright (C) 2015, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* infra
///
/// @copyright Copyright (C) 2015-2017, Syntacore Ltd. All Rights Reserved.
/// @author mn-sc
///
/// @brief Architecture specific defs and inlines

#ifndef SCR_INFRA_ARCH_H
#define SCR_INFRA_ARCH_H

#include <hal/arch.h>

#ifndef __ASSEMBLER__
void __attribute__((noreturn)) hart_halt(void);
#endif // __ASSEMBLER__

#endif // SCR_INFRA_ARCH_H
