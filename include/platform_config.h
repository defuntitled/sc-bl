/*
 * Copyright (C) 2015, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* framework
///
/// @copyright Copyright (C) 2015-2016, Syntacore Ltd. All Rights Reserved.
/// @author mn-sc
///
/// @brief platform specific configurations

#ifndef PLATFORM_CONFIG_H
#define PLATFORM_CONFIG_H

#ifndef PLATFORM
#define PLATFORM unknown
#endif

#ifdef __ASSEMBLER__
#define _AC(X,Y)	X
#define _AT(T,X)	X
#else // __ASSEMBLER__
#define __AC(X,Y)	(X##Y)
#define _AC(X,Y)	__AC(X,Y)
#define _AT(T,X)	((T)(X))
#endif // __ASSEMBLER__

#define _UL(x)		(_AC(x, UL))
#define _ULL(x)		(_AC(x, ULL))

#if __riscv_xlen == 32
#define EXPAND32ADDR(x) (x)
#else
#define EXPAND32ADDR(x) (((((x) / 0x80000000) & 1) * 0xffffffff00000000) + (x))
#endif

#ifdef PLATFORM_HDR
#include PLATFORM_HDR
#else
#include "plf.h"
#endif

#ifdef PLF_CORE_VARIANT_SCR1
#define PLF_CORE_VARIANT_SCR 1
#define PLF_CORE_VARIANT     SCR1
#elif defined(PLF_CORE_VARIANT_SCR3)
#define PLF_CORE_VARIANT_SCR 3
#define PLF_CORE_VARIANT     SCR3
#elif defined(PLF_CORE_VARIANT_SCR4)
#define PLF_CORE_VARIANT_SCR 4
#define PLF_CORE_VARIANT     SCR4
#elif defined(PLF_CORE_VARIANT_SCR5)
#define PLF_CORE_VARIANT_SCR 5
#define PLF_CORE_VARIANT     SCR5
#elif defined(PLF_CORE_VARIANT_SCR6)
#define PLF_CORE_VARIANT_SCR 6
#define PLF_CORE_VARIANT     SCR6
#elif defined(PLF_CORE_VARIANT_SCR7)
#define PLF_CORE_VARIANT_SCR 7
#define PLF_CORE_VARIANT     SCR7
#elif defined(PLF_CORE_VARIANT_SCR9)
#define PLF_CORE_VARIANT_SCR 9
#define PLF_CORE_VARIANT     SCR9
#else
#error Platform CPU core variant is not defined
#endif

// platform info
#ifndef PLF_CPU_NAME
#define PLF_CPU_NAME #PLF_CORE_VARIANT
#endif
#ifndef PLF_IMPL_STR
#define PLF_IMPL_STR "custom"
#endif

// cache info

#ifndef PLF_CACHELINE_SIZE
#define PLF_CACHELINE_SIZE 0
#endif // PLF_CACHELINE_SIZE

#ifndef PLF_CACHE_INFO_L1I
#define PLF_CACHE_INFO_L1I 0
#endif // PLF_CACHE_INFO_L1I
#ifndef PLF_CACHE_INFO_L1D
#define PLF_CACHE_INFO_L1D 0
#endif // PLF_CACHE_INFO_L1D

#ifndef PLF_CACHE_L2_DEDICATED
#define PLF_CACHE_L2_DEDICATED 0
#endif // PLF_CACHE_L2_DEDICATED

#ifndef PLF_UART_BAUDRATE
#define PLF_UART_BAUDRATE 115200
#endif

#ifndef PLF_MASTER_HART
#define PLF_MASTER_HART 0
#endif

#ifndef PLF_RTC_SRC_EXTERNAL
#define PLF_RTC_SRC_EXTERNAL 0
#endif

#ifndef PLF_RTC_TIMEBASE
#define PLF_RTC_TIMEBASE 1000000
#endif

#ifndef PLF_HSM_SUPPORT
#if PLF_SMP_SUPPORT
#define PLF_HSM_SUPPORT 1
#endif
#endif

#if defined(PLF_CLUSTER_CFG_BASE) && !defined(PLF_HSM_HOTPLUG)
#define PLF_HSM_HOTPLUG 1
#endif

#if !PLF_HSM_SUPPORT && PLF_HSM_HOTPLUG
#error "PLF_HSM_HOTPLUG enabled without PLF_HSM_SUPPORT"
#endif

#ifndef PLF_MPU_SUPPORT
#define PLF_MPU_SUPPORT (PLF_CORE_VARIANT_SCR >= 3)
#endif

#ifndef PLF_MMODE_ONLY
#define PLF_MMODE_ONLY (PLF_CORE_VARIANT_SCR < 5)
#endif

#if PLF_SMP_SUPPORT
#ifndef PLF_SMP_HART_NUM

#if __riscv_xlen > 32
#define PLF_SMP_HART_NUM (8)
#else
#define PLF_SMP_HART_NUM (4)
#endif /* __riscv_xlen > 32 */

#endif /* PLF_SMP_HART_NUM */
#endif /* PLF_SMP_SUPPORT */

#if !(PLF_MMODE_ONLY)
#ifndef PAGE_OFFSET
#if __riscv_xlen == 32
#define PAGE_OFFSET    (0xc0000000)
#else
// __riscv_cmodel_medany == 1
/* #define PAGE_OFFSET    (0xffffffff80000000) // cmodel= medlow */
#define PAGE_OFFSET    (0xffffffe000000000) // cmodel= medany
#endif
#endif // !PAGE_OFFSET
#ifndef KERNEL_VIRT_SIZE
#if __riscv_xlen == 32
#define KERNEL_VIRT_SIZE (0x40000000) // 1G
#else // __riscv_xlen == 32
#if PAGE_OFFSET == 0xffffffe000000000
#define KERNEL_VIRT_SIZE (0x2000000000) // 128G
#else
#define KERNEL_VIRT_SIZE (0x80000000) // 2G
#endif // PAGE_OFFSET == 0xffffffe000000000
#endif // __riscv_xlen == 32
#endif // KERNEL_VIRT_SIZE
#else
#define KERNEL_VIRT_SIZE (0x20000000)
#endif // !PLF_MMODE_ONLY

#ifndef PLF_IPI_ASM_HANDLERS
#define PLF_IPI_ASM_HANDLERS 1
#endif

#ifndef PLF_NVRAM_CHIPSELECT
#define PLF_NVRAM_CHIPSELECT  (0x1)
#endif

#ifndef __ASSEMBLER__
#define SC_STATIC_ASSERT(pred, msg) typedef char static_assertion_##msg[(!!(pred))*2-1]
#endif // __ASSEMBLER__

//MISALIGN
#ifndef PLF_MISALIGN_ACCESS_SUPPORT
#define PLF_MISALIGN_ACCESS_SUPPORT 0
#endif // PLF_MISALIGN_ACCESS_SUPPORT

#ifndef PLF_MISALIGN_ACCESS_ACTIVATE
#define PLF_MISALIGN_ACCESS_ACTIVATE 0
#endif // PLF_MISALIGN_ACCESS_ACTIVATE

#if defined(PLF_RISCV_ISA) && defined(PLF_RVA22_SUPPORT)
#error "PLF_RISCV_ISA and PLF_RVA22_SUPPORT are incompatible"
#endif

#if defined(PLF_RVA22_SUPPORT) && !defined(PLF_CORE_VARIANT_SCR9)
#error "PLF_RVA22_SUPPORT is supported for SCR9 only"
#endif

#define WRONG_STARTUP_ADDRESS_POISON 0xDEFEC8ED

#ifndef PLF_PMU_STRICT_EVENTS
#define PLF_PMU_STRICT_EVENTS 1
#endif

#endif // PLATFORM_CONFIG_H
