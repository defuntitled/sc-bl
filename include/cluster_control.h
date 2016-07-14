/*
 * Copyright (C) 2023, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* framework
///
/// @copyright Copyright (C) 2023, Syntacore Ltd. All Rights Reserved.
/// @author gdi-sc
///
/// @brief Cluster control register definitions

#ifndef SCR_EXT_CONTROL_H
#define SCR_EXT_CONTROL_H

#include "platform_config.h"

#ifndef __ASSEMBLER__
// structure to store register values from SBI HSM
typedef struct {
    unsigned long hartid;
    unsigned long start_addr;
    unsigned long opaque;
    bool initialized;
} sbi_hsm_start_regs;
#endif // __ASSEMBLER__

#ifdef PLF_CLUSTER_CFG_BASE

#define PLF_CLUSTER_CPU_BASE            (PLF_CLUSTER_CFG_BASE + 4 * 1024)
#define CL_CPU_DESCR                    (PLF_CLUSTER_CPU_BASE + 0)    ///< CPU0 Control Register
#define CL_CPU_0_CTRL                   (PLF_CLUSTER_CPU_BASE + 0x40) ///< CPU0 MHARTID Register
#define CL_CPU_0_MHARTID                (PLF_CLUSTER_CPU_BASE + 0x48) ///< CPU0 PC to start after reset
#define CL_CPU_0_RESETPC                (PLF_CLUSTER_CPU_BASE + 0x50) ///< CPU0 JTAG ID code
#define CL_CPU_0_JTAGIDCODE             (PLF_CLUSTER_CPU_BASE + 0x58)
#define CL_CPU_N_CTRL(N)                (CL_CPU_0_CTRL + (N) * 0x40)
#define CL_CPU_N_MHARTID(N)             (CL_CPU_0_MHARTID + (N) * 0x40)
#define CL_CPU_N_RESETPC(N)             (CL_CPU_0_RESETPC + (N) * 0x40)
#define CL_CPU_N_JTAGIDCODE(N)          (CL_CPU_0_JTAGIDCODE + (N) * 0x40)

#define CL_CPU_CTRL_RESET_N_BIT                 (1 << 0)      ///< This bit correpsonds to rst_n pin of CL2
#define CL_CPU_CTRL_WFI_BIT                     (1 << 9)      ///< Core WFI status bit
#define CL_CPU_CTRL_ACT_BIT                     (1 << 10)     ///< Core Activity status bit
#define CL_CPU_CTRL_LINKUP_BIT                  (1ULL << 32)  ///< CPU CHI-Link activation enable/disable control
#define CL_CPU_CTRL_LINKRXREQSTS_BIT            (1ULL << 40)  ///< CPU CHI-RxLink LINKACTIVEREQ status
#define CL_CPU_CTRL_LINKRXASKSTS_BIT            (1ULL << 41)  ///< CPU CHI-RxLink LINKACTIVEASK status
#define CL_CPU_CTRL_LINKTXREQSTS_BIT            (1ULL << 42)  ///< CPU CHI-TxLink LINKACTIVEREQ status
#define CL_CPU_CTRL_LINKTXASKSTS_BIT            (1ULL << 43)  ///< CPU CHI-TxLink LINKACTIVEASK status

#define CL_CPU_CTRL_LINK_STS_MASK               (CL_CPU_CTRL_LINKRXREQSTS_BIT | CL_CPU_CTRL_LINKRXASKSTS_BIT | CL_CPU_CTRL_LINKTXREQSTS_BIT | CL_CPU_CTRL_LINKTXASKSTS_BIT) ///< Mask for 4-bit link status

#ifndef __ASSEMBLER__

void plf_cluster_cpu_init(void);
void plf_local_cpu_deactivation_prepare(void);
void plf_ext_cpu_deactivate(uint32_t cpu_id);
void plf_ext_cpu_activate(uint32_t cpu_id);

#endif // __ASSEMBLER__

#endif // PLF_CLUSTER_CFG_BASE

#endif // SCR_EXT_CONTROL_H
