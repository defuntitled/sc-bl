/*
 * Copyright (C) 2022, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* framework
///
/// @copyright (C) Western Digital Corporation or its affiliates.
/// @copyright (C) 2022 Ventana Micro Systems Inc.
/// @copyright Copyright (C) 2022, Syntacore Ltd. All rights reserved.
///
/// NB: minimal port of OpenSBI implementation
///
/// @brief APLIC defines and inline functions

#include <stdint.h>

#include "bitops.h"
#include "aplic.h"
#include "imsic.h"
#include "utils.h"

#if PLF_AIA_SUPPORT

#define APLIC_xMSICFGADDRH_L           BIT(31)
#define APLIC_xMSICFGADDRH_HHXS_MASK   0x1f
#define APLIC_xMSICFGADDRH_HHXS_SHIFT  24
#define APLIC_xMSICFGADDRH_LHXS_MASK   0x7
#define APLIC_xMSICFGADDRH_LHXS_SHIFT  20
#define APLIC_xMSICFGADDRH_HHXW_MASK   0x7
#define APLIC_xMSICFGADDRH_HHXW_SHIFT  16
#define APLIC_xMSICFGADDRH_LHXW_MASK   0xf
#define APLIC_xMSICFGADDRH_LHXW_SHIFT  12
#define APLIC_xMSICFGADDRH_BAPPN_MASK  0xfff

#define APLIC_xMSICFGADDR_PPN_SHIFT    12

#define APLIC_xMSICFGADDR_PPN_HART(__lhxs) \
	((1UL << (__lhxs)) - 1)

#define APLIC_xMSICFGADDR_PPN_LHX_MASK(__lhxw) \
	((1UL << (__lhxw)) - 1)
#define APLIC_xMSICFGADDR_PPN_LHX_SHIFT(__lhxs) \
	((__lhxs))
#define APLIC_xMSICFGADDR_PPN_LHX(__lhxw, __lhxs) \
	(APLIC_xMSICFGADDR_PPN_LHX_MASK(__lhxw) << \
	 APLIC_xMSICFGADDR_PPN_LHX_SHIFT(__lhxs))

#define APLIC_xMSICFGADDR_PPN_HHX_MASK(__hhxw) \
	((1UL << (__hhxw)) - 1)
#define APLIC_xMSICFGADDR_PPN_HHX_SHIFT(__hhxs) \
	((__hhxs) + APLIC_xMSICFGADDR_PPN_SHIFT)
#define APLIC_xMSICFGADDR_PPN_HHX(__hhxw, __hhxs) \
	(APLIC_xMSICFGADDR_PPN_HHX_MASK(__hhxw) << \
	 APLIC_xMSICFGADDR_PPN_HHX_SHIFT(__hhxs))

struct aplic_msicfg_data {
	unsigned long lhxs;
	unsigned long lhxw;
	unsigned long hhxs;
	unsigned long hhxw;
	unsigned long base_addr;
};

struct aplic_delegate_data {
	uint32_t first_irq;
	uint32_t last_irq;
	uint32_t child_index;
};

struct aplic_data {
	unsigned long addr;
	unsigned long size;
	unsigned long num_source;
	struct aplic_delegate_data delegate[APLIC_MAX_DELEGATE];
};

static void aplic_write_msiconfig(struct aplic_msicfg_data *msicfg,
				  void *msicfgaddr, void *msicfgaddrH)
{
	uint32_t val;
	unsigned long base_ppn;

	/* Check if MSI config is already locked */
	if (readl(msicfgaddrH) & APLIC_xMSICFGADDRH_L)
		return;

	/* Compute the MSI base PPN */
	base_ppn = msicfg->base_addr >> APLIC_xMSICFGADDR_PPN_SHIFT;
	base_ppn &= ~APLIC_xMSICFGADDR_PPN_HART(msicfg->lhxs);
	base_ppn &= ~APLIC_xMSICFGADDR_PPN_LHX(msicfg->lhxw, msicfg->lhxs);
	base_ppn &= ~APLIC_xMSICFGADDR_PPN_HHX(msicfg->hhxw, msicfg->hhxs);

	/* Write the lower MSI config register */
	writel((uint32_t)base_ppn, msicfgaddr);

	/* Write the upper MSI config register */
	val = (((uint64_t)base_ppn) >> 32) &
		APLIC_xMSICFGADDRH_BAPPN_MASK;
	val |= (msicfg->lhxw & APLIC_xMSICFGADDRH_LHXW_MASK)
		<< APLIC_xMSICFGADDRH_LHXW_SHIFT;
	val |= (msicfg->hhxw & APLIC_xMSICFGADDRH_HHXW_MASK)
		<< APLIC_xMSICFGADDRH_HHXW_SHIFT;
	val |= (msicfg->lhxs & APLIC_xMSICFGADDRH_LHXS_MASK)
		<< APLIC_xMSICFGADDRH_LHXS_SHIFT;
	val |= (msicfg->hhxs & APLIC_xMSICFGADDRH_HHXS_MASK)
		<< APLIC_xMSICFGADDRH_HHXS_SHIFT;

	/* Write the upper MSI config register */
	writel(val, msicfgaddrH);
}

void aplic_init(void)
{
	uint32_t i, j;
	struct aplic_delegate_data *deleg;
	uint32_t first_deleg_irq, last_deleg_irq;
	uint32_t domain_cfg = APLIC_DOMAINCFG_DM;
	struct aplic_msicfg_data msicfg = {};
	struct aplic_data aplic_base = {
		.addr = PLF_APLIC_BASE,
		.size = PLF_APLIC_SIZE,
		.num_source = PLF_APLIC_NUM_SOURCE,
		.delegate = { PLF_APLIC_DELEGATE }
	};
	struct aplic_data *aplic = &aplic_base;

	/* Set MSI delivery mode, LE byte order, interrupts disabled globally */
	writel(domain_cfg, (void *)(aplic->addr + APLIC_DOMAINCFG));

	/* Disable all interrupts */
	for (i = 0; i <= aplic->num_source; i++)
		writel(-1U, (void *)(aplic->addr + APLIC_CLRIE_BASE +
				     (i / 32) * sizeof(uint32_t)));

	/* Set interrupt type and priority for all interrupts */
	for (i = 1; i <= aplic->num_source; i++) {
		/* Set IRQ source configuration to 0 */
		writel(0, (void *)(aplic->addr + APLIC_SOURCECFG_BASE +
			  (i - 1) * sizeof(uint32_t)));
		/* Set IRQ target hart index and priority to 1 */
		writel(APLIC_DEFAULT_PRIORITY, (void *)(aplic->addr +
						APLIC_TARGET_BASE +
						(i - 1) * sizeof(uint32_t)));
	}

	/* Configure IRQ delegation */
	first_deleg_irq = -1U;
	last_deleg_irq = 0;
	for (i = 0; i < APLIC_MAX_DELEGATE; i++) {
		deleg = &aplic->delegate[i];
		if (!deleg->first_irq || !deleg->last_irq)
			continue;
		if (aplic->num_source < deleg->first_irq ||
		    aplic->num_source < deleg->last_irq)
			continue;
		if (APLIC_SOURCECFG_CHILDIDX_MASK < deleg->child_index)
			continue;
		if (deleg->first_irq > deleg->last_irq) {
			uint32_t tmp = deleg->first_irq;
			deleg->first_irq = deleg->last_irq;
			deleg->last_irq = tmp;
		}
		if (deleg->first_irq < first_deleg_irq)
			first_deleg_irq = deleg->first_irq;
		if (last_deleg_irq < deleg->last_irq)
			last_deleg_irq = deleg->last_irq;
		for (j = deleg->first_irq; j <= deleg->last_irq; j++)
			writel(APLIC_SOURCECFG_D | deleg->child_index,
				(void *)(aplic->addr + APLIC_SOURCECFG_BASE +
				(j - 1) * sizeof(uint32_t)));
	}

	msicfg.base_addr = PLF_IMSIC_BASE_M;
	msicfg.lhxw = PLF_APLIC_LHXW_M;          // low hart index shift
	msicfg.lhxs = PLF_APLIC_LHXS_M;          // low hart index width
	msicfg.hhxw = PLF_APLIC_HHXW_M;          // high hart index width
	msicfg.hhxs = 2 * IMSIC_MMIO_PAGE_SHIFT; // high hart index shift
	aplic_write_msiconfig(&msicfg,
			     (void *)(aplic->addr + APLIC_MMSICFGADDR),
			     (void *)(aplic->addr + APLIC_MMSICFGADDRH));

	msicfg.base_addr = PLF_IMSIC_BASE_S;
	msicfg.lhxw = PLF_APLIC_LHXW_S;
	msicfg.lhxs = PLF_APLIC_LHXS_S;
	msicfg.hhxw = PLF_APLIC_HHXW_S;
	msicfg.hhxs = 2 * IMSIC_MMIO_PAGE_SHIFT ;
	aplic_write_msiconfig(&msicfg,
			     (void *)(aplic->addr + APLIC_SMSICFGADDR),
			     (void *)(aplic->addr + APLIC_SMSICFGADDRH));
}

#endif // PLF_AIA_SUPPORT
