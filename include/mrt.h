/*
 * Copyright (C) 2015, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* infra
///
/// @copyright Copyright (C) 2015-2023, Syntacore Ltd. All Rights Reserved.
/// @author 
///
/// @brief MRT defs and funcs

#ifndef SCR_INFRA_MRT_H
#define SCR_INFRA_MRT_H

#include <stddef.h>
#include "arch.h"
#include "mmu.h"

#include <hal/drivers/mrt.h>

#if PLF_MRT_SUPPORT


#define MRT_ENTRY_NUM(descr)            ((descr >> 8) & 0xff)

#define MRT_ENTRY_VALID                 (1)

#ifdef PLF_ENTRY_BOOT_DEFAULT
#define MRT_ENTRY_BOOT_DEFAULT          (PLF_ENTRY_BOOT_DEFAULT)
#else
#define MRT_ENTRY_BOOT_DEFAULT          (1)
#endif
#define MRT_ENTRY_BOOT_MASK             (0xFFFFFFFF00000000ULL)

#define MRT_ENTRY_IO_DEFAULT            (2) 
#define MRT_ENTRY_IO_ADDR               (PLF_MMIO_BASE)
#define MRT_ENTRY_IO_MASK               (~(PLF_MMIO_SIZE - 1))

#define MRT_REGION_SIZE 0x100000000ULL
#define MRT_IO_REGION_SIZE 0x10000

static inline int scr_mrt_num_regions(void)
{
    uint64_t descr = *(volatile uint64_t *)MRT_VID;
    return MRT_ENTRY_NUM(descr);
}

static inline void scr_mrt_default_setup(void)
{
    uint64_t rd_val = *(volatile uint64_t*)MRT_A_ENTRY_N_CTRL(MRT_ENTRY_BOOT_DEFAULT) & MRT_CTRL_MTYPE_MASK;
    create_mrt_region(MRT_ENTRY_BOOT_DEFAULT, 0, MRT_REGION_SIZE, rd_val | (MRT_ENTRY_VALID | MRT_CTRL_MTYPE_C_WO));
    create_mrt_region(MRT_ENTRY_IO_DEFAULT, MRT_ENTRY_IO_ADDR, MRT_IO_REGION_SIZE, (MRT_CTRL_MTYPE_NC_SO | MRT_ENTRY_VALID));
    *(volatile uint64_t *)MRT_CMD = MRT_CMD_RUN_BIT;
    while( (*(volatile uint64_t *)MRT_CMD & MRT_CMD_RUN_BIT) == 1);
    ifence();
}

#endif // PLF_MRT_SUPPORT
#endif // SCR_INFRA_MRT_H
