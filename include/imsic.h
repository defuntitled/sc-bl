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
/// @copyright Copyright (C) 2022, Syntacore Ltd. All Rights Reserved.
///
/// @brief IMSIC defines and inline functions

#ifndef SCR_IMSIC_H
#define SCR_IMSIC_H

#include "platform_config.h"

#if PLF_AIA_SUPPORT

#define IMSIC_MMIO_PAGE_SHIFT		12
#define IMSIC_MMIO_PAGE_SZ		(1UL << IMSIC_MMIO_PAGE_SHIFT)

#define IMSIC_MAX_REGS			16

#define CSR_MISELECT			0x350
#define CSR_MIREG			0x351

#define CSR_MTOPEI			0x35c
#define CSR_MTOPI			0xfb0

#define CSR_MVIEN			0x308
#define CSR_MVIP			0x309

#define IMSIC_MMIO_PAGE_LE		0x00
#define IMSIC_MMIO_PAGE_BE		0x04

#define IMSIC_MIN_ID			63
#define IMSIC_MAX_ID			2047

#define IMSIC_EIDELIVERY		0x70

#define IMSIC_EITHRESHOLD		0x72

#define IMSIC_TOPEI			0x76
#define IMSIC_TOPEI_ID_SHIFT		16
#define IMSIC_TOPEI_ID_MASK		0x7ff
#define IMSIC_TOPEI_PRIO_MASK		0x7ff

#define IMSIC_EIP0			0x80

#define IMSIC_EIP63			0xbf

#define IMSIC_EIPx_BITS			32

#define IMSIC_EIE0			0xc0

#define IMSIC_EIE63			0xff

#define IMSIC_EIEx_BITS			32

#define IMSIC_DISABLE_EIDELIVERY	0
#define IMSIC_ENABLE_EIDELIVERY		1
#define IMSIC_DISABLE_EITHRESHOLD	1
#define IMSIC_ENABLE_EITHRESHOLD	0

#define IMSIC_IPI_ID			1

#define imsic_csr_write(__c, __v)	\
do { \
	write_csr(CSR_MISELECT, __c); \
	write_csr(CSR_MIREG, __v); \
} while (0)

#define imsic_csr_read(__c)	\
({ \
	unsigned long __v; \
	write_csr(CSR_MISELECT, __c); \
	__v = read_csr(CSR_MIREG); \
	__v; \
})

#define imsic_csr_set(__c, __v)		\
do { \
	write_csr(CSR_MISELECT, __c); \
	set_csr(CSR_MIREG, __v); \
} while (0)

#define imsic_csr_clear(__c, __v)	\
do { \
	write_csr(CSR_MISELECT, __c); \
	clear_csr(CSR_MIREG, __v); \
} while (0)

void imsic_init(void);

#else

static inline void imsic_init(void)
{
}

#endif // PLF_AIA_SUPPORT

#endif // SCR_IMSIC_H
