/*
 * Copyright (C) 2022, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* infra
///
/// @copyright Copyright (C) 2022, Syntacore Ltd. All Rights Reserved.
/// @author sm-sc
///
/// @brief PMU (Performance Monitoring Unit)

#ifndef SCR_PMU_H
#define SCR_PMU_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "platform_config.h"
#include "bitops.h"
#include "csr.h"
#include "sbi.h"
#include "hls.h"
#include "utils.h"

#if PLF_PMU_SUPPORT

/*
 * Platform specific part
 *
 */

#if defined(PLF_CORE_VARIANT_SCR7) || defined(PLF_CORE_VARIANT_SCR9)

/*
 *  bits 0,2: fixed PMU counters CYCLE, INSTRET
 *  bits 3-31: configurable PMU counters, not all of them may be available
 *  bit 1: TIME accessibility, not for PMU
 */
#define PMU_HW_CTR_MAX   31
#define PMU_HW_CTR_START 3
#else
#error "No PMU support for this platform"
#endif

/*
 * Common part
 *
 */

#ifdef PLF_SMP_HART_NUM
#define PMU_HART_NUM    PLF_SMP_HART_NUM
#else
#define PMU_HART_NUM    1
#endif

/* User HPM counters/timers */

#define CSR_CYCLE           0xc00
#define CSR_TIME            0xc01
#define CSR_INSTRET         0xc02
#define CSR_HPMCOUNTER3     0xc03
#define CSR_HPMCOUNTER4     0xc04
#define CSR_HPMCOUNTER5     0xc05
#define CSR_HPMCOUNTER6     0xc06
#define CSR_HPMCOUNTER7     0xc07
#define CSR_HPMCOUNTER8     0xc08
#define CSR_HPMCOUNTER9     0xc09
#define CSR_HPMCOUNTER10    0xc0a
#define CSR_HPMCOUNTER11    0xc0b
#define CSR_HPMCOUNTER12    0xc0c
#define CSR_HPMCOUNTER13    0xc0d
#define CSR_HPMCOUNTER14    0xc0e
#define CSR_HPMCOUNTER15    0xc0f
#define CSR_HPMCOUNTER16    0xc10
#define CSR_HPMCOUNTER17    0xc11
#define CSR_HPMCOUNTER18    0xc12
#define CSR_HPMCOUNTER19    0xc13
#define CSR_HPMCOUNTER20    0xc14
#define CSR_HPMCOUNTER21    0xc15
#define CSR_HPMCOUNTER22    0xc16
#define CSR_HPMCOUNTER23    0xc17
#define CSR_HPMCOUNTER24    0xc18
#define CSR_HPMCOUNTER25    0xc19
#define CSR_HPMCOUNTER26    0xc1a
#define CSR_HPMCOUNTER27    0xc1b
#define CSR_HPMCOUNTER28    0xc1c
#define CSR_HPMCOUNTER29    0xc1d
#define CSR_HPMCOUNTER30    0xc1e
#define CSR_HPMCOUNTER31    0xc1f

#define CSR_CYCLEH          0xc80
#define CSR_TIMEH           0xc81
#define CSR_INSTRETH        0xc82
#define CSR_HPMCOUNTER3H    0xc83
#define CSR_HPMCOUNTER4H    0xc84
#define CSR_HPMCOUNTER5H    0xc85
#define CSR_HPMCOUNTER6H    0xc86
#define CSR_HPMCOUNTER7H    0xc87
#define CSR_HPMCOUNTER8H    0xc88
#define CSR_HPMCOUNTER9H    0xc89
#define CSR_HPMCOUNTER10H   0xc8a
#define CSR_HPMCOUNTER11H   0xc8b
#define CSR_HPMCOUNTER12H   0xc8c
#define CSR_HPMCOUNTER13H   0xc8d
#define CSR_HPMCOUNTER14H   0xc8e
#define CSR_HPMCOUNTER15H   0xc8f
#define CSR_HPMCOUNTER16H   0xc90
#define CSR_HPMCOUNTER17H   0xc91
#define CSR_HPMCOUNTER18H   0xc92
#define CSR_HPMCOUNTER19H   0xc93
#define CSR_HPMCOUNTER20H   0xc94
#define CSR_HPMCOUNTER21H   0xc95
#define CSR_HPMCOUNTER22H   0xc96
#define CSR_HPMCOUNTER23H   0xc97
#define CSR_HPMCOUNTER24H   0xc98
#define CSR_HPMCOUNTER25H   0xc99
#define CSR_HPMCOUNTER26H   0xc9a
#define CSR_HPMCOUNTER27H   0xc9b
#define CSR_HPMCOUNTER28H   0xc9c
#define CSR_HPMCOUNTER29H   0xc9d
#define CSR_HPMCOUNTER30H   0xc9e
#define CSR_HPMCOUNTER31H   0xc9f

/* Machine HPM counters/timers */

#define CSR_MCYCLE              0xb00
#define CSR_MINSTRET            0xb02
#define CSR_MHPMCOUNTER3        0xb03
#define CSR_MHPMCOUNTER4        0xb04
#define CSR_MHPMCOUNTER5        0xb05
#define CSR_MHPMCOUNTER6        0xb06
#define CSR_MHPMCOUNTER7        0xb07
#define CSR_MHPMCOUNTER8        0xb08
#define CSR_MHPMCOUNTER9        0xb09
#define CSR_MHPMCOUNTER10       0xb0a
#define CSR_MHPMCOUNTER11       0xb0b
#define CSR_MHPMCOUNTER12       0xb0c
#define CSR_MHPMCOUNTER13       0xb0d
#define CSR_MHPMCOUNTER14       0xb0e
#define CSR_MHPMCOUNTER15       0xb0f
#define CSR_MHPMCOUNTER16       0xb10
#define CSR_MHPMCOUNTER17       0xb11
#define CSR_MHPMCOUNTER18       0xb12
#define CSR_MHPMCOUNTER19       0xb13
#define CSR_MHPMCOUNTER20       0xb14
#define CSR_MHPMCOUNTER21       0xb15
#define CSR_MHPMCOUNTER22       0xb16
#define CSR_MHPMCOUNTER23       0xb17
#define CSR_MHPMCOUNTER24       0xb18
#define CSR_MHPMCOUNTER25       0xb19
#define CSR_MHPMCOUNTER26       0xb1a
#define CSR_MHPMCOUNTER27       0xb1b
#define CSR_MHPMCOUNTER28       0xb1c
#define CSR_MHPMCOUNTER29       0xb1d
#define CSR_MHPMCOUNTER30       0xb1e
#define CSR_MHPMCOUNTER31       0xb1f
#define CSR_MCYCLEH             0xb80
#define CSR_MINSTRETH           0xb82
#define CSR_MHPMCOUNTER3H       0xb83
#define CSR_MHPMCOUNTER4H       0xb84
#define CSR_MHPMCOUNTER5H       0xb85
#define CSR_MHPMCOUNTER6H       0xb86
#define CSR_MHPMCOUNTER7H       0xb87
#define CSR_MHPMCOUNTER8H       0xb88
#define CSR_MHPMCOUNTER9H       0xb89
#define CSR_MHPMCOUNTER10H      0xb8a
#define CSR_MHPMCOUNTER11H      0xb8b
#define CSR_MHPMCOUNTER12H      0xb8c
#define CSR_MHPMCOUNTER13H      0xb8d
#define CSR_MHPMCOUNTER14H      0xb8e
#define CSR_MHPMCOUNTER15H      0xb8f
#define CSR_MHPMCOUNTER16H      0xb90
#define CSR_MHPMCOUNTER17H      0xb91
#define CSR_MHPMCOUNTER18H      0xb92
#define CSR_MHPMCOUNTER19H      0xb93
#define CSR_MHPMCOUNTER20H      0xb94
#define CSR_MHPMCOUNTER21H      0xb95
#define CSR_MHPMCOUNTER22H      0xb96
#define CSR_MHPMCOUNTER23H      0xb97
#define CSR_MHPMCOUNTER24H      0xb98
#define CSR_MHPMCOUNTER25H      0xb99
#define CSR_MHPMCOUNTER26H      0xb9a
#define CSR_MHPMCOUNTER27H      0xb9b
#define CSR_MHPMCOUNTER28H      0xb9c
#define CSR_MHPMCOUNTER29H      0xb9d
#define CSR_MHPMCOUNTER30H      0xb9e
#define CSR_MHPMCOUNTER31H      0xb9f

/* Machine counter setup */

#define CSR_MCOUNTINHIBIT       0x320
#define CSR_MHPMEVENT3          0x323
#define CSR_MHPMEVENT4          0x324
#define CSR_MHPMEVENT5          0x325
#define CSR_MHPMEVENT6          0x326
#define CSR_MHPMEVENT7          0x327
#define CSR_MHPMEVENT8          0x328
#define CSR_MHPMEVENT9          0x329
#define CSR_MHPMEVENT10         0x32a
#define CSR_MHPMEVENT11         0x32b
#define CSR_MHPMEVENT12         0x32c
#define CSR_MHPMEVENT13         0x32d
#define CSR_MHPMEVENT14         0x32e
#define CSR_MHPMEVENT15         0x32f
#define CSR_MHPMEVENT16         0x330
#define CSR_MHPMEVENT17         0x331
#define CSR_MHPMEVENT18         0x332
#define CSR_MHPMEVENT19         0x333
#define CSR_MHPMEVENT20         0x334
#define CSR_MHPMEVENT21         0x335
#define CSR_MHPMEVENT22         0x336
#define CSR_MHPMEVENT23         0x337
#define CSR_MHPMEVENT24         0x338
#define CSR_MHPMEVENT25         0x339
#define CSR_MHPMEVENT26         0x33a
#define CSR_MHPMEVENT27         0x33b
#define CSR_MHPMEVENT28         0x33c
#define CSR_MHPMEVENT29         0x33d
#define CSR_MHPMEVENT30         0x33e
#define CSR_MHPMEVENT31         0x33f

/* Machine RV32 counter setup */

#define CSR_MHPMEVENT3H         0x723
#define CSR_MHPMEVENT4H         0x724
#define CSR_MHPMEVENT5H         0x725
#define CSR_MHPMEVENT6H         0x726
#define CSR_MHPMEVENT7H         0x727
#define CSR_MHPMEVENT8H         0x728
#define CSR_MHPMEVENT9H         0x729
#define CSR_MHPMEVENT10H        0x72a
#define CSR_MHPMEVENT11H        0x72b
#define CSR_MHPMEVENT12H        0x72c
#define CSR_MHPMEVENT13H        0x72d
#define CSR_MHPMEVENT14H        0x72e
#define CSR_MHPMEVENT15H        0x72f
#define CSR_MHPMEVENT16H        0x730
#define CSR_MHPMEVENT17H        0x731
#define CSR_MHPMEVENT18H        0x732
#define CSR_MHPMEVENT19H        0x733
#define CSR_MHPMEVENT20H        0x734
#define CSR_MHPMEVENT21H        0x735
#define CSR_MHPMEVENT22H        0x736
#define CSR_MHPMEVENT23H        0x737
#define CSR_MHPMEVENT24H        0x738
#define CSR_MHPMEVENT25H        0x739
#define CSR_MHPMEVENT26H        0x73a
#define CSR_MHPMEVENT27H        0x73b
#define CSR_MHPMEVENT28H        0x73c
#define CSR_MHPMEVENT29H        0x73d
#define CSR_MHPMEVENT30H        0x73e
#define CSR_MHPMEVENT31H        0x73f

#define MHPMEVENT_OF            (_UL(1) << 63)
#define MHPMEVENT_MINH          (_UL(1) << 62)
#define MHPMEVENT_SINH          (_UL(1) << 61)
#define MHPMEVENT_UINH          (_UL(1) << 60)
#define MHPMEVENT_VSINH         (_UL(1) << 59)
#define MHPMEVENT_VUINH         (_UL(1) << 58)

#define PMU_DFLT_EN_MINH(x)	((x) & ~5UL)

#ifndef __ASSEMBLER__

#define ARCH_MASK_SUPPORT(arch)  BIT(arch)
#define SCR7_SUPPORT             ARCH_MASK_SUPPORT(7)
#define SCR9_SUPPORT             ARCH_MASK_SUPPORT(9)

/* SBI specification: PMU counters info */
union sbi_pmu_ctr_info {
    unsigned long value;
    struct {
        unsigned long csr:12;
        unsigned long width:6;
#if __riscv_xlen == 32
        unsigned long reserved:13;
#else
        unsigned long reserved:45;
#endif
        unsigned long type:1;
    };
};

/* hardware event data */
struct sbi_pmu_hw_event {
    union {
        struct hw_gen_event {
            uint32_t event_idx;
            uint64_t select;
        } hw_gen_event;
        struct hw_raw_event {
            uint64_t select;
            uint32_t arch_mask;
        } hw_raw_event;
    };
};

void pmu_hart_init(void);
unsigned long sbi_pmu_num_ctr(unsigned long *out);
unsigned long sbi_pmu_ctr_get_info(unsigned long cidx, unsigned long *out);
unsigned long sbi_pmu_ctr_cfg_match(unsigned long cidx_base,
        unsigned long cidx_mask, unsigned long flags,
        unsigned long eidx, unsigned long data1,
        unsigned long data2, unsigned long *out);
unsigned long sbi_pmu_ctr_start(unsigned long cbase,
        unsigned long cmask, unsigned long flags,
        unsigned long data1, unsigned long data2);
unsigned long sbi_pmu_ctr_stop(unsigned long cbase,
        unsigned long cmask, unsigned long flags);
unsigned long sbi_pmu_fw_ctr_read(unsigned long cidx,
        unsigned long *out);
void sbi_pmu_apply_hw_flags(unsigned long flags, uint64_t *event);

#endif // !__ASSEMBLER__
#endif // PLF_PMU_SUPPORT
#endif // SCR_PMU_H
