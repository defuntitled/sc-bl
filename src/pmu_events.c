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
/// @brief PMU HW events

#include "pmu.h"

#if PLF_PMU_SUPPORT

#if defined(PLF_CORE_VARIANT_SCR7) || defined(PLF_CORE_VARIANT_SCR9)

#define SCR9_SCR7_SUPPORT        (SCR9_SUPPORT | SCR7_SUPPORT)

const struct sbi_pmu_hw_event hw_sbi_events[] = {
    /* hardware general events */
    { .hw_gen_event = { 0x00001, 0x0000001 } }, /* SBI_PMU_HW_CPU_CYCLES -> GEN_CYC */
    { .hw_gen_event = { 0x00002, 0x0000021 } }, /* SBI_PMU_HW_INSTRUCTIONS -> GEN_INST */
    { .hw_gen_event = { 0x00003, 0x0000005 } }, /* SBI_PMU_HW_CACHE_REFERENCES -> L1D_HIT */
    { .hw_gen_event = { 0x00004, 0x0000015 } }, /* SBI_PMU_HW_CACHE_MISSES -> L1D_MISS */
    { .hw_gen_event = { 0x00005, 0x0000012 } }, /* SBI_PMU_HW_BRANCH_INSTRUCTIONS -> PRD_BRANCH */
    { .hw_gen_event = { 0x00006, 0x0000092 } }, /* SBI_PMU_HW_BRANCH_MISSES -> PRD_BR_MIS */
    { .hw_gen_event = { 0x00008, 0x0000083 } }, /* SBI_PMU_HW_STALLED_CYCLES_FRONTEND -> EXC_BE_IDLE */
    { .hw_gen_event = { 0x00009, 0x00000e3 } }, /* SBI_PMU_HW_STALLED_CYCLES_BACKEND -> EXC_ST_ISS */

    /* hardware cache events */
    { .hw_gen_event = { 0x10000, 0x00000d5 } }, /* L1D_READ_ACCESS -> L1D_LD */
    { .hw_gen_event = { 0x10001, 0x0000015 } }, /* L1D_READ_MISS -> L1D_MISS */
    { .hw_gen_event = { 0x10004, 0x0000019 } }, /* L1D_PREFETCH_ACCESS -> L1D_PF_REQ */
    { .hw_gen_event = { 0x10005, 0x0000055 } }, /* L1D_PREFETCH_MISS -> L1D_PREF_MISS */
    { .hw_gen_event = { 0x10009, 0x0000014 } }, /* L1I_READ_MISS -> L1I_MISS */
    { .hw_gen_event = { 0x1000d, 0x0000044 } }, /* L1I_PREFETCH_MISS -> L1I_PREF_MISS */
    { .hw_gen_event = { 0x10018, 0x0000065 } }, /* DTLB_READ_ACCESS -> L1D_TLB_ACCESS */
    { .hw_gen_event = { 0x10019, 0x0000075 } }, /* DTLB_READ_MISS -> L1D_TLB_MISS */
    { .hw_gen_event = { 0x10028, 0x0000012 } }, /* BPU_READ_ACCESS -> PRD_BRANCH */
    { .hw_gen_event = { 0x10029, 0x0000092 } }, /* BPU_READ_MISS -> PRD_BR_MIS */

    /* last */
    { .hw_gen_event = { SBI_PMU_EVENT_IDX_INVALID, SBI_PMU_EVENT_NOT_SUPP } }
};

#if PLF_PMU_STRICT_EVENTS
const struct sbi_pmu_hw_event hw_raw_events[] = {
    /* General group */
    { .hw_raw_event = { 0x0000001, SCR9_SCR7_SUPPORT } }, /* GEN_CYC */
    { .hw_raw_event = { 0x0000021, SCR9_SCR7_SUPPORT } }, /* GEN_INST */
    { .hw_raw_event = { 0x0000031, SCR9_SCR7_SUPPORT } }, /* GEN_ISS */
    { .hw_raw_event = { 0x0000041, SCR9_SCR7_SUPPORT } }, /* GEN_SPEC_EXC */
    { .hw_raw_event = { 0x0000061, SCR9_SCR7_SUPPORT } }, /* GEN_STRAP */
    { .hw_raw_event = { 0x0000071, SCR9_SCR7_SUPPORT } }, /* GEN_MTRAP */
    { .hw_raw_event = { 0x0000081, SCR9_SCR7_SUPPORT } }, /* GEN_URET */
    { .hw_raw_event = { 0x0000091, SCR9_SCR7_SUPPORT } }, /* GEN_SRET */
    { .hw_raw_event = { 0x00000a1, SCR9_SCR7_SUPPORT } }, /* GEN_MRET */

    /* Prediction group */
    { .hw_raw_event = { 0x0000002, SCR9_SCR7_SUPPORT } }, /* PRD_CFI */
    { .hw_raw_event = { 0x0000012, SCR9_SCR7_SUPPORT } }, /* PRD_BRANCH */
    { .hw_raw_event = { 0x0000022, SCR9_SCR7_SUPPORT } }, /* PRD_CALL */
    { .hw_raw_event = { 0x0000032, SCR9_SCR7_SUPPORT } }, /* PRD_RET */
    { .hw_raw_event = { 0x0000042, SCR9_SCR7_SUPPORT } }, /* PRD_DIR_JMP */
    { .hw_raw_event = { 0x0000052, SCR9_SCR7_SUPPORT } }, /* PRD_REL_JMP */
    { .hw_raw_event = { 0x0000062, SCR9_SCR7_SUPPORT } }, /* PRD_CFI_MIS */
    { .hw_raw_event = { 0x0000072, SCR9_SCR7_SUPPORT } }, /* PRD_CALL_MIS */
    { .hw_raw_event = { 0x0000082, SCR9_SCR7_SUPPORT } }, /* PRD_RET_MIS */
    { .hw_raw_event = { 0x0000092, SCR9_SCR7_SUPPORT } }, /* PRD_BR_MIS */
    { .hw_raw_event = { 0x00000a2, SCR9_SUPPORT } },      /* PRD_LD_MISSPEC */

    /* Execution group */
    { .hw_raw_event = { 0x0000003, SCR9_SCR7_SUPPORT } }, /* EXE_INT_RTR */
    { .hw_raw_event = { 0x0000013, SCR9_SCR7_SUPPORT } }, /* EXE_FP_RTR */
    { .hw_raw_event = { 0x0000023, SCR9_SCR7_SUPPORT } }, /* EXE_MEM_RTR */
    { .hw_raw_event = { 0x0000033, SCR9_SCR7_SUPPORT } }, /* EXE_0IPC_RTR */
    { .hw_raw_event = { 0x0000043, SCR9_SCR7_SUPPORT } }, /* EXE_1IPC_RTR */
    { .hw_raw_event = { 0x0000053, SCR9_SCR7_SUPPORT } }, /* EXE_2IPC_RTR */
    { .hw_raw_event = { 0x0000063, SCR9_SCR7_SUPPORT } }, /* EXE_3IPC_RTR */
    { .hw_raw_event = { 0x0000073, SCR9_SCR7_SUPPORT } }, /* EXE_4IPC_RTR */
    { .hw_raw_event = { 0x0000083, SCR9_SCR7_SUPPORT } }, /* EXE_BE_IDLE */
    { .hw_raw_event = { 0x0000093, SCR9_SCR7_SUPPORT } }, /* EXE_W_ALUMUL */
    { .hw_raw_event = { 0x00000a3, SCR9_SCR7_SUPPORT } }, /* EXE_W_ALUDIV */
    { .hw_raw_event = { 0x00000b3, SCR9_SCR7_SUPPORT } }, /* EXE_W_FPU */
    { .hw_raw_event = { 0x00000c3, SCR9_SCR7_SUPPORT } }, /* EXE_W_LSU */
    { .hw_raw_event = { 0x00000d3, SCR9_SCR7_SUPPORT } }, /* EXE_W_CSR */
    { .hw_raw_event = { 0x00000e3, SCR9_SCR7_SUPPORT } }, /* EXE_ST_ISS */
    { .hw_raw_event = { 0x00000f3, SCR9_SCR7_SUPPORT } }, /* EXE_ST_SRCT */
    { .hw_raw_event = { 0x0000103, SCR9_SCR7_SUPPORT } }, /* EXE_ST_INT */
    { .hw_raw_event = { 0x0000113, SCR9_SCR7_SUPPORT } }, /* EXE_ST_LSU */
    { .hw_raw_event = { 0x0000123, SCR9_SCR7_SUPPORT } }, /* EXE_ST_FPU */
    { .hw_raw_event = { 0x0000133, SCR9_SUPPORT } }, /* EXE_RECOVERY */
    { .hw_raw_event = { 0x0000143, SCR9_SUPPORT } }, /* EXE_RECOVERY_MISPREDICT */
    { .hw_raw_event = { 0x0000153, SCR9_SUPPORT } }, /* EXE_UOPS_NOT_DELIVERED */
    { .hw_raw_event = { 0x0000163, SCR9_SUPPORT } }, /* EXE_ISS_STALL_LSU */

    /* L1I group */
    { .hw_raw_event = { 0x0000004, SCR9_SCR7_SUPPORT } }, /* L1I_HIT */
    { .hw_raw_event = { 0x0000014, SCR9_SCR7_SUPPORT } }, /* L1I_MISS */
    { .hw_raw_event = { 0x0000024, SCR9_SCR7_SUPPORT } }, /* L1I_REFILL */
    { .hw_raw_event = { 0x0000034, SCR9_SCR7_SUPPORT } }, /* L1I_PREF_HIT */
    { .hw_raw_event = { 0x0000044, SCR9_SCR7_SUPPORT } }, /* L1I_PREF_MISS */
    { .hw_raw_event = { 0x0000054, SCR9_SCR7_SUPPORT } }, /* L1I_PREF_REFILL */
    { .hw_raw_event = { 0x0000064, SCR9_SCR7_SUPPORT } }, /* L1I_TLB_ACCESS */
    { .hw_raw_event = { 0x0000074, SCR9_SCR7_SUPPORT } }, /* L1I_TLB_MISS */
    { .hw_raw_event = { 0x0000084, SCR9_SCR7_SUPPORT } }, /* L1I_SNP */
    { .hw_raw_event = { 0x0000094, SCR9_SCR7_SUPPORT } }, /* L1I_SNP_DUMMY */
    { .hw_raw_event = { 0x00000a4, SCR9_SCR7_SUPPORT } }, /* L1I_EVICT */
    { .hw_raw_event = { 0x00000b4, SCR9_SCR7_SUPPORT } }, /* L1I_NC_REQ */
    { .hw_raw_event = { 0x00000c4, SCR9_SCR7_SUPPORT } }, /* L1I_SO_REQ */

    /* L1D group */
    { .hw_raw_event = { 0x0000005, SCR9_SCR7_SUPPORT } }, /* L1D_HIT */
    { .hw_raw_event = { 0x0000015, SCR9_SCR7_SUPPORT } }, /* L1D_MISS */
    { .hw_raw_event = { 0x0000025, SCR9_SCR7_SUPPORT } }, /* L1D_REFILL */
    { .hw_raw_event = { 0x0000035, SCR9_SUPPORT } }, /* L1D_SNP_STALL */
    { .hw_raw_event = { 0x0000045, SCR9_SUPPORT } }, /* L1D_SNP_DUMMY_STALL */
    { .hw_raw_event = { 0x0000055, SCR9_SUPPORT } }, /* L1D_ST_L2_WR */
    { .hw_raw_event = { 0x0000065, SCR9_SCR7_SUPPORT } }, /* L1D_TLB_ACCESS */
    { .hw_raw_event = { 0x0000075, SCR9_SCR7_SUPPORT } }, /* L1D_TLB_MISS */
    { .hw_raw_event = { 0x0000085, SCR9_SCR7_SUPPORT } }, /* L1D_SNP */
    { .hw_raw_event = { 0x0000095, SCR9_SCR7_SUPPORT } }, /* L1D_SNP_DUMMY */
    { .hw_raw_event = { 0x00000a5, SCR9_SCR7_SUPPORT } }, /* L1D_EVICT */
    { .hw_raw_event = { 0x00000b5, SCR9_SCR7_SUPPORT } }, /* L1D_NC_REQ */
    { .hw_raw_event = { 0x00000c5, SCR9_SCR7_SUPPORT } }, /* L1D_SO_REQ */
    { .hw_raw_event = { 0x00000d5, SCR9_SCR7_SUPPORT } }, /* L1D_LD */
    { .hw_raw_event = { 0x00000e5, SCR9_SCR7_SUPPORT } }, /* L1D_ST */
    { .hw_raw_event = { 0x00000f5, SCR9_SCR7_SUPPORT } }, /* L1D_FENCE */
    { .hw_raw_event = { 0x0000105, SCR9_SCR7_SUPPORT } }, /* L1D_AMO */
    { .hw_raw_event = { 0x0000115, SCR9_SCR7_SUPPORT } }, /* L1D_LR */
    { .hw_raw_event = { 0x0000125, SCR9_SCR7_SUPPORT } }, /* L1D_SC */
    { .hw_raw_event = { 0x0000135, SCR9_SCR7_SUPPORT } }, /* L1D_CM */
    { .hw_raw_event = { 0x0000145, SCR9_SUPPORT } }, /* L1D_LD_REPL */
    { .hw_raw_event = { 0x0000155, SCR9_SUPPORT } }, /* L1D_ST_REPL */
    { .hw_raw_event = { 0x0000165, SCR9_SUPPORT } }, /* L1D_PREF_HIT */
    { .hw_raw_event = { 0x0000175, SCR9_SUPPORT } }, /* L1D_PREF_MISS */
    { .hw_raw_event = { 0x0000185, SCR9_SUPPORT } }, /* L1D_LD_MSLGN */
    { .hw_raw_event = { 0x0000195, SCR9_SUPPORT } }, /* L1D_ST_MSLGN */
    { .hw_raw_event = { 0x00001a5, SCR9_SUPPORT } }, /* L1D_LD_VEC */
    { .hw_raw_event = { 0x00001b5, SCR9_SUPPORT } }, /* L1D_ST_VEC */
    { .hw_raw_event = { 0x00001c5, SCR9_SUPPORT } }, /* L1D_ECC_COR */
    { .hw_raw_event = { 0x00001d5, SCR9_SUPPORT } }, /* L1D_ECC_UNCOR */
    { .hw_raw_event = { 0x00001e5, SCR9_SUPPORT } }, /* L1D_ECC_ERR */
    { .hw_raw_event = { 0x00001f5, SCR9_SUPPORT } }, /* L1D_LD_HIT */

    /* Exceptions group */
    { .hw_raw_event = { 0x0000016, SCR9_SCR7_SUPPORT } }, /* EXC_IAF */
    { .hw_raw_event = { 0x0000026, SCR9_SCR7_SUPPORT } }, /* EXC_ILLEG */
    { .hw_raw_event = { 0x0000036, SCR9_SCR7_SUPPORT } }, /* EXC_BP */
    { .hw_raw_event = { 0x0000046, SCR9_SCR7_SUPPORT } }, /* EXC_LAM */
    { .hw_raw_event = { 0x0000056, SCR9_SCR7_SUPPORT } }, /* EXC_LAF */
    { .hw_raw_event = { 0x0000066, SCR9_SCR7_SUPPORT } }, /* EXC_SAM */
    { .hw_raw_event = { 0x0000076, SCR9_SCR7_SUPPORT } }, /* EXC_SAF */
    { .hw_raw_event = { 0x0000086, SCR9_SCR7_SUPPORT } }, /* EXC_UECALL */
    { .hw_raw_event = { 0x0000096, SCR9_SCR7_SUPPORT } }, /* EXC_SECALL */
    { .hw_raw_event = { 0x00000b6, SCR9_SCR7_SUPPORT } }, /* EXC_MECALL */
    { .hw_raw_event = { 0x00000c6, SCR9_SCR7_SUPPORT } }, /* EXC_IPF */
    { .hw_raw_event = { 0x00000d6, SCR9_SCR7_SUPPORT } }, /* EXC_LPF */
    { .hw_raw_event = { 0x00000f6, SCR9_SCR7_SUPPORT } }, /* EXC_SPF */

    /* IRQ group */
    { .hw_raw_event = { 0x0000017, SCR9_SCR7_SUPPORT } }, /* IRQ_SSI */
    { .hw_raw_event = { 0x0000037, SCR9_SCR7_SUPPORT } }, /* IRQ_MSI */
    { .hw_raw_event = { 0x0000057, SCR9_SCR7_SUPPORT } }, /* IRQ_STI */
    { .hw_raw_event = { 0x0000077, SCR9_SCR7_SUPPORT } }, /* IRQ_MTI */
    { .hw_raw_event = { 0x0000097, SCR9_SCR7_SUPPORT } }, /* IRQ_SEI */
    { .hw_raw_event = { 0x00000b7, SCR9_SCR7_SUPPORT } }, /* IRQ_MEI */

	/* Traps group */
    { .hw_raw_event = { 0x0000008, SCR9_SCR7_SUPPORT } }, /* TRP_SSI */
    { .hw_raw_event = { 0x0000018, SCR9_SCR7_SUPPORT } }, /* TRP_MSI */
    { .hw_raw_event = { 0x0000028, SCR9_SCR7_SUPPORT } }, /* TRP_STI */
    { .hw_raw_event = { 0x0000038, SCR9_SCR7_SUPPORT } }, /* TRP_MTI */
    { .hw_raw_event = { 0x0000048, SCR9_SCR7_SUPPORT } }, /* TRP_SEI */
    { .hw_raw_event = { 0x0000058, SCR9_SCR7_SUPPORT } }, /* TRP_MEI */
    { .hw_raw_event = { 0x0000068, SCR9_SCR7_SUPPORT } }, /* TRP_IAF */
    { .hw_raw_event = { 0x0000078, SCR9_SCR7_SUPPORT } }, /* TRP_ILLEG */
    { .hw_raw_event = { 0x0000088, SCR9_SCR7_SUPPORT } }, /* TRP_BP */
    { .hw_raw_event = { 0x0000098, SCR9_SCR7_SUPPORT } }, /* TRP_LAM */
    { .hw_raw_event = { 0x00000a8, SCR9_SCR7_SUPPORT } }, /* TRP_LAF */
    { .hw_raw_event = { 0x00000b8, SCR9_SCR7_SUPPORT } }, /* TRP_SAM */
    { .hw_raw_event = { 0x00000c8, SCR9_SCR7_SUPPORT } }, /* TRP_SAF */
    { .hw_raw_event = { 0x00000d8, SCR9_SCR7_SUPPORT } }, /* TRP_UECALL */
    { .hw_raw_event = { 0x00000e8, SCR9_SCR7_SUPPORT } }, /* TRP_SECALL */
    { .hw_raw_event = { 0x00000f8, SCR9_SCR7_SUPPORT } }, /* TRP_MECALL */
    { .hw_raw_event = { 0x0000108, SCR9_SCR7_SUPPORT } }, /* TRP_IPF */
    { .hw_raw_event = { 0x0000118, SCR9_SCR7_SUPPORT } }, /* TRP_LPF */
    { .hw_raw_event = { 0x0000128, SCR9_SCR7_SUPPORT } }, /* TRP_SPF */
    { .hw_raw_event = { 0x0000138, SCR9_SCR7_SUPPORT } }, /* TRP_TRAP_NUM */
    { .hw_raw_event = { 0x0000148, SCR9_SCR7_SUPPORT } }, /* TRP_IRQ_NUM */

    /* L1D Prefetcher group */
    { .hw_raw_event = { 0x0000009, SCR9_SCR7_SUPPORT } }, /* L1D_PF_ISS */
    { .hw_raw_event = { 0x0000019, SCR9_SCR7_SUPPORT } }, /* L1D_PF_REQ */
    { .hw_raw_event = { 0x0000029, SCR9_SCR7_SUPPORT } }, /* L1D_PF_ISS_HIT */
    { .hw_raw_event = { 0x0000039, SCR9_SCR7_SUPPORT } }, /* L1D_PF_FLT_HIT */
    { .hw_raw_event = { 0x0000049, SCR9_SCR7_SUPPORT } }, /* L1D_PF_HIT_SM */
    { .hw_raw_event = { 0x0000059, SCR9_SCR7_SUPPORT } }, /* L1D_PF_HIT_DC */
    { .hw_raw_event = { 0x0000069, SCR9_SCR7_SUPPORT } }, /* L1D_PF_HIT_CLB */
    { .hw_raw_event = { 0x0000079, SCR9_SCR7_SUPPORT } }, /* L1D_PF_CANCEL */
    { .hw_raw_event = { 0x0000089, SCR9_SCR7_SUPPORT } }, /* L1D_PF_ISS_INV */

    /* HPW group */
    { .hw_raw_event = { 0x000000a, SCR9_SCR7_SUPPORT } }, /* HPW_4K_HIT */
    { .hw_raw_event = { 0x000001a, SCR9_SCR7_SUPPORT } }, /* HPW_MP_HIT */
    { .hw_raw_event = { 0x000002a, SCR9_SCR7_SUPPORT } }, /* HPW_GP_HIT */
    { .hw_raw_event = { 0x000003a, SCR9_SCR7_SUPPORT } }, /* HPW_ITLB_REQ */
    { .hw_raw_event = { 0x000004a, SCR9_SCR7_SUPPORT } }, /* HPW_DTLB_REQ */
    { .hw_raw_event = { 0x000005a, SCR9_SCR7_SUPPORT } }, /* HPW_PREF_REQ */
    { .hw_raw_event = { 0x000006a, SCR9_SCR7_SUPPORT } }, /* HPW_PAGE_FAULT */
    { .hw_raw_event = { 0x000007a, SCR9_SCR7_SUPPORT } }, /* HPW_ACCESS_FAULT */
    { .hw_raw_event = { 0x000008a, SCR9_SCR7_SUPPORT } }, /* HPW_NO_ERR */
    { .hw_raw_event = { 0x000009a, SCR9_SCR7_SUPPORT } }, /* HPW_TP_HIT */
    { .hw_raw_event = { 0x00000aa, SCR9_SCR7_SUPPORT } }, /* HPW_PP_HIT */

    /* sTLB group */
    { .hw_raw_event = { 0x000000b, SCR9_SCR7_SUPPORT } }, /* STLB_REQ */
    { .hw_raw_event = { 0x000001b, SCR9_SCR7_SUPPORT } }, /* STLB_REFILL */
    { .hw_raw_event = { 0x000002b, SCR9_SCR7_SUPPORT } }, /* STLB_ITLB_REQ */
    { .hw_raw_event = { 0x000003b, SCR9_SCR7_SUPPORT } }, /* STLB_DTLB_REQ */
    { .hw_raw_event = { 0x000004b, SCR9_SCR7_SUPPORT } }, /* STLB_ITLB_FLUSH */
    { .hw_raw_event = { 0x000005b, SCR9_SCR7_SUPPORT } }, /* STLB_DTLB_FLUSH */
    { .hw_raw_event = { 0x000006b, SCR9_SCR7_SUPPORT } }, /* STLB_ITLB_MISS */
    { .hw_raw_event = { 0x000007b, SCR9_SCR7_SUPPORT } }, /* STLB_DTLB_MISS */
    { .hw_raw_event = { 0x000008b, SCR9_SCR7_SUPPORT } }, /* STLB_ITLB_STALL */
    { .hw_raw_event = { 0x000009b, SCR9_SCR7_SUPPORT } }, /* STLB_DTLB_STALL */
    { .hw_raw_event = { 0x00000ab, SCR9_SCR7_SUPPORT } }, /* STLB_HPW_STALL */
    { .hw_raw_event = { 0x00000bb, SCR9_SCR7_SUPPORT } }, /* STLB_SFENCE_STALL */
    { .hw_raw_event = { 0x00000cb, SCR9_SCR7_SUPPORT } }, /* STLB_SFENCE_FLUSH_ALL */
    { .hw_raw_event = { 0x00000db, SCR9_SCR7_SUPPORT } }, /* STLB_SFENCE_FLUSH_VPN */
    { .hw_raw_event = { 0x00000eb, SCR9_SCR7_SUPPORT } }, /* STLB_SFENCE_FLUSH_ASID */
    { .hw_raw_event = { 0x00000fb, SCR9_SCR7_SUPPORT } }, /* STLB_SFENCE_FLUSH_TARGET */
    { .hw_raw_event = { 0x000010b, SCR9_SCR7_SUPPORT } }, /* STLB_RESP_WO_ERR */
    { .hw_raw_event = { 0x000011b, SCR9_SCR7_SUPPORT } }, /* STLB_RESP_ANY */

    /* PREF group */
    { .hw_raw_event = { 0x000000c, SCR9_SCR7_SUPPORT } }, /* PREF_REQ */
    { .hw_raw_event = { 0x000001c, SCR9_SCR7_SUPPORT } }, /* PREF_HIT */
    { .hw_raw_event = { 0x000002c, SCR9_SCR7_SUPPORT } }, /* PREF_HPW_REQ */
    { .hw_raw_event = { 0x000003c, SCR9_SCR7_SUPPORT } }, /* PREF_LATE_REQ */

    /* last */
    { .hw_raw_event = { SBI_PMU_EVENT_NOT_SUPP, 0u } }
};
#endif

void sbi_pmu_apply_hw_flags(unsigned long flags, uint64_t *event)
{

    if (flags & SBI_PMU_CFG_FLAG_SET_VUINH)
        *event |= MHPMEVENT_VUINH;

    if (flags & SBI_PMU_CFG_FLAG_SET_VSINH)
        *event |= MHPMEVENT_VSINH;

    if (flags & SBI_PMU_CFG_FLAG_SET_UINH)
        *event |= MHPMEVENT_UINH;

    if (flags & SBI_PMU_CFG_FLAG_SET_SINH)
        *event |= MHPMEVENT_SINH;

    if (flags & SBI_PMU_CFG_FLAG_SET_MINH)
        *event |= MHPMEVENT_MINH;
}

#else
#error "No PMU support for this platform"
#endif

#endif // PLF_PMU_SUPPORT
