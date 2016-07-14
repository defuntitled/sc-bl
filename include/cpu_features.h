/*
 * Copyright (C) 2023, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* framework
///
/// @copyright Copyright (C) 2023, Syntacore Ltd. All Rights Reserved.
/// @author dsm-sc
///
/// @brief Syntacore CPU features extension

#ifndef SC_BL_SRC_CPU_FEATURES_H_
#define SC_BL_SRC_CPU_FEATURES_H_

#include "platform_config.h"

#if PLF_MISALIGN_ACCESS_SUPPORT
void plf_set_misalign(void);
#endif /* PLF_MISALIGN_ACCESS_SUPPORT */

#endif /* SC_BL_SRC_CPU_FEATURES_H_ */
