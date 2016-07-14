/*
 * Copyright (C) 2022, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* framework
///
/// @copyright Copyright (C) 2022, Syntacore Ltd. All rights reserved.
/// @author dz-sc kn-sc
///
/// NB: minimal port of OpenSBI L2 PMU implementation
///
/// @brief L2 PMU funcs

#include "utils.h"
#include "l2_pmu.h"
#include "cache.h"
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

#if PLF_L2_PMU_SUPPORT

#define SCR_L2_PMU_CTR_NUM			4
#define SCR_L2_PMU_CONTROL_BASE		0x400
#define SCR_L2_CTR_LOW				0x08
#define SCR_L2_CTR_HIGH				0x0c

/** SCR L2 counters control registers */
#define SCR_PMU_BANKS_LOW_BIT	16
#define SCR_PMU_BANKS_HIGH_BIT	19
#define SCR_PMU_BANKS_SEL_MASK	GENMASK(SCR_PMU_BANKS_HIGH_BIT, SCR_PMU_BANKS_LOW_BIT)

#define SCR_L2_PMU_EVENT_SELECTOR	0
#define SCR_L2_PMU_EVENT_SELECTOR_MASK	GENMASK(3, 0)
#define get_cidx_addr(x, offset, cidx) ((x) + SCR_L2_PMU_CONTROL_BASE + (offset) + ((cidx) << 4))

/** Features flags */
#define L2_PMU_DEDICATED_FLAG		BIT(1)

#if PLF_CACHE_L2_DEDICATED
	#define DEDICATED __thread
#else
	#define DEDICATED
#endif

// #define VERBOSE_PRINT
#ifdef VERBOSE_PRINT
extern int printk(const char *fmt, ...);
#endif // VERBOSE_PRINT

/* Version identifier is taken from L2_VID
 * description of corresponding EAS
 */
#define SCR_L2_VID_V2          0x24062001
#define SCR_L2_CACHE_EVENTS_V1 9
#define SCR_L2_CACHE_EVENTS_V2 36

static int scr_l2_cache_events = 0;
static unsigned int scr_vid;

/**
 * @brief Static variables used during L2 pmu execution
 *
 */
static DEDICATED uint32_t active_events[SCR_L2_PMU_CTR_NUM];
static uint64_t l2_cache_addr = 0xffffffffffffffff;

/**
 * @brief Static functions used for L2 pmu execution
 *
 */
static uint32_t scr_l2_banks_num(uint64_t event_data)
{
	return (event_data & SCR_PMU_BANKS_SEL_MASK) >> SCR_PMU_BANKS_LOW_BIT;
}

static inline void scr_l2_pmu_ctr_write_event(uint32_t cidx, uint64_t ival)
{
	void *addr = (void *)get_cidx_addr(l2_cache_addr, 0x0, cidx);

	writel(ival, addr);
#ifdef VERBOSE_PRINT
	printk("%s : addr=0x%p, ival=0x%lx\n", __func__, addr, ival);
#endif //VERBOSE_PRINT
}

static inline void scr_l2_pmu_ctr_write_hw(uint32_t cidx, uint64_t ival)
{
	uint32_t low = 0, high = 0;
	void *addr_low = 0, *addr_high = 0;

	low = (uint32_t)(ival & 0xFFFFFFFF);
	high = (uint32_t)(ival >> 32);

	addr_low = (void *)get_cidx_addr(l2_cache_addr, SCR_L2_CTR_LOW, cidx);
	addr_high = (void *)get_cidx_addr(l2_cache_addr, SCR_L2_CTR_HIGH, cidx);

	writel(low, addr_low);
	writel(high, addr_high);

#ifdef VERBOSE_PRINT
	printk("W: addr_low=0x%p, addr_high=0x%p\n   ival: %lu\n   l: %lu\n   h: %lu\n\n", addr_low, addr_high, ival, low, high);
#endif // VERBOSE_PRINT
}

static inline void scr_l2_pmu_ctr_start_hw(uint32_t cidx, uint64_t ival, bool ival_update)
{
	if (ival_update)
		scr_l2_pmu_ctr_write_hw(cidx, ival);
}

static int pmu_ctr_validate(uint32_t cidx, uint32_t *event_idx_code)
{
	if (cidx >= SCR_L2_PMU_CTR_NUM || (active_events[cidx] == SBI_PMU_EVENT_IDX_INVALID))
		return SBI_ERROR_INVALID_PARAM;

	if (active_events[cidx] >= scr_l2_cache_events)
		return SBI_ERROR_INVALID_PARAM;

	return active_events[cidx];
}

static int scr_l2_pmu_reset_event(int ctr_idx)
{
	if (ctr_idx < 0 || ctr_idx >= SCR_L2_PMU_CTR_NUM)
		return SBI_ERROR_FAILURE;
	scr_l2_pmu_ctr_write_event(ctr_idx, 0);
	return 0;
}

static int scr_l2_pmu_read(uint32_t cidx, unsigned long *cval)
{
	unsigned long low = 0, high_prev = 0, high = 0;
	void *addr_low = 0, *addr_high = 0;

	addr_low  = (void *)get_cidx_addr(l2_cache_addr, SCR_L2_CTR_LOW, cidx);
	addr_high = (void *)get_cidx_addr(l2_cache_addr, SCR_L2_CTR_HIGH, cidx);

	high_prev = readl(addr_high);
	low = readl(addr_low);
	high = readl(addr_high);
#ifdef VERBOSE_PRINT
	printk("R: addr_low=0x%p, addr_high=0x%p\n   l: %lu\n   hp: %lu\n   h: %lu\n\n", addr_low, addr_high, low, high_prev, high);
#endif // VERBOSE_PRINT
	if (high != high_prev){
		low = readl(addr_low);
	}

	*cval = ((uint64_t)high << 32) | low;

#ifdef VERBOSE_PRINT
	printk("%s cidx=0x%x, val=%lu\n", __func__, cidx, *cval);
#endif // VERBOSE_PRINT
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
unsigned long sbi_l2_pmu_ctr_get_info(unsigned long cidx, unsigned long *out)
{
	return SBI_ERROR_NOT_SUPPORTED;
}

unsigned long sbi_l2_pmu_num_ctr(unsigned long *out)
{
	*out = SCR_L2_PMU_CTR_NUM;

	return SBI_ERROR_SUCCESS;
}

unsigned long sbi_l2_pmu_ctr_cfg_match(unsigned long cidx_base,
		unsigned long cidx_mask, unsigned long flags,
		unsigned long event_idx, unsigned long data1,
		unsigned long *out)
{
	int ctr_idx = SBI_ERROR_NOT_SUPPORTED;
	unsigned int event_type = event_idx & SCR_L2_PMU_EVENT_SELECTOR_MASK;
	uint64_t event_data;
	unsigned long tmp = cidx_mask << cidx_base;
	int i;
	uint64_t pmu_event_val = 0;

	event_data = data1;

#ifdef VERBOSE_PRINT
	printk("%s:\n\tcidx_base=0x%lx,\n\tcidx_mask=0x%lx,\n\tflags=0x%lx,\n\tevent_idx=0x%lx,\n\tevent_data=0x%lx\n",
		   __func__, cidx_base, cidx_mask, flags, event_idx, event_data);
#endif // VERBOSE_PRINT
	/* Do a basic sanity check of counter base & mask */
	if (sbi_fls(tmp) >= SCR_L2_PMU_CTR_NUM || event_type >= scr_l2_cache_events ||
		!scr_l2_banks_num(event_data))
		return SBI_ERROR_INVALID_PARAM;

	if (flags & SBI_PMU_CFG_FLAG_SKIP_MATCH) {
		if (active_events[cidx_base] == SBI_PMU_EVENT_IDX_INVALID)
			return SBI_ERROR_INVALID_PARAM;

		ctr_idx = cidx_base;
		goto skip_match;
	}

	if (event_idx >= scr_l2_cache_events)
		return SBI_ERROR_NOT_SUPPORTED;

	/* find free counter */
	for (i = 0; i < SCR_L2_PMU_CTR_NUM; i++) {
		if (active_events[i] == SBI_PMU_EVENT_IDX_INVALID)
			ctr_idx = i;
	}

	if (ctr_idx < 0)
		return SBI_ERROR_NOT_SUPPORTED;

	/*
	 * 31..20	RSV	RZ	0	Reserved
	 * 19..16	BANK	RW	0	Bank selector (one bit for each bank)
	 * 15..4	RSV	RZ	0	Reserved
	 * 3..0		PCE	RW	0	Performance counter event selector.
	 */
	/* set counters + banks */
	pmu_event_val = event_data | event_type;
	scr_l2_pmu_ctr_write_event(ctr_idx, pmu_event_val);
	active_events[ctr_idx] = event_idx;

skip_match:
	if (flags & SBI_PMU_CFG_FLAG_CLEAR_VALUE)
		scr_l2_pmu_ctr_write_hw(ctr_idx, 0);
	if (flags & SBI_PMU_CFG_FLAG_AUTO_START)
		scr_l2_pmu_ctr_start_hw(ctr_idx, 0, false);
#ifdef VERBOSE_PRINT
	printk("Ret %d\n", ctr_idx);
#endif // VERBOSE_PRINT
	*out = ctr_idx;
	return SBI_ERROR_SUCCESS;
}

unsigned long sbi_l2_pmu_ctr_start(unsigned long cbase,
		unsigned long cmask, unsigned long flags,
		unsigned long data1, unsigned long data2)
{
	int ret = SBI_ERROR_INVALID_PARAM;
	int event_idx_type;
	uint32_t event_code;
	unsigned long ctr_mask = cmask << cbase;
	bool bUpdate = false;
	uint64_t init;

#if __riscv_xlen == 32
	init = ((uint64_t)data2 << 32) | data1;
#else
	init = data1;
#endif

#ifdef VERBOSE_PRINT
	printk("%s:\n\tcidx_base=0x%lx,\n\tcidx_mask=0x%lx,\n\tflags=0x%lx,\n\tival=0x%lx\n",
		   __func__, cbase, cmask, flags, init);
#endif // VERBOSE_PRINT
	if (sbi_fls(ctr_mask) >= SCR_L2_PMU_CTR_NUM)
		return ret;

	if (flags & SBI_PMU_START_FLAG_SET_INIT_VALUE)
		bUpdate = true;

	for_each_set_bit_from(cbase, &ctr_mask, SCR_L2_PMU_CTR_NUM) {
		event_idx_type = pmu_ctr_validate(cbase, &event_code);
		if (event_idx_type < 0)
			/* Continue the start operation for other counters */
			continue;

		scr_l2_pmu_ctr_start_hw(cbase, init, bUpdate);
		ret = 0;
	}

	return ret;
}

unsigned long sbi_l2_pmu_ctr_stop(unsigned long cbase,
		unsigned long cmask, unsigned long flags)
{
	int event_idx_type;
	uint32_t event_code;
	unsigned long ctr_mask = cmask << cbase;

#ifdef VERBOSE_PRINT
	printk("%s:\n\tcidx_base=0x%lx,\n\tcidx_mask=0x%lx,\n\tflag=0x%lx\n",
		   __func__, cbase, cmask, flags);
#endif // VERBOSE_PRINT
	if (sbi_fls(ctr_mask) >= SCR_L2_PMU_CTR_NUM)
		return SBI_ERROR_INVALID_PARAM;

	for_each_set_bit_from(cbase, &ctr_mask, SCR_L2_PMU_CTR_NUM) {
		event_idx_type = pmu_ctr_validate(cbase, &event_code);
		if (event_idx_type < 0)
			/* Continue the stop operation for other counters */
			continue;

		if (flags & SBI_PMU_STOP_FLAG_RESET) {
			active_events[cbase] = SBI_PMU_EVENT_IDX_INVALID;
			scr_l2_pmu_reset_event(cbase);
		}
	}
	return SBI_ERROR_SUCCESS;
}

unsigned long sbi_l2_pmu_hw_ctr_read(unsigned long cidx,
		unsigned long *out)
{
	return (unsigned long)scr_l2_pmu_read(cidx, out);
}

unsigned long sbi_l2_pmu_features_flags(void)
{
	unsigned long flags = 0UL;

#if PLF_CACHE_L2_DEDICATED
	flags |= L2_PMU_DEDICATED_FLAG;
#endif

	return flags;
}

unsigned long sbi_l2_pmu_get_vid(unsigned long cidx,
		unsigned long *out)
{
	if (out)
		*out = scr_vid;

	return 0ul;
}

int scr_fdt_l2_pmu_init(void)
{
	int i;
#if PLF_L2CTL_BASE
	l2_cache_addr = (uint64_t)PLF_L2CTL_BASE;
	scr_vid = readl(L2_CSR_VER);
	if (scr_vid < SCR_L2_VID_V2) {
		scr_l2_cache_events = SCR_L2_CACHE_EVENTS_V1;
	} else {
		scr_l2_cache_events = SCR_L2_CACHE_EVENTS_V2;
	}
#ifdef VERBOSE_PRINT
	printk("l2_cache_addr : %p vid : 0x%x\n", l2_cache_addr, scr_vid);
#endif // VERBOSE_PRINT
#else
#error "L2 PMU requires PLF_L2CTL_BASE to be set"
#endif // PLF_L2CTL_BASE
	for (i = 0; i < SCR_L2_PMU_CTR_NUM; i++)
		active_events[i] = SBI_PMU_EVENT_IDX_INVALID;

	return 0;
}

#endif
