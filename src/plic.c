/*
 * Copyright (C) 2022, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* framework
///
/// @copyright Copyright (C) 2022, Syntacore Ltd. All rights reserved.
///
///
/// @brief PLIC funcs
#include "plic.h"
#include "utils.h"

#ifdef PLF_PLIC_BASE

extern int printk(const char *fmt, ...);

struct plic_irq_modes {
	uint32_t line;
	uint32_t mode;
};

static int plic_get_irq_num(void)
{
	return PLF_PLIC_IRQ_NUM;
}

void plic_init(bool dbg_print)
{
	uint32_t irq_num = plic_get_irq_num();

	// configure all IRQs
	for (int i = 1; i < irq_num; ++i)
		PLF_SCR_PLIC_MODES[i] = SCR_PLIC_SRC_MODE_OFF;

#ifdef PLF_PLIC_IRQ_CFG
	struct plic_irq_modes irq_modes[] = { PLF_PLIC_IRQ_CFG };

	static const char *plic_mode_descr[SCR_PLIC_SRC_MODE_MAX + 1] = {
		"OFF",
		"High-Level",
		"Low-Level",
		"Rising-Edge",
		"Falling-Edge",
		"Both-Edge",
	};

	for (unsigned i = 0; i < ARRAY_SIZE(irq_modes); ++i)
		PLF_SCR_PLIC_MODES[irq_modes[i].line + 1] = irq_modes[i].mode;

	if (dbg_print) {
		printk("init: PLIC configuration (line:mode):");
		for (unsigned i = 0; i < ARRAY_SIZE(irq_modes); ++i) {
		    if (irq_modes[i].mode < ARRAY_SIZE(plic_mode_descr))
			printk(" %u:%s", (unsigned)irq_modes[i].line, plic_mode_descr[irq_modes[i].mode]);
		    else
			printk(" %u:%u", (unsigned)irq_modes[i].line, (unsigned)irq_modes[i].mode);
		}
		printk("\n");
	}
#else // PLF_PLIC_IRQ_CFG
	if (dbg_print)
		printk("init: PLIC configuration: none\n");
#endif // PLF_PLIC_IRQ_CFG
}

#endif // PLF_PLIC_BASE
