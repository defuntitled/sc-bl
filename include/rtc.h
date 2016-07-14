/*
 * Copyright (C) 2015, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* infra
///
/// @copyright Copyright (C) 2015-2022, Syntacore Ltd. All Rights Reserved.
/// @author mn-sc
///
/// @brief RTC defs and inline funcs

#ifndef SCR_RTC_H
#define SCR_RTC_H

#include "platform_config.h"

#ifdef PLF_MTIMER_BASE

#define SCR_RTC_CTL_OFF   0
#define SCR_RTC_DIV_OFF   4
#define SCR_RTC_TIME_OFF  8
#define SCR_RTC_TIMEH_OFF 12
#define SCR_RTC_CMP_OFF   16
#define SCR_RTC_CMPH_OFF  20

#define SCR_RTC_CTL       (PLF_MTIMER_BASE+SCR_RTC_CTL_OFF)
#define SCR_RTC_DIVIDER   (PLF_MTIMER_BASE+SCR_RTC_DIV_OFF)
#define SCR_RTC_MTIME     (PLF_MTIMER_BASE+SCR_RTC_TIME_OFF)
#define SCR_RTC_MTIMEH    (PLF_MTIMER_BASE+SCR_RTC_TIMEH_OFF)
#define SCR_RTC_MTIMECMP  (PLF_MTIMER_BASE+SCR_RTC_CMP_OFF)
#define SCR_RTC_MTIMECMPH (PLF_MTIMER_BASE+SCR_RTC_CMPH_OFF)

#ifdef PLF_MTIMER_EXT
#define SCR_RTC_MTIMECMP_OFFS         (0x1000)
#define SCR_RTC_MTIMECMP_CORE(x)      (PLF_MTIMER_BASE + SCR_RTC_MTIMECMP_OFFS + ((x) << 3))
#define SCR_RTC_MTIMECMPH_CORE(x)     (PLF_MTIMER_BASE + SCR_RTC_MTIMECMP_OFFS + ((x) << 3) + 4)
#endif //PLF_MTIMER_EXT

#define SCR_RTC_CTL_EN (1 << 0)
#define SCR_RTC_CTL_INTERNAL_SRC (0 << 1)
#define SCR_RTC_CTL_EXTERNAL_SRC (1 << 1)

#else // PLF_MTIMER_BASE

#ifdef PLF_RTC_TIMEBASE
#undef PLF_RTC_TIMEBASE
#endif
#define PLF_RTC_TIMEBASE plf_cpu_clk

#endif // PLF_MTIMER_BASE

#if (PLF_RTC_TIMEBASE) == 1000000
#define PLF_RTC_TIMEBASE_1M 1
#endif

#ifndef __ASSEMBLER__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "csr.h"
#include "clk.h"

typedef unsigned long sys_tick_t;

static inline sys_tick_t now(void)
{
#ifdef PLF_MTIMER_BASE
    return rdtime();
#else
    return rdcycle();
#endif
}

static inline long ticks2ms(sys_tick_t t)
{
    return t / (PLF_RTC_TIMEBASE / 1000);
}

static inline sys_tick_t ms2ticks(long t)
{
    return t * PLF_RTC_TIMEBASE / 1000;
}

static inline sys_tick_t us2ticks(unsigned long us)
{
#ifdef PLF_RTC_TIMEBASE_1M
    return us; // tick == 1us
#else
    return us * (PLF_RTC_TIMEBASE / 976) / 1024;
#endif
}

static inline void rtc_delay_us(unsigned long us)
{
    sys_tick_t t = now();
    sys_tick_t ticks = us2ticks(us);
    do ; while ((now() - t) < ticks);
}

static inline int scr_time2str(char *buf, unsigned len, sys_tick_t t)
{
    int ssz = 0;

    unsigned long s = t / PLF_RTC_TIMEBASE;
    unsigned long m = s / 60;
    unsigned long h = m / 60;
    unsigned long d = h / 24;

    h %= 24;
    m %= 60;
    s %= 60;

    if (d) {
        ssz = snprintf(buf, len, "%lu days, ", d);
    }
    if (ssz < len) {
        ssz += snprintf(buf + ssz, len - ssz, "%lu:%02lu:%02lu", h, m, s);
    }

    return ssz;
}

#ifdef PLF_MTIMER_BASE
static inline void scr_rtc_setcmp(uint64_t when)
{
#if __riscv_xlen == 32
    *(volatile uint32_t*)SCR_RTC_MTIMECMPH = UINT32_MAX;
    *(volatile uint32_t*)SCR_RTC_MTIMECMP = (uint32_t)when;
    *(volatile uint32_t*)SCR_RTC_MTIMECMPH = (uint32_t)(when >> 32);
#else //  __riscv_xlen == 32
    *(volatile uint64_t*)SCR_RTC_MTIMECMP = when;
#endif //  __riscv_xlen == 32
}

static inline void scr_rtc_init(void)
{
    // configure RTC timebase (divisor) and reset time counter, alarm time
    *(volatile uint32_t*)SCR_RTC_CTL = 0;

#if __riscv_xlen == 32
    *(volatile uint32_t*)SCR_RTC_MTIME = 0;
    *(volatile uint32_t*)SCR_RTC_MTIMEH = 0;
#else //  __riscv_xlen == 32
    *(volatile uint64_t*)SCR_RTC_MTIME = 0;
#endif //  __riscv_xlen == 32

#if __riscv_xlen == 32
    *(volatile uint32_t*)SCR_RTC_MTIMECMPH = UINT32_MAX;
    *(volatile uint32_t*)SCR_RTC_MTIMECMP = UINT32_MAX;
#else //  __riscv_xlen == 32
    *(volatile uint64_t*)SCR_RTC_MTIMECMP = UINT64_MAX;
#endif //  __riscv_xlen == 32

    struct clk_mtimer_response mtimer_response = clk_mtimer_get();
    *(volatile uint32_t*)SCR_RTC_DIVIDER = (mtimer_response.clk / PLF_RTC_TIMEBASE) - 1;
    if (mtimer_response.clock_source == SCR_CLK_MTIMER_EXTERNAL) {
        *(volatile uint32_t*)SCR_RTC_CTL = SCR_RTC_CTL_EN | SCR_RTC_CTL_EXTERNAL_SRC;
    } else {
        *(volatile uint32_t*)SCR_RTC_CTL = SCR_RTC_CTL_EN | SCR_RTC_CTL_INTERNAL_SRC;
    }
}

#else // PLF_MTIMER_BASE

#define scr_rtc_setcmp(when) do {} while (0)

#define scr_rtc_init() do {} while (0)

#endif // PLF_MTIMER_BASE

#endif // !__ASSEMBLER__
#endif // SCR_RTC_H
