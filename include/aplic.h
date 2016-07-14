/*
 * Copyright (C) 2022, Syntacore Ltd.
 * All Rights Reserved.
 */

/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Western Digital Corporation or its affiliates.
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

/// Syntacore SCR* framework
///
/// @copyright Copyright (C) 2022, Syntacore Ltd. All Rights Reserved.
///
/// @brief APLIC defines and inline functions

#ifndef SCR_APLIC_H
#define SCR_APLIC_H

#include "platform_config.h"

#if PLF_AIA_SUPPORT

#define APLIC_MAX_DELEGATE             2

#define APLIC_DOMAINCFG                0x0000
#define APLIC_DOMAINCFG_IE             BIT(8)
#define APLIC_DOMAINCFG_DM             BIT(2)
#define APLIC_DOMAINCFG_BE             BIT(0)

#define APLIC_SOURCECFG_BASE           0x0004
#define APLIC_SOURCECFG_D              BIT(10)
#define APLIC_SOURCECFG_CHILDIDX_MASK  0x000003ff
#define APLIC_SOURCECFG_SM_MASK        0x00000007
#define APLIC_SOURCECFG_SM_INACTIVE    0x0
#define APLIC_SOURCECFG_SM_DETACH      0x1
#define APLIC_SOURCECFG_SM_EDGE_RISE   0x4
#define APLIC_SOURCECFG_SM_EDGE_FALL   0x5
#define APLIC_SOURCECFG_SM_LEVEL_HIGH  0x6
#define APLIC_SOURCECFG_SM_LEVEL_LOW   0x7

#define APLIC_MMSICFGADDR              0x1bc0
#define APLIC_MMSICFGADDRH             0x1bc4
#define APLIC_SMSICFGADDR              0x1bc8
#define APLIC_SMSICFGADDRH             0x1bcc

#define APLIC_SETIP_BASE               0x1c00
#define APLIC_SETIPNUM                 0x1cdc

#define APLIC_CLRIP_BASE               0x1d00
#define APLIC_CLRIPNUM                 0x1ddc

#define APLIC_SETIE_BASE               0x1e00
#define APLIC_SETIENUM                 0x1edc

#define APLIC_CLRIE_BASE               0x1f00
#define APLIC_CLRIENUM                 0x1fdc

#define APLIC_SETIPNUM_LE              0x2000
#define APLIC_SETIPNUM_BE              0x2004

#define APLIC_GENMSI                   0x3000
#define APLIC_GENMSI_BUSY              BIT(12)
#define APLIC_GENMSI_HARTIDX_MASK      0x3fff
#define APLIC_GENMSI_HARTIDX_SHIFT     18
#define APLIC_GENMSI_EIID_MASK         0x7f

#define APLIC_TARGET_BASE              0x3004
#define APLIC_TARGET_HART_IDX_SHIFT    18
#define APLIC_TARGET_HART_IDX_MASK     0x3fff
#define APLIC_TARGET_GUEST_IDX_SHIFT   12
#define APLIC_TARGET_GUEST_IDX_MASK    0x3f
#define APLIC_TARGET_IPRIO_MASK        0xff
#define APLIC_TARGET_EIID_MASK         0x7ff

#define APLIC_MAX_SOURCE               1024

#define APLIC_DEFAULT_PRIORITY         1
#define APLIC_DISABLE_IDELIVERY        0
#define APLIC_ENABLE_IDELIVERY         1
#define APLIC_DISABLE_ITHRESHOLD       1
#define APLIC_ENABLE_ITHRESHOLD        0

void aplic_init(void);

#else

static inline void aplic_init(void)
{
}

#endif // PLF_AIA_SUPPORT

#endif // SCR_APLIC_H
