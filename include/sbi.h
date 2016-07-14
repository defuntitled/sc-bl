/*
 * Copyright (C) 2015, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* infra
///
/// @copyright Copyright (C) 2015-2020, Syntacore Ltd. All Rights Reserved.
/// @author mn-sc
///
/// @brief SBI API constants

// RISC-V SBI: https://github.com/riscv/riscv-sbi-doc/blob/master/riscv-sbi.adoc

#ifndef SCR_INFRA_RISCV_SBI_H
#define SCR_INFRA_RISCV_SBI_H

// SBI v0.1
#define SBI_SET_TIMER              0
#define SBI_CONSOLE_PUTCHAR        1
#define SBI_CONSOLE_GETCHAR        2
#define SBI_CLEAR_IPI              3
#define SBI_SEND_IPI               4
#define SBI_REMOTE_FENCE_I         5
#define SBI_REMOTE_SFENCE_VMA      6
#define SBI_REMOTE_SFENCE_VMA_ASID 7
#define SBI_SHUTDOWN               8

// SBI v0.2
#define SBI_EXT_BASE   0x10
#define SBI_EXT_TIME   0x54494D45
#define SBI_EXT_IPI    0x735049
#define SBI_EXT_RFENCE 0x52464E43
#define SBI_EXT_HSM    0x48534D

/* range for vendor extensions */
#define SBI_EXT_VENDOR_START  0x09000000
#define SBI_EXT_VENDOR_END    0x09FFFFFF

// SBI v0.3
#define SBI_EXT_PMU    0x504D55

// SBI_EXT_BASE
#define SBI_EXT_BASE_FN_GET_SPEC_VERSION 0
#define SBI_EXT_BASE_FN_GET_IMP_ID       1
#define SBI_EXT_BASE_FN_GET_IMP_VERSION  2
#define SBI_EXT_BASE_FN_PROBE_EXT        3
#define SBI_EXT_BASE_FN_GET_MVENDORID    4
#define SBI_EXT_BASE_FN_GET_MARCHID      5
#define SBI_EXT_BASE_FN_GET_MIMPID       6

// SBI_EXT_TIME
#define SBI_EXT_TIME_FN_SET 0

// SBI_EXT_IPI
#define SBI_EXT_IPI_FN_SEND 0

// SBI_EXT_RFENCE
#define SBI_EXT_RFENCE_FN_FENCE_I          0
#define SBI_EXT_RFENCE_FN_SFENCE_VMA       1
#define SBI_EXT_RFENCE_FN_SFENCE_VMA_ASID  2
#define SBI_EXT_RFENCE_FN_HFENCE_GVMA      3
#define SBI_EXT_RFENCE_FN_HFENCE_GVMA_VMID 4
#define SBI_EXT_RFENCE_FN_HFENCE_VVMA      5
#define SBI_EXT_RFENCE_FN_HFENCE_VVMA_ASID 6

// SBI_EXT_HSM
#define SBI_EXT_HSM_FN_HART_START  0
#define SBI_EXT_HSM_FN_HART_STOP   1
#define SBI_EXT_HSM_FN_HART_STATUS 2

// SBI return error codes
#define SBI_ERROR_SUCCESS            0
#define SBI_ERROR_FAILURE           -1
#define SBI_ERROR_NOT_SUPPORTED     -2
#define SBI_ERROR_INVALID_PARAM     -3
#define SBI_ERROR_DENIED            -4
#define SBI_ERROR_INVALID_ADDRESS   -5
#define SBI_ERROR_ALREADY_AVAILABLE -6
#define SBI_ERROR_ALREADY_STARTED   -7
#define SBI_ERROR_ALREADY_STOPPED   -8

// SBI HSM return states
#define SBI_HSM_STATE_START             0
#define SBI_HSM_STATE_STOP              1
#define SBI_HSM_STATE_START_PENDING     2
#define SBI_HSM_STATE_STOP_PENDING      3

// Syntacore SBI ID
#define SCR_SBI_SPEC    3
#define SCR_SBI_IMP_ID  8
#define SCR_SBI_IMP_VER 3

// SBI_EXT_PMU
#define SBI_EXT_PMU_NUM_COUNTERS        0
#define SBI_EXT_PMU_COUNTER_GET_INFO    1
#define SBI_EXT_PMU_COUNTER_CFG_MATCH   2
#define SBI_EXT_PMU_COUNTER_START       3
#define SBI_EXT_PMU_COUNTER_STOP        4
#define SBI_EXT_PMU_COUNTER_FW_READ     5

/* SBI PMU counter type */
#define SBI_PMU_CTR_TYPE_HW 0
#define SBI_PMU_CTR_TYPE_FW 1

/* SBI PMU event idx type */
#define SBI_PMU_EVENT_TYPE_HW       0x0
#define SBI_PMU_EVENT_TYPE_HW_CACHE 0x1
#define SBI_PMU_EVENT_TYPE_HW_RAW   0x2
#define SBI_PMU_EVENT_TYPE_FW       0xf

/* Flags defined for config matching function */
#define SBI_PMU_CFG_FLAG_SKIP_MATCH     (1 << 0)
#define SBI_PMU_CFG_FLAG_CLEAR_VALUE    (1 << 1)
#define SBI_PMU_CFG_FLAG_AUTO_START     (1 << 2)
#define SBI_PMU_CFG_FLAG_SET_VUINH      (1 << 3)
#define SBI_PMU_CFG_FLAG_SET_VSINH      (1 << 4)
#define SBI_PMU_CFG_FLAG_SET_UINH       (1 << 5)
#define SBI_PMU_CFG_FLAG_SET_SINH       (1 << 6)
#define SBI_PMU_CFG_FLAG_SET_MINH       (1 << 7)

/* Flags defined for counter start function */
#define SBI_PMU_START_FLAG_SET_INIT_VALUE   (1 << 0)

/* Flags defined for counter stop function */
#define SBI_PMU_STOP_FLAG_RESET         (1 << 0)

/* Event helpers */
#define SBI_PMU_EVENT_IDX_INVALID   0xFFFFFFFF
#define SBI_PMU_EVENT_IDX_TYPE_MASK 0xF0000
#define SBI_PMU_EVENT_IDX_CODE_MASK 0xFFFF
#define SBI_PMU_EVENT_RAW_IDX       0x20000
#define SBI_PMU_EVENT_NOT_SUPP      (~0ULL)

#define get_cidx_type(x) ((x & SBI_PMU_EVENT_IDX_TYPE_MASK) >> 16)
#define get_cidx_code(x) (x & SBI_PMU_EVENT_IDX_CODE_MASK)

/* fixed events mask */
#define SBI_PMU_FIXED_CTR_MASK 0x07

/* General PMU event codes specified in SBI PMU extension */
#define SBI_PMU_HW_NO_EVENT                     0
#define SBI_PMU_HW_CPU_CYCLES                   1
#define SBI_PMU_HW_INSTRUCTIONS                 2
#define SBI_PMU_HW_CACHE_REFERENCES             3
#define SBI_PMU_HW_CACHE_MISSES                 4
#define SBI_PMU_HW_BRANCH_INSTRUCTIONS          5
#define SBI_PMU_HW_BRANCH_MISSES                6
#define SBI_PMU_HW_BUS_CYCLES                   7
#define SBI_PMU_HW_STALLED_CYCLES_FRONTEND      8
#define SBI_PMU_HW_STALLED_CYCLES_BACKEND       9
#define SBI_PMU_HW_REF_CPU_CYCLES               10

#endif /* SCR_INFRA_RISCV_SBI_H */
