/*
 * Copyright (C) 2023, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* infra
///
/// @copyright Copyright (C) 2023, Syntacore Ltd. All Rights Reserved.
/// @author sm-sc
///
/// @brief SBI vendor specific API constants

#ifndef SCR_VENDOR_RISCV_SBI_H
#define SCR_VENDOR_RISCV_SBI_H

// Syntacore vendor specific extension space
#define SBI_EXT_SCR           0x090006bb

// Syntacore vendor specific functional blocks
#define SBI_SCR7_L2_PMU_FN    0x1
#define SBI_SCR7_L3_PMU_FN    0x2
#define SBI_SCR_VCS_FN        0x3

#define SBI_SCR_PLFINFO	      0x101

#define SBI_EXT_SCR_PMU_COUNTER_HW_READ  0x6
#define SBI_EXT_SCR_PMU_PROBE            0x7
#define SBI_EXT_SCR_PMU_VID              0x8

#define SBI_EXT_SCR_VCS_PROBE  0x1
#define SBI_EXT_SCR_VCS_WRITE  0x2
#define SBI_EXT_SCR_VCS_READ   0x3

#define SBI_EXT_SCR_PLF_GET_FEAT_EN                 0
#define SBI_EXT_SCR_PLF_GET_VCR_SCHEMA              1
#define SBI_EXT_SCR_PLF_GET_CORE_TIMESTAMP_ID       2
#define SBI_EXT_SCR_PLF_GET_CORE_BUILD_TARGET_ID    3
#define SBI_EXT_SCR_PLF_GET_CORE_CFG_ID             4
#define SBI_EXT_SCR_PLF_GET_CL_VID                  5
#define SBI_EXT_SCR_PLF_GET_L1D_PF_CTRL0            6

#endif /* SCR_VENDOR_RISCV_SBI_H */
