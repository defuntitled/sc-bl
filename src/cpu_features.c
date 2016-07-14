/*
 * Copyright (C) 2023, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* framework
///
/// @copyright Copyright (C) 2023, Syntacore Ltd. All rights reserved.
/// @author dsm-sc
///
/// @brief Syntacore CPU features extension

#include "cpu_features.h"
#include "sbi.h"
#include "csr.h"
#include "bitops.h"

#define MSLGN           (9)
#define MSLGN_MASK      BIT(MSLGN)

#if PLF_MISALIGN_ACCESS_SUPPORT
void plf_set_misalign(void)
{
#if PLF_MISALIGN_ACCESS_ACTIVATE
	if (read_csr(FEAT_AUTH_CSR) & MSLGN_MASK) {
		set_csr(FEAT_EN_CSR, MSLGN_MASK);
	}
#endif /* PLF_MISALIGN_ACCESS_ACTIVATE */
}
#endif /* PLF_MISALIGN_ACCESS_SUPPORT */
