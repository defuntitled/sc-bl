/*
 * Copyright (C) 2022, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* infra
///
/// @copyright Copyright (C) 2022, Syntacore Ltd. All Rights Reserved.
/// @author dz-sc kn-sc
///
/// @brief L2 PMU (Performance Monitoring Unit)

#ifndef SCR_L2_PMU_H
#define SCR_L2_PMU_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "platform_config.h"
#include "bitops.h"
#include "csr.h"
#include "sbi.h"
#include "sbi_vendor.h"
#include "hls.h"

#if PLF_L2_PMU_SUPPORT

int scr_fdt_l2_pmu_init(void);
unsigned long sbi_l2_pmu_features_flags(void);
unsigned long sbi_l2_pmu_ctr_get_info(unsigned long cidx,
        unsigned long *out);
unsigned long sbi_l2_pmu_num_ctr(unsigned long *out);
unsigned long sbi_l2_pmu_ctr_cfg_match(unsigned long cidx_base,
        unsigned long cidx_mask, unsigned long flags,
        unsigned long event_idx, unsigned long data1,
        unsigned long *out);
unsigned long sbi_l2_pmu_ctr_start(unsigned long cbase,
        unsigned long cmask, unsigned long flags,
        unsigned long data1, unsigned long data2);
unsigned long sbi_l2_pmu_ctr_stop(unsigned long cbase,
        unsigned long cmask, unsigned long flags);
unsigned long sbi_l2_pmu_hw_ctr_read(unsigned long cidx,
        unsigned long *out);
unsigned long sbi_l2_pmu_get_vid(unsigned long cidx,
	unsigned long *out);
#endif //PLF_L2_PMU_SUPPORT

#endif
