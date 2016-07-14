/*
 * Copyright (C) 2023, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* framework
///
/// @copyright Copyright (C) 2023, Syntacore Ltd. All rights reserved.
/// @author dsm-sc
///
///
/// @brief L3 PMU funcs

#include "utils.h"
#include "l3_pmu.h"
#include "cache.h"
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

#if PLF_L3_PMU_SUPPORT

#if !PLF_L3CTL_BASE
#error "L3 PMU requires PLF_L3CTL_BASE to be set"
#endif // PLF_L3CTL_BASE

#define SCR_L3_PMU_EVENT_SELECTOR       0
#define SCR_L3_PMU_EVENT_SELECTOR_MASK  GENMASK(7, 0)
#define SCR_L3_PMU_BANKS_LOW_BIT        8
#define SCR_L3_PMU_BANKS_HIGH_BIT       15
#define SCR_L3_PMU_BANKS_SEL_MASK       \
    GENMASK(SCR_L3_PMU_BANKS_HIGH_BIT, SCR_L3_PMU_BANKS_LOW_BIT)

/* default number of counters */
#ifndef SCR_L3_PMU_COUNTERS
#define SCR_L3_PMU_COUNTERS             4
#endif

// #define VERBOSE_PRINT
#ifdef VERBOSE_PRINT
extern int printk(const char *fmt, ...);
#define debug(...) printk(__VA_ARGS__)
#else
#define debug(...)
#endif // VERBOSE_PRINT

/* Version identifier is taken from L3_VID
 * description of corresponding EAS
 */
#define SCR_L3_VID_V1           0x2022010101
#define SCR_L3_CACHE_EVENTS_V1  15
#define SCR_L3_CACHE_EVENTS_V2  16

/* v1 and v2 event codes starts from 1 */
#define SCR_L3_CACHE_NOP     0

static int scr_l3_cache_events = 0;
static uint64_t scr_vid;

/**
 * @brief Static variables used during L3 pmu execution
 *
 */
struct l3_pmu_event {
	uint64_t ctrl;
	uint32_t banks_mask;
	int banks;
};
static struct l3_pmu_event active_events[SCR_L3_PMU_COUNTERS];
static uint32_t supported_banks = 0U;
static uint32_t supported_banks_mask = 0U;

/**
 * @brief Static functions used for L3 pmu execution
 *
 */
static uint32_t scr_l3_banks_mask_num(uint64_t event_data)
{
	return (event_data & SCR_L3_PMU_BANKS_SEL_MASK) >> SCR_L3_PMU_BANKS_LOW_BIT;
}

static bool scr_l3_is_event_active(struct l3_pmu_event *e)
{
	return (e->ctrl != SCR_L3_CACHE_NOP);
}

static inline void scr_l3_pmu_ctr_write_event(uint32_t cidx, uint64_t ival)
{
	void *addr = (void *)L3C_PCE_CTRL_N(cidx);
	writeq(ival, addr);

	debug("%s : addr=0x%p, ival=0x%lx\n", __func__, addr, ival);
}

static inline void scr_l3_pmu_bank_write_ctrl(unsigned long banks_mask,
                                     uint32_t cidx, uint64_t ival)
{
	int bank = 0;

	for_each_set_bit_from(bank, &banks_mask, supported_banks) {
		void *addr = (void *)L3B_PCE_CTRL_N(bank, cidx);
		writeq(ival, addr);

		debug("%s : addr=0x%p, ival=0x%lx\n", __func__, addr, ival);
	}
}

static int scr_l3_pmu_event_validate(uint32_t cidx)
{
	if (cidx >= SCR_L3_PMU_COUNTERS)
		return SBI_ERROR_INVALID_PARAM;

	if (!scr_l3_is_event_active(&active_events[cidx]))
		return SBI_ERROR_INVALID_PARAM;

	return 0;
}

static int scr_l3_pmu_read_cntr(unsigned long banks_mask,
		                        uint32_t cidx, unsigned long *dval)
{
	uint64_t val = 0U;
	int bank = 0;

	for_each_set_bit_from(bank, &banks_mask, supported_banks) {
		val += readq(L3B_PCE_CNTR_N(bank, cidx));

		debug("%s addr=0x%x bank=0x%x, cidx=0x%x, val=%lu\n", __func__,
				L3B_PCE_CNTR_N(bank, cidx), bank, cidx, val);
	}
	*dval = val;

	return 0;
}

static int scr_l3_pmu_write_cntr(unsigned long banks_mask, uint32_t cidx, uint64_t val)
{
	int bank = 0;

	for_each_set_bit_from(bank, &banks_mask, supported_banks) {
		void *addr = (void *)L3B_PCE_CNTR_N(bank, cidx);

		debug("%s addr=0x%x bank=0x%x, cidx=0x%x, val=%lu\n", __func__, addr, bank, cidx, val);

		writeq(val, addr);
	}

	return 0;
}

static int scr_l3_pmu_ctr_stop_hw(uint32_t cidx)
{
	scr_l3_pmu_ctr_write_event(cidx, SCR_L3_CACHE_NOP);
	return 0;
}

static inline void scr_l3_pmu_ctr_start_hw(uint32_t banks_mask, uint32_t cidx, uint64_t ctrl_val,
                                           uint64_t cntr_val, bool cntr_update)
{
	if (cntr_update)
		scr_l3_pmu_write_cntr(banks_mask, cidx, cntr_val);

	scr_l3_pmu_bank_write_ctrl(banks_mask, cidx, ctrl_val);

	debug("%s banks_mask=0x%x, cidx=0x%x, ctrl=0x%lx, val=%lu\n", __func__, banks_mask, cidx, ctrl_val, cntr_val);
}


unsigned long sbi_l3_pmu_ctr_get_info(unsigned long cidx, unsigned long *out)
{
	return SBI_ERROR_NOT_SUPPORTED;
}

unsigned long sbi_l3_pmu_num_ctr(unsigned long *out)
{
	*out = SCR_L3_PMU_COUNTERS;

	return SBI_ERROR_SUCCESS;
}

unsigned long sbi_l3_pmu_ctr_cfg_match(unsigned long cidx_base,
		unsigned long cidx_mask, unsigned long flags,
		unsigned long event_idx, unsigned long data1,
		unsigned long *out)
{
	int ctr_idx = SBI_ERROR_NOT_SUPPORTED;
	unsigned int event_type = event_idx & SCR_L3_PMU_EVENT_SELECTOR_MASK;
	uint64_t event_data;
	unsigned long cnt_mask = cidx_mask << cidx_base;
	uint32_t banks_mask;
	int banks;
	int i;

	event_data = data1;
	banks_mask = scr_l3_banks_mask_num(event_data);

	debug("%s:\n\tcidx_base=0x%lx,\n\tcidx_mask=0x%lx,\n\tflags=0x%lx,\n\tevent_idx=0x%lx,\n\tevent_data=0x%lx\n",
		   __func__, cidx_base, cidx_mask, flags, event_idx, event_data);

	if (!banks_mask)
		return SBI_ERROR_INVALID_PARAM;

	if (event_type >= scr_l3_cache_events || event_type == SCR_L3_CACHE_NOP)
		return SBI_ERROR_NOT_SUPPORTED;

	if (flags & SBI_PMU_CFG_FLAG_SKIP_MATCH) {
		ctr_idx = sbi_ffs(cnt_mask);

		if (ctr_idx >= SCR_L3_PMU_COUNTERS ||
				!scr_l3_is_event_active(&active_events[ctr_idx]))
			return SBI_ERROR_INVALID_PARAM;

		goto skip_match;
	}

	if (sbi_fls(cnt_mask) >= SCR_L3_PMU_COUNTERS)
		return SBI_ERROR_NOT_SUPPORTED;

	/* find free counter */
	for (i = 0; i < SCR_L3_PMU_COUNTERS; i++) {
		if (!scr_l3_is_event_active(&active_events[i])) {
			ctr_idx = i;
			break;
		}
	}

	if (ctr_idx < 0)
		return SBI_ERROR_NOT_SUPPORTED;

	banks_mask &= supported_banks_mask;
	banks = __builtin_popcount(banks_mask);

	if (!banks)
		return SBI_ERROR_INVALID_PARAM;

	/* set events for specified banks */
	active_events[ctr_idx].banks_mask = banks_mask;
	active_events[ctr_idx].ctrl = event_type;
	active_events[ctr_idx].banks = banks;

skip_match:
	if (flags & SBI_PMU_CFG_FLAG_CLEAR_VALUE)
		/* clear counter for all banks */
		scr_l3_pmu_write_cntr(~0U, ctr_idx, 0);
	if (flags & SBI_PMU_CFG_FLAG_AUTO_START)
		scr_l3_pmu_ctr_start_hw(banks_mask, ctr_idx, event_type, 0, false);

	debug("Ret %d\n", ctr_idx);

	*out = ctr_idx;
	return SBI_ERROR_SUCCESS;
}

unsigned long sbi_l3_pmu_ctr_start(unsigned long cbase,
		unsigned long cmask, unsigned long flags,
		unsigned long data1, unsigned long data2)
{
	int ret = SBI_ERROR_INVALID_PARAM;
	unsigned long ctr_mask = cmask << cbase;
	bool bUpdate = false;
	uint64_t init;

#if __riscv_xlen == 32
	init = ((uint64_t)data2 << 32) | data1;
#else
	init = data1;
#endif

	debug("%s:\n\tcidx_base=0x%lx,\n\tcidx_mask=0x%lx,\n\tflags=0x%lx,\n\tival=0x%lx\n",
		   __func__, cbase, cmask, flags, init);

	if (sbi_fls(ctr_mask) >= SCR_L3_PMU_COUNTERS)
		return ret;

	if (flags & SBI_PMU_START_FLAG_SET_INIT_VALUE)
		bUpdate = true;

	for_each_set_bit_from(cbase, &ctr_mask, SCR_L3_PMU_COUNTERS) {
		struct l3_pmu_event *event;
		uint64_t init_val = init;

		if (scr_l3_pmu_event_validate(cbase))
			/* Continue the start operation for other counters */
			continue;

		event = &active_events[cbase];

		/* As we have counters per bank
		 * we have to div init value on
		 * number of banks
		 */
		init_val = init_val / event->banks;

		scr_l3_pmu_ctr_start_hw(event->banks_mask, cbase,
			event->ctrl, init_val, bUpdate);
		ret = 0;
	}

	return ret;
}

unsigned long sbi_l3_pmu_ctr_stop(unsigned long cbase,
		unsigned long cmask, unsigned long flags)
{
	unsigned long ctr_mask = cmask << cbase;

	debug("%s:\n\tcidx_base=0x%lx,\n\tcidx_mask=0x%lx,\n\tflag=0x%lx\n",
		   __func__, cbase, cmask, flags);

	if (sbi_fls(ctr_mask) >= SCR_L3_PMU_COUNTERS)
		return SBI_ERROR_INVALID_PARAM;

	for_each_set_bit_from(cbase, &ctr_mask, SCR_L3_PMU_COUNTERS) {
		if (scr_l3_pmu_event_validate(cbase))
			/* Continue the stop operation for other counters */
			continue;

		if (flags & SBI_PMU_STOP_FLAG_RESET) {
			active_events[cbase].ctrl = SCR_L3_CACHE_NOP;
		}

		/* Stop timer */
		scr_l3_pmu_ctr_stop_hw(cbase);
	}
	return SBI_ERROR_SUCCESS;
}

unsigned long sbi_l3_pmu_hw_ctr_read(unsigned long cidx,
		unsigned long *out)
{
	if (scr_l3_pmu_event_validate(cidx))
		return SBI_ERROR_INVALID_PARAM;
	return (unsigned long)scr_l3_pmu_read_cntr(active_events[cidx].banks_mask, cidx, out);
}

unsigned long sbi_l3_pmu_features_flags(void)
{
	unsigned long flags = 0UL;

	return flags;
}

unsigned long sbi_l3_pmu_get_vid(unsigned long cidx,
		unsigned long *out)
{
	if (out)
		*out = scr_vid;

	return 0ul;
}

int scr_fdt_l3_pmu_init(void)
{
	int i;

	supported_banks = (readq(L3C_DESCR_CACHE) >> L3C_DESCR_CACHE_OFFS_BANK_NUM) &
			L3C_DESCR_CACHE_MASK_BANK_NUM;

	if (supported_banks <= sizeof(supported_banks_mask) * 8 && supported_banks > 0)
		supported_banks_mask = GENMASK(supported_banks - 1, 0);

	scr_vid = readq(L3C_VID);
	if (scr_vid <= SCR_L3_VID_V1) {
		scr_l3_cache_events = SCR_L3_CACHE_EVENTS_V1;
	} else {
		scr_l3_cache_events = SCR_L3_CACHE_EVENTS_V2;
	}

	debug("L3 banks %d, L3 VID : 0x%lx\n", supported_banks, scr_vid);

	for (i = 0; i < SCR_L3_PMU_COUNTERS; i++)
		active_events[i].ctrl = SCR_L3_CACHE_NOP;

	return 0;
}

#endif
