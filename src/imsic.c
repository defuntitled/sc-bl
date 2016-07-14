/*
 * Copyright (C) 2022, Syntacore Ltd.
 * All Rights Reserved.
 */

/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Western Digital Corporation or its affiliates.
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

/// Syntacore SCR* framework
///
/// @copyright (C) 2021 Western Digital Corporation or its affiliates.
/// @copyright (C) 2022 Ventana Micro Systems Inc.
/// @copyright Copyright (C) 2022, Syntacore Ltd. All rights reserved.
///
/// NB: minimal adaptation from OpenSBI implementation
///
/// @brief Implementaion module for IMSIC

#include <stdint.h>

#include "bitops.h"
#include "csr.h"
#include "sbi.h"
#include "imsic.h"

#if PLF_AIA_SUPPORT
static void imsic_local_eix_update(unsigned long base_id,
				   unsigned long num_id, bool pend, bool val)
{
	unsigned long i, isel, ireg;
	unsigned long id = base_id, last_id = base_id + num_id;

	while (id < last_id) {
		isel = id / __riscv_xlen;
		isel *= __riscv_xlen / IMSIC_EIPx_BITS;
		isel += (pend) ? IMSIC_EIP0 : IMSIC_EIE0;

		ireg = 0;
		for (i = id & (__riscv_xlen - 1);
		     (id < last_id) && (i < __riscv_xlen); i++) {
			ireg |= BIT(i);
			id++;
		}

		if (val)
			imsic_csr_set(isel, ireg);
		else
			imsic_csr_clear(isel, ireg);
	}
}

void imsic_init(void)
{
	/* Disable all interrupts */
	imsic_local_eix_update(1, PLF_IMSIC_NUM_IDS, false, false);

	/* Clear IPI pending */
	imsic_local_eix_update(IMSIC_IPI_ID, 1, true, false);

	/* Setup threshold to allow all enabled interrupts */
	imsic_csr_write(IMSIC_EITHRESHOLD, IMSIC_ENABLE_EITHRESHOLD);

	/* Enable interrupt delivery */
	imsic_csr_write(IMSIC_EIDELIVERY, IMSIC_ENABLE_EIDELIVERY);

	/* Enable IPI */
	imsic_local_eix_update(IMSIC_IPI_ID, 1, false, true);
}

#endif /* PLF_AIA_SUPPORT */
