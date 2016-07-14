/*
 * Copyright (C) 2023, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* framework
///
/// @copyright Copyright (C) 2023, Syntacore Ltd. All rights reserved.
/// @author gdi-sc
///
/// @brief Cluster control funcs

#include <stdint.h>
#include <stddef.h>
#include "cache.h"
#include "cluster_control.h"
#include "iccm.h"

#ifdef PLF_CLUSTER_CFG_BASE

#define SCR_JTAGIDCODE 1

static void plf_cluster_set_jtagid(void)
{
    unsigned long slots_num;
#if PLF_SMP_SUPPORT && PLF_SMP_ICCM_SUPPORT
    slots_num = ipi_get_num_slots();
#else
    slots_num = 1;
#endif

    for (uint8_t cpu_num = 0; cpu_num < slots_num; ++cpu_num) {
        /// Set Jtag IDs
        *(volatile uint64_t*)(CL_CPU_N_JTAGIDCODE(cpu_num)) = SCR_JTAGIDCODE;
    }
}

static void plf_ext_cpu_start(void)
{
#if PLF_SMP_SUPPORT && PLF_SMP_ICCM_SUPPORT && PLF_CLUSTER_RESET_CORES
    unsigned long slots_num = ipi_get_num_slots();
    for (uint8_t cpu_num = 1; cpu_num < slots_num; ++cpu_num) {
        /// Set Hart IDs
        *(volatile uint64_t*)(CL_CPU_N_MHARTID(cpu_num)) = cpu_num;
    }

    /// Turn on CPU
    for (uint8_t cpu_num = 1; cpu_num < slots_num; ++cpu_num) {
        *(volatile uint64_t*)(CL_CPU_N_CTRL(cpu_num)) = 0 \
                                                        | CL_CPU_CTRL_RESET_N_BIT \
                                                        | CL_CPU_CTRL_LINKUP_BIT;
    }
#endif
}

void plf_cluster_cpu_init(void)
{
    plf_cluster_set_jtagid();

    plf_ext_cpu_start();
}

/**
 * \brief CPU self side steps for deactivation
 *
 * \param None
 * \return None
 */
void plf_local_cpu_deactivation_prepare(void)
{
    // Actions according L3 Cluster External Architecture Specification
    // Mask all interrupts
    write_csr(mstatus, 0x0);
    write_csr(mie, 0x0);
    // Disable L1 caches
    cache_l1_disable();
    // Disable L2 cache
    scr_l2cache_disable();
    // Jump to WFI
    wfi();
}

/**
 * \brief Actions from other CPU to deactivate target CPU
 *
 * \param cpu_id number of CPU to deactivate
 * \return None
 */
void plf_ext_cpu_deactivate(uint32_t cpu_id)
{
    // Check: can't deactivate self
    if (cpu_id == read_csr(mhartid)) {
        return;
    }
    // Actions according L3 Cluster External Architecture Specification
    // Read spin until CPU WFI state
    while (!(*(volatile uint64_t*)CL_CPU_N_CTRL(cpu_id) & CL_CPU_CTRL_WFI_BIT));
    // Read spin until the end of CPU's side activity
    while (*(volatile uint64_t*)CL_CPU_N_CTRL(cpu_id) & CL_CPU_CTRL_ACT_BIT);
    // Turn off CHI link
    *(volatile uint64_t*)CL_CPU_N_CTRL(cpu_id) &= ~CL_CPU_CTRL_LINKUP_BIT;
    while (*(volatile uint64_t*)CL_CPU_N_CTRL(cpu_id) & CL_CPU_CTRL_LINK_STS_MASK);
    // Assert CPU reset
    *(volatile uint64_t*)CL_CPU_N_CTRL(cpu_id) &= ~CL_CPU_CTRL_RESET_N_BIT;
    // Bypass CPU-TAP controller, turn off power is not implemented here
}

/**
 * \brief Actions from other CPU to activate target CPU
 *
 * \param cpu_id number of CPU to activate
 * \return None
 */
void plf_ext_cpu_activate(uint32_t cpu_id)
{
    extern void _start(void);
    // Actions according L3 Cluster External Architecture Specification
    // First, power must be turned on and TAP Controller must be reset
    // Deassert CPU reset, Turn ON CHI Link
    *(volatile uint64_t*)CL_CPU_N_RESETPC (cpu_id) = (uint64_t)&_start;
    *(volatile uint64_t*)CL_CPU_N_CTRL(cpu_id) |= (CL_CPU_CTRL_RESET_N_BIT | CL_CPU_CTRL_LINKUP_BIT);
    while ((*(volatile uint64_t*)CL_CPU_N_CTRL(cpu_id) & CL_CPU_CTRL_LINK_STS_MASK) != CL_CPU_CTRL_LINK_STS_MASK);
}

#endif // PLF_CLUSTER_CFG_BASE
