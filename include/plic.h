/*
 * Copyright (C) 2019, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* framework
///
/// @copyright Copyright (C) 2019, Syntacore Ltd. All Rights Reserved.
/// @author mn-sc
///
/// @brief PLIC defs and inline funcs

#ifndef SCR_PLIC_H
#define SCR_PLIC_H

#include <stdbool.h>
#include <stdint.h>

#include "platform_config.h"
#include "bitops.h"
#include "utils.h"

#ifdef PLF_PLIC_BASE
#define PLF_SCR_PLIC_MODE_BASE (PLF_PLIC_BASE + 0x1f0000)

#define PLF_SCR_PLIC_MODES ((volatile uint32_t*)PLF_SCR_PLIC_MODE_BASE)

#define SCR_PLIC_SRC_MODE_OFF          0
#define SCR_PLIC_SRC_MODE_LEVEL_HIGH   1
#define SCR_PLIC_SRC_MODE_LEVEL_LOW    2
#define SCR_PLIC_SRC_MODE_EDGE_RISING  3
#define SCR_PLIC_SRC_MODE_EDGE_FALLING 4
#define SCR_PLIC_SRC_MODE_EDGE_BOTH    5
#define SCR_PLIC_SRC_MODE_MAX          SCR_PLIC_SRC_MODE_EDGE_BOTH

#define PLF_PLIC_IRQ_NUM               1024

#ifdef PLF_CLUSTER_CFG_BASE
#define SCR_PLIC_MSI_OFFSET            (PLF_PLIC_BASE + 0x1000000UL)
#define SCR_PLIC_MSI_COMPAT_INFO       (SCR_PLIC_MSI_OFFSET + 4)
#define SCR_PLIC_MSI_COMPAT_INFO_S     0
#define SCR_PLIC_MSI_COMPAT_INFO_M     GENMASK(3, 0)
#define SCR_PLIC_MSI_SOURCES_S         4
#define SCR_PLIC_MSI_SOURCES_M         GENMASK(31, 4)

/* PLIC offsets that differ in different PLIC MSI versions */
struct plic_info {
    uint32_t en_base;
    uint32_t en_size;
};

static inline void plic_msi_get_info(struct plic_info *info)
{
    uint32_t msi_compat = readl(SCR_PLIC_MSI_COMPAT_INFO);
    int plic_msi_version = (msi_compat >> SCR_PLIC_MSI_COMPAT_INFO_S) & SCR_PLIC_MSI_COMPAT_INFO_M;

    if (plic_msi_version == 1) {
        info->en_base = 0x4000;
        info->en_size = 0x100;
    } else {
        info->en_base = 0x2000;
        info->en_size = 0x80;
    }
}
#endif // PLF_CLUSTER_CFG_BASE

void plic_init(bool dbg_print);

#endif // PLF_PLIC_BASE
#endif // SCR_PLIC_H
