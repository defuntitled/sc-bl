/*
 * Copyright (C) 2024, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* framework
///
/// @copyright Copyright (C) 2024, Syntacore Ltd. All Rights Reserved.
/// @author i.mamay
///
/// @brief Runtime clocks frequency reading routines

#include <stdint.h>

#include "platform_config.h"
#include "reg_detect.h"
#include "clk.h"

#define SCR_CLK_DATA_ADDR PLF_OCRAM_BASE

#define SCR_CLK_MAGIC_V1 0xe1c5da10

struct clk_header_v1 {
    unsigned long magic;
    unsigned long build_id;
    unsigned long axi_bus_clk;
    unsigned long l3_cluster_clk;
    unsigned long mtimer_ext_clk;
    unsigned long uart_clk;
    unsigned long uart_addr;
    unsigned long num_harts;
    struct hart_clk_data {
        unsigned long hartid;
        unsigned long clk;
    } hart_clk[];
};

struct clk_header {
    union {
        unsigned long magic;
        struct clk_header_v1 v1;
    };
};

struct clk_header_callbacks {
    unsigned long (*get_build_id)();
    unsigned long (*get_axi_bus_clk)();
    unsigned long (*get_l3_cluster_clk)();
    struct clk_mtimer_response (*get_mtimer_clk)();
    unsigned long (*get_uart_clk)();
    unsigned long (*get_uart_addr)();
    unsigned long (*get_num_harts)();
    int (*get_hart)(unsigned long idx, unsigned long* hartid, unsigned long* clk);
} clk_callbacks;

static inline struct clk_header const* clk_header_get() {
    return (struct clk_header const*)SCR_CLK_DATA_ADDR;
}

static inline unsigned long clk_magic_get() {
    return clk_header_get()->magic;
}

static unsigned long clk_buildid_get_fallback() {
    // Wow, there is a HAL call to get build id, but all other legacy clk getters
    // implemented directly in sc-bl.....
    return get_build_id();
}
static unsigned long clk_sys_get_fallback() {
#ifdef PLF_SYSCLK_MHZ_ADDR
    return (*(uint32_t*)(PLF_SYSCLK_MHZ_ADDR)) * 1000000;
#elif defined PLF_SYS_FREQ
    return PLF_CPU_FREQ;
#else
    hart_halt();
#endif
}
static unsigned long clk_cluster_get_fallback() {
#ifdef PLF_CLSCLK_MHZ_ADDR
    if(mmio_read_allowed(PLF_CLSCLK_MHZ_ADDR, NULL)) {
        /* internal mtimer clock: CLUSTER_freq */
        return (*(uint32_t*)(PLF_CLSCLK_MHZ_ADDR)) * 1000000;
    } else
#endif
    {
        return clk_sys_get_fallback();
    }
}
static struct clk_mtimer_response clk_mtimer_get_fallback() {
    struct clk_mtimer_response response;
#if PLF_RTC_SRC_EXTERNAL
    // in IPDev L3 bitstreams external mtimer clock is AXI_freq / 4
    response.clk = clk_sys_get_fallback() / 4;
    response.clock_source = SCR_CLK_MTIMER_EXTERNAL;
    return response;
#else
    response.clk = clk_cluster_get_fallback();
    response.clock_source = SCR_CLK_MTIMER_INTERNAL;
    return response;
#endif

#if defined(PLF_RTC_FREQ)
#if (PLF_RTC_TIMEBASE) > (PLF_RTC_FREQ)
#error PLF_RTC_TIMEBASE > PLF_RTC_FREQ
#endif
    response.clk = PLF_RTC_FREQ;
    response.clock_source = SCR_CLK_MTIMER_INTERNAL;
    return response;
#endif
}
static unsigned long clk_uart_get_fallback() {
#ifdef PLF_UART_CLK
    return PLF_UART_CLK;
#elif defined PLF_SYS_FREQ
    return PLF_SYS_FREQ;
#else
    // hart_halt();
    return 0; // there are platforms like qemu, that does not require UART clk
#endif
}
static unsigned long clk_uartaddr_get_fallback() {
    return SCR_CLK_INVALID_VAL;
}
static unsigned long clk_numharts_get_fallback() {
    return SCR_CLK_INVALID_VAL;
}
static int clk_hart_get_fallback(unsigned long idx, unsigned long* hartid, unsigned long* clk) {
#ifdef PLF_CPUCLK_MHZ_ADDR
        *clk = *(uint32_t*)(PLF_CPUCLK_MHZ_ADDR) * 1000000;
#elif defined PLF_CPU_FREQ
        *clk = PLF_CPU_FREQ;
#else
    hart_halt();
#endif
    *hartid = SCR_CLK_INVALID_VAL;
    return SCR_CLK_UNIFORM_HARTS;
}

static unsigned long clk_buildid_get_v1() {
    return clk_header_get()->v1.build_id;
}
static unsigned long clk_sys_get_v1() {
    return clk_header_get()->v1.axi_bus_clk;
}
static unsigned long clk_cluster_get_v1() {
    return clk_header_get()->v1.l3_cluster_clk;
}
static struct clk_mtimer_response clk_mtimer_get_v1() {
    struct clk_mtimer_response response;

#if defined PLF_RTC_SRC_EXTERNAL
    unsigned long mtimer_blob_clk = clk_header_get()->v1.mtimer_ext_clk;
    if(mtimer_blob_clk) {
        response.clk = mtimer_blob_clk;
        response.clock_source = SCR_CLK_MTIMER_EXTERNAL;
        return response;
    }
    // external clock source not present, falling back to internal clocking
#endif

    response.clk = clk_cluster_get_v1();
    response.clock_source = SCR_CLK_MTIMER_INTERNAL;
    return response;
}
static unsigned long clk_uart_get_v1() {
    unsigned long clk = clk_header_get()->v1.uart_clk;
    return clk == 0 ? clk_uart_get_fallback() : clk;
}
static unsigned long clk_uartaddr_get_v1() {
    unsigned long addr =  clk_header_get()->v1.uart_addr;
    return addr == ULONG_MAX ? clk_uartaddr_get_fallback() : addr;
}
static unsigned long clk_numharts_get_v1() {
    return clk_header_get()->v1.num_harts;
}
static int clk_hart_get_v1(unsigned long idx, unsigned long* hartid, unsigned long* clk) {
    if(idx >= clk_header_get()->v1.num_harts) {
        return SCR_CLK_OUT_OF_RANGE;
    }

    *hartid = clk_header_get()->v1.hart_clk[idx].hartid;
    *clk = clk_header_get()->v1.hart_clk[idx].clk;

    return SCR_CLK_SUCCESS;
}
void clk_header_init() {
    // magic value determines version of clk_header struct, so
    // each new version will incease variablility of clocks 
    // computation methods. I think single structure with callbacks 
    // (a.k.a pure virtual class / interface) will keep code 
    // readable. Otherwise we will get switch in each getter method,
    // that will lead to copy-paste errors and hard maintance
    
    clk_callbacks.get_build_id = clk_buildid_get_v1;
    clk_callbacks.get_axi_bus_clk = clk_sys_get_v1;
    clk_callbacks.get_l3_cluster_clk = clk_cluster_get_v1;
    clk_callbacks.get_mtimer_clk = clk_mtimer_get_v1;
    clk_callbacks.get_uart_clk = clk_uart_get_v1;
    clk_callbacks.get_num_harts = clk_numharts_get_v1;
    clk_callbacks.get_uart_addr = clk_uartaddr_get_v1;
    clk_callbacks.get_hart = clk_hart_get_v1;

    switch(clk_magic_get()) {
    case SCR_CLK_MAGIC_V1:
        // it would reduce code size in future...
        break;

    default:
        clk_callbacks.get_build_id = clk_buildid_get_fallback;
        clk_callbacks.get_axi_bus_clk = clk_sys_get_fallback;
        clk_callbacks.get_l3_cluster_clk = clk_cluster_get_fallback;
        clk_callbacks.get_mtimer_clk = clk_mtimer_get_fallback;
        clk_callbacks.get_uart_clk = clk_uart_get_fallback;
        clk_callbacks.get_num_harts = clk_numharts_get_fallback;
        clk_callbacks.get_uart_addr = clk_uartaddr_get_fallback;
        clk_callbacks.get_hart= clk_hart_get_fallback;
        break;
    }
}

unsigned long clk_buildid_get() {
    return clk_callbacks.get_build_id();
}
unsigned long clk_sys_get() {
    return clk_callbacks.get_axi_bus_clk();
}
unsigned long clk_cluster_get() {
    return clk_callbacks.get_l3_cluster_clk();
}
struct clk_mtimer_response clk_mtimer_get() {
    return clk_callbacks.get_mtimer_clk();
}
unsigned long clk_uart_get() {
    return clk_callbacks.get_uart_clk();
}
unsigned long clk_uartaddr_get() {
    return clk_callbacks.get_uart_addr();
}
unsigned long clk_numharts_get() {
    return clk_callbacks.get_num_harts();
}
int clk_hart_get(unsigned long idx, unsigned long* hartid, unsigned long* clk) {
    return clk_callbacks.get_hart(idx, hartid, clk);
}

int clk_hart_get_by_id(unsigned long hartid, unsigned long* clk) {
    for(unsigned long i = 0; i < clk_numharts_get(); ++i) {
        unsigned long test_hartid, test_clk;
        int ret = clk_hart_get(i, &test_hartid, &test_clk);
        switch(ret) {
        case SCR_CLK_UNIFORM_HARTS:
            *clk = test_clk;
            return SCR_CLK_SUCCESS;
            break;

        case SCR_CLK_SUCCESS:
            if (hartid == test_hartid) {
                *clk = test_clk;
                return SCR_CLK_SUCCESS;
            }
            break;

        default:
            *clk = SCR_CLK_INVALID_VAL;
            return ret;
            break;
        }
    }

    *clk = SCR_CLK_INVALID_VAL;
    return SCR_CLK_BAD_HARTID;
}
