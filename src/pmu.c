/*
 * Copyright (C) 2022, Syntacore Ltd.
 * All Rights Reserved.
 */

/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Atish Patra <atish.patra@wdc.com>
 */

/// Syntacore SCR* framework
///
/// @copyright Copyright (C) 2022, Syntacore Ltd. All rights reserved.
/// @author sm-sc
///
/// NB: minimal port of OpenSBI PMU implementation
///
/// @brief PMU funcs

#include "pmu.h"

#if PLF_PMU_SUPPORT

#ifdef VERBOSE_PRINT
extern int printk(const char *fmt, ...);
#define debug(...) printk(__VA_ARGS__)
#else
#define debug(...)
#endif // VERBOSE_PRINT

extern const struct sbi_pmu_hw_event hw_sbi_events[];
extern const struct sbi_pmu_hw_event hw_raw_events[];

static __thread uint64_t active_events[PMU_HW_CTR_MAX + 1];
static unsigned long pmu_hw_ctr_avail;

static void csr_write_num(int csr_num, unsigned long val)
{
#define switchcase_csr_write(__csr_num, __val)      \
    case __csr_num:                 \
        write_csr(__csr_num, __val);        \
        break;
#define switchcase_csr_write_2(__csr_num, __val)    \
    switchcase_csr_write(__csr_num + 0, __val)  \
    switchcase_csr_write(__csr_num + 1, __val)
#define switchcase_csr_write_4(__csr_num, __val)    \
    switchcase_csr_write_2(__csr_num + 0, __val)    \
    switchcase_csr_write_2(__csr_num + 2, __val)
#define switchcase_csr_write_8(__csr_num, __val)    \
    switchcase_csr_write_4(__csr_num + 0, __val)    \
    switchcase_csr_write_4(__csr_num + 4, __val)
#define switchcase_csr_write_16(__csr_num, __val)   \
    switchcase_csr_write_8(__csr_num + 0, __val)    \
    switchcase_csr_write_8(__csr_num + 8, __val)
#define switchcase_csr_write_32(__csr_num, __val)   \
    switchcase_csr_write_16(__csr_num + 0, __val)   \
    switchcase_csr_write_16(__csr_num + 16, __val)
#define switchcase_csr_write_64(__csr_num, __val)   \
    switchcase_csr_write_32(__csr_num + 0, __val)   \
    switchcase_csr_write_32(__csr_num + 32, __val)

    switch (csr_num) {
    switchcase_csr_write(CSR_MCYCLE, val)
    switchcase_csr_write(CSR_MINSTRET, val)
    switchcase_csr_write(CSR_MHPMCOUNTER3, val)
    switchcase_csr_write_4(CSR_MHPMCOUNTER4, val)
    switchcase_csr_write_8(CSR_MHPMCOUNTER8, val)
    switchcase_csr_write_16(CSR_MHPMCOUNTER16, val)
#if __riscv_xlen == 32
    switchcase_csr_write(CSR_MCYCLEH, val)
    switchcase_csr_write(CSR_MINSTRETH, val)
    switchcase_csr_write(CSR_MHPMCOUNTER3H, val)
    switchcase_csr_write_4(CSR_MHPMCOUNTER4H, val)
    switchcase_csr_write_8(CSR_MHPMCOUNTER8H, val)
    switchcase_csr_write_16(CSR_MHPMCOUNTER16H, val)
    switchcase_csr_write(CSR_MHPMEVENT3H, val)
    switchcase_csr_write_4(CSR_MHPMEVENT4H, val)
    switchcase_csr_write_8(CSR_MHPMEVENT8H, val)
    switchcase_csr_write_16(CSR_MHPMEVENT16H, val)
#endif
    switchcase_csr_write(CSR_MHPMEVENT3, val)
    switchcase_csr_write_4(CSR_MHPMEVENT4, val)
    switchcase_csr_write_8(CSR_MHPMEVENT8, val)
    switchcase_csr_write_16(CSR_MHPMEVENT16, val)

    default:
        __builtin_unreachable();
        break;
    };

#undef switchcase_csr_write_64
#undef switchcase_csr_write_32
#undef switchcase_csr_write_16
#undef switchcase_csr_write_8
#undef switchcase_csr_write_4
#undef switchcase_csr_write_2
#undef switchcase_csr_write
}

static int pmu_find_raw_event_selector(uint64_t event_data, uint64_t *selector)
{
#if PLF_PMU_STRICT_EVENTS
    uint32_t mask = 0u;
    int res = SBI_ERROR_NOT_SUPPORTED;
    int i;

    switch (arch_coreid()) {
    case 7:
        mask = SCR7_SUPPORT;
        break;
    case 9:
        mask = SCR9_SUPPORT;
        break;
    default:
        debug("Unsupported core id (%lu)\n", arch_coreid());
        return SBI_ERROR_INVALID_PARAM;
    }

    for (i = 0; hw_raw_events[i].hw_raw_event.select != SBI_PMU_EVENT_NOT_SUPP; i++) {
        if (event_data == hw_raw_events[i].hw_raw_event.select) {
            if (hw_raw_events[i].hw_raw_event.arch_mask & mask) {
                *selector = hw_raw_events[i].hw_raw_event.select;
                res = SBI_ERROR_SUCCESS;
            }

            break;
        }
    }

    return res;
#else
#warning Using unsafe PMU code
    *selector = event_data;
    return SBI_ERROR_SUCCESS;
#endif
}

static int pmu_find_event_selector(unsigned long event_idx, uint64_t event_data, uint64_t *selector)
{
    int type = get_cidx_type(event_idx);
    int i;

    switch (type) {
    case SBI_PMU_EVENT_TYPE_HW:
    case SBI_PMU_EVENT_TYPE_HW_CACHE:
        for (i = 0; hw_sbi_events[i].hw_gen_event.event_idx != SBI_PMU_EVENT_IDX_INVALID; i++) {
            if (event_idx == hw_sbi_events[i].hw_gen_event.event_idx) {
                if (hw_sbi_events[i].hw_gen_event.select != SBI_PMU_EVENT_NOT_SUPP) {
                    *selector = hw_sbi_events[i].hw_gen_event.select;
                    return SBI_ERROR_SUCCESS;
                }
            }
        }
        break;
    case SBI_PMU_EVENT_TYPE_HW_RAW:
        return pmu_find_raw_event_selector(event_data, selector);
    default:
        break;
    }

    return SBI_ERROR_NOT_SUPPORTED;
}

static int pmu_find_hw_cntr(unsigned long event_idx, unsigned long base, unsigned long mask)
{
    unsigned long ctr_mask;
    unsigned long bit, tmp;

    /* skip fixed counters */
    ctr_mask = (mask << base) & (~SBI_PMU_FIXED_CTR_MASK);

    if (!ctr_mask)
        return SBI_ERROR_INVALID_PARAM;

    if (sbi_fls(ctr_mask) > pmu_hw_ctr_avail)
        return SBI_ERROR_INVALID_PARAM;

    for_each_set_bit(bit, ctr_mask, tmp) {
        if (bit > pmu_hw_ctr_avail || bit == 1)
            return SBI_ERROR_INVALID_PARAM;

        if (active_events[bit] == SBI_PMU_EVENT_NOT_SUPP)
            return bit;
    }

    return SBI_ERROR_FAILURE;
}

static void pmu_set_hw_event(unsigned long cidx, uint64_t event)
{
    if (cidx < 3 || cidx > pmu_hw_ctr_avail)
        return;

#if __riscv_xlen == 32
    csr_write_num(CSR_MHPMEVENT3 + cidx - 3, event & 0xFFFFFFFF);
    csr_write_num(CSR_MHPMEVENT3H + cidx - 3, event >> BITS_PER_LONG);
#else
    csr_write_num(CSR_MHPMEVENT3 + cidx - 3, event);
#endif
}

static void pmu_write_hw_ctr(unsigned long cidx, uint64_t init)
{
    if (cidx == 1 || cidx > pmu_hw_ctr_avail)
        return;

#if __riscv_xlen == 32
    csr_write_num(CSR_MCYCLE + cidx, 0);
    csr_write_num(CSR_MCYCLE + cidx, init & 0xFFFFFFFF);
    csr_write_num(CSR_MCYCLEH + cidx, init >> BITS_PER_LONG);
#else
    csr_write_num(CSR_MCYCLE + cidx, init);
#endif
}

/* */

void pmu_hart_init(void)
{
    int i;

    for (i = 0; i <= PMU_HW_CTR_MAX; i++)
        active_events[i] = SBI_PMU_EVENT_NOT_SUPP;

}

unsigned long sbi_pmu_num_ctr(unsigned long *out)
{
    unsigned long reg, bit, tmp;

    /* MCYCLE/TIME/MINSTRET are always available */
    pmu_hw_ctr_avail = 3;

    /*
     * Function hart_init enables all the PMU counters writing ones to MCOUNTEREN
     * Reading back from MCOUNTEREN returns ones only for available counters.
     */
    reg = read_csr(mcounteren) >> PMU_HW_CTR_START;

    for_each_set_bit(bit, reg, tmp) {
        pmu_hw_ctr_avail += 1;
    }

    *out = pmu_hw_ctr_avail;

    return SBI_ERROR_SUCCESS;
}

unsigned long sbi_pmu_ctr_get_info(unsigned long cidx, unsigned long *out)
{
    union sbi_pmu_ctr_info cinfo = {0};

    if (cidx > pmu_hw_ctr_avail || cidx == 1)
        return SBI_ERROR_INVALID_PARAM;

    cinfo.type = SBI_PMU_CTR_TYPE_HW;
    cinfo.csr = CSR_CYCLE + cidx;
    cinfo.width = 63;

    *out = cinfo.value;

    return SBI_ERROR_SUCCESS;
}

unsigned long sbi_pmu_ctr_cfg_match(unsigned long cidx_base,
        unsigned long cidx_mask, unsigned long flags,
        unsigned long event_idx, unsigned long data1,
        unsigned long data2, unsigned long *out)
{
    int event_type = get_cidx_type(event_idx);
    uint64_t event_data;
    uint64_t selector;
    int hw_cntr;
    int ret;

#if __riscv_xlen == 32
    event_data = ((uint64_t)data2 << 32) | data1;
#else
    event_data = data1;
#endif

    if (event_type == SBI_PMU_EVENT_TYPE_FW)
        return SBI_ERROR_NOT_SUPPORTED;

    ret = pmu_find_event_selector(event_idx, event_data, &selector);
    if (ret < 0)
        return ret;

    if (flags & SBI_PMU_CFG_FLAG_SKIP_MATCH) {
        if (!cidx_mask)
            return SBI_ERROR_INVALID_PARAM;

        hw_cntr = cidx_base + sbi_ffs(cidx_mask);
        if (hw_cntr > pmu_hw_ctr_avail)
            return SBI_ERROR_INVALID_PARAM;
    } else {
        hw_cntr = pmu_find_hw_cntr(event_idx, cidx_base, cidx_mask);
        if (hw_cntr < 0)
            return hw_cntr;
    }

    /* configure event mode filters */
    sbi_pmu_apply_hw_flags(flags, &selector);

    active_events[hw_cntr] = selector;
    /* Always set the OVF bit (disable OVF interrupts). OVF interrupt should be enabled during the start call. */
    pmu_set_hw_event(hw_cntr, selector | MHPMEVENT_OF);
    *out = hw_cntr;

    return SBI_ERROR_SUCCESS;
}


unsigned long sbi_pmu_ctr_start(unsigned long cbase,
        unsigned long cmask, unsigned long flags,
        unsigned long data1, unsigned long data2)
{
    unsigned long ctr_mask = cmask << cbase;
    unsigned long mctr_inhbt, bit, tmp;
    unsigned long mip_val;
    uint64_t selector;
    uint64_t init;

#if __riscv_xlen == 32
    init = ((uint64_t)data2 << 32) | data1;
#else
    init = data1;
#endif

    if (!ctr_mask)
        return SBI_ERROR_INVALID_PARAM;

    if (sbi_fls(ctr_mask) > pmu_hw_ctr_avail)
        return SBI_ERROR_INVALID_PARAM;

    for_each_set_bit(bit, ctr_mask, tmp) {
        if (bit > pmu_hw_ctr_avail || bit == 1)
            return SBI_ERROR_INVALID_PARAM;

        if (flags & SBI_PMU_START_FLAG_SET_INIT_VALUE)
            pmu_write_hw_ctr(bit, init);

        mctr_inhbt = read_csr(CSR_MCOUNTINHIBIT);

        /* check if already started */
        if (!__test_bit(bit, &mctr_inhbt))
            continue;

        __clear_bit(bit, &mctr_inhbt);

        mip_val = read_csr(mip);
        if (!(mip_val & (1 << 13))) {
            selector = active_events[bit];
            pmu_set_hw_event(bit, selector);
        }

        write_csr(CSR_MCOUNTINHIBIT, PMU_DFLT_EN_MINH(mctr_inhbt));
    }

    return SBI_ERROR_SUCCESS;
}

unsigned long sbi_pmu_ctr_stop(unsigned long cbase,
        unsigned long cmask, unsigned long flags)
{
    unsigned long ctr_mask = cmask << cbase;
    unsigned long mctr_inhbt, bit, tmp;

    if (!ctr_mask)
        return SBI_ERROR_INVALID_PARAM;

    if (sbi_fls(ctr_mask) > pmu_hw_ctr_avail)
        return SBI_ERROR_INVALID_PARAM;

    for_each_set_bit(bit, ctr_mask, tmp) {
        if (bit > pmu_hw_ctr_avail || bit == 1)
            return SBI_ERROR_INVALID_PARAM;

        mctr_inhbt = read_csr(CSR_MCOUNTINHIBIT);
        if (!__test_bit(bit, &mctr_inhbt)) {
            __set_bit(bit, &mctr_inhbt);
            write_csr(CSR_MCOUNTINHIBIT, PMU_DFLT_EN_MINH(mctr_inhbt));
        }

        if (flags & SBI_PMU_STOP_FLAG_RESET) {
            active_events[bit] = SBI_PMU_EVENT_NOT_SUPP;
            pmu_set_hw_event(bit, 0);
        }
    }

    return SBI_ERROR_SUCCESS;
}

unsigned long sbi_pmu_fw_ctr_read(unsigned long cidx,
        unsigned long *out)
{
    // TODO

    return SBI_ERROR_NOT_SUPPORTED;
}

#endif // PLF_PMU_SUPPORT
