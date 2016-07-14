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


#ifndef SCR_CLK_H
#define SCR_CLK_H

#include <stdint.h>
#include <limits.h>

#include "arch.h"

#define SCR_CLK_INVALID_VAL ULONG_MAX

enum {
    SCR_CLK_OUT_OF_RANGE = -1,
    SCR_CLK_UNIFORM_HARTS = -2,
    SCR_CLK_BAD_HARTID = -3,
    SCR_CLK_SUCCESS = 0,
};

enum {
    SCR_CLK_MTIMER_INTERNAL = 0,
    SCR_CLK_MTIMER_EXTERNAL = 1
};

struct clk_mtimer_response {
    unsigned long clk;
    int clock_source;
};

void clk_header_init();
unsigned long clk_buildid_get();
unsigned long clk_sys_get();
unsigned long clk_cluster_get();
struct clk_mtimer_response clk_mtimer_get();
unsigned long clk_uart_get();
unsigned long clk_uartaddr_get();
unsigned long clk_numharts_get();
int clk_hart_get(unsigned long idx, unsigned long* hartid, unsigned long* clk);
int clk_hart_get_by_id(unsigned long hartid, unsigned long* clk);

static inline unsigned long clk_masterhart_get() {
    unsigned long clk;
    // clk_hart_get_by_id can never fail when looking for
    // master hart frequency because the master hart has
    // started us. We never reach this code if it hartid was
    // invalid
    (void)clk_hart_get_by_id(PLF_MASTER_HART, &clk);
    return clk;
}


#endif // SCR_CLK_H
