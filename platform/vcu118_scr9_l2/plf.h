/*
 * Copyright (C) 2023, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCRx framework
///
/// @copyright Copyright (C) 2023, Syntacore Ltd. All Rights Reserved.
/// @author sm-sc
///
/// @brief platform specific configuration: SCR9/L2/PMP with default memory map

#ifndef PLATFORM_VCU118_SCR9_L2_CONFIG_H
#define PLATFORM_VCU118_SCR9_L2_CONFIG_H

#define PLF_RTC_TIMEBASE 1000000

#define PLF_CORE_VARIANT_SCR9 1
#define PLF_IMPL_STR "Syntacore FPGA"

//----------------------
// feature configuration
//----------------------

#define PLF_SMP_SUPPORT 1
#define PLF_AIA_SUPPORT 0
#define PLF_PMP_SUPPORT 1
#define PLF_MPU_SUPPORT 0

#define PLF_SUPPORT_CACHE_INFO 1

//----------------------
// memory configuration
//----------------------
#define PLF_MEM_BASE    EXPAND32ADDR(0)
#define PLF_MEM_SIZE    (4*1024UL*1024UL*1024UL)
#define PLF_MEM_ATTR    (SCR_PMP_CTRL_MT_WEAKLY | PMP_RWX)
#define PLF_MEM_NAME    "DDR"

#define PLF_CMEM_SIZE   (16*1024*1024)
#define PLF_CMEM_BASE   (PLF_MEM_BASE + PLF_MEM_SIZE - PLF_CMEM_SIZE)
#define PLF_CMEM_ATTR   (SCR_PMP_CTRL_MT_STRONG |  \
                         PMP_R |         \
                         PMP_W)
#define PLF_CMEM_NAME   "DMA RAM"

#define PLF_MMCFG_BASE  EXPAND32ADDR(0xf0040000)
#define PLF_MMCFG_SIZE (8*1024)
#define PLF_MMCFG_ATTR (SCR_PMP_CTRL_MT_CONFIG)
#define PLF_MMCFG_NAME "MMCFG"

#define PLF_MTIMER_BASE (PLF_MMCFG_BASE)
#define PLF_L2CTL_BASE  (PLF_MMCFG_BASE + 0x1000)

#define PLF_MMIO_BASE   EXPAND32ADDR(0xff000000)
#define PLF_MMIO_SIZE   (8*1024*1024)
#define PLF_MMIO_ATTR   (SCR_PMP_CTRL_MT_STRONG | \
                         PMP_R |        \
                         PMP_W)
#define PLF_MMIO_NAME   "MMIO"

#if PLF_AIA_SUPPORT == 0
#define PLF_PLIC_BASE   EXPAND32ADDR(0xfe000000)
#define PLF_PLIC_SIZE   (16*1024*1024)
#define PLF_PLIC_ATTR   (SCR_PMP_CTRL_MT_STRONG |  \
                         PMP_R |         \
                         PMP_W)
#define PLF_PLIC_NAME   "PLIC"
#else
// TODO: redefine in accordance with HW implementation
#define PLF_APLIC_BASE  (0xffffff0c000000)
#define PLF_APLIC_SIZE  (16*1024)
#define PLF_APLIC_ATTR  (SCR_PMP_CTRL_MT_STRONG |  \
                         PMP_R |         \
                         PMP_W)
#define PLF_APLIC_NAME   "APLIC"

#define PLF_IMSIC_BASE_M  (0xffffff24000000)
#define PLF_IMSIC_SIZE_M  (8*4*1024)
#define PLF_IMSIC_ATTR_M  (SCR_PMP_CTRL_MT_STRONG |  \
                           PMP_R |         \
                           PMP_W)
#define PLF_IMSIC_NAME_M   "IMSIC_M"

#define PLF_IMSIC_BASE_S  (0xffffff28000000)
#define PLF_IMSIC_SIZE_S  (8*4*1024)
#define PLF_IMSIC_ATTR_S  (SCR_PMP_CTRL_MT_STRONG |  \
                           PMP_R |         \
                           PMP_W)
#define PLF_IMSIC_NAME_S   "IMSIC_S"
#endif // PLF_AIA_SUPPORT

#define PLF_OCRAM_BASE  EXPAND32ADDR(0xffff0000)
#define PLF_OCRAM_SIZE  (64*1024)
#define PLF_OCRAM_ATTR  (SCR_PMP_CTRL_MT_WEAKLY)
#define PLF_OCRAM_NAME  "On-Chip RAM"

#define PLF_KERNEL_ENTRY PLF_MEM_BASE
#define PLF_NVRAM_KERNEL_BASE (0x4000000)
#define PLF_NVRAM_KERNEL_SIZE (64*1024*1024)

#ifdef PLF_CMEM_SIZE
#define PLF_CMEM_REGION {PLF_CMEM_BASE, PLF_CMEM_SIZE, PLF_CMEM_ATTR, PLF_CMEM_NAME},
#else
#define PLF_CMEM_REGION
#endif

//----------------------
// For MRT and PMP, the last entry in this table is the most prioritized. So
// for PMP platforms keep CMEM region after MEM to make sure that CMEM is
// uncached.
//----------------------
#if PLF_AIA_SUPPORT == 0
#define PLF_MEM_MAP                                                     \
    {PLF_MEM_BASE, PLF_MEM_SIZE, PLF_MEM_ATTR, PLF_MEM_NAME},           \
    PLF_CMEM_REGION                                                     \
    {PLF_MMCFG_BASE, PLF_MMCFG_SIZE, PLF_MMCFG_ATTR, PLF_MMCFG_NAME},   \
    {PLF_PLIC_BASE, PLF_PLIC_SIZE, PLF_PLIC_ATTR, PLF_PLIC_NAME},       \
    {PLF_MMIO_BASE, PLF_MMIO_SIZE, PLF_MMIO_ATTR, PLF_MMIO_NAME},       \
    {PLF_OCRAM_BASE, PLF_OCRAM_SIZE, PLF_OCRAM_ATTR, PLF_OCRAM_NAME}
#else
#define PLF_MEM_MAP                                                     \
    {PLF_MEM_BASE, PLF_MEM_SIZE, PLF_MEM_ATTR, PLF_MEM_NAME},           \
    PLF_CMEM_REGION                                                     \
    {PLF_MMCFG_BASE, PLF_MMCFG_SIZE, PLF_MMCFG_ATTR, PLF_MMCFG_NAME},   \
    {PLF_MMIO_BASE, PLF_MMIO_SIZE, PLF_MMIO_ATTR, PLF_MMIO_NAME},       \
    {PLF_OCRAM_BASE, PLF_OCRAM_SIZE, PLF_OCRAM_ATTR, PLF_OCRAM_NAME},   \
    {PLF_APLIC_BASE, PLF_APLIC_SIZE, PLF_APLIC_ATTR, PLF_APLIC_NAME},   \
    {PLF_IMSIC_BASE_M, PLF_IMSIC_SIZE_M, PLF_IMSIC_ATTR_M, PLF_IMSIC_NAME_M}, \
    {PLF_IMSIC_BASE_S, PLF_IMSIC_SIZE_S, PLF_IMSIC_ATTR_S, PLF_IMSIC_NAME_S}
#endif // PLF_AIA_SUPPORT

//----------------------
// MMIO configuration
//----------------------

// FPGA build ID
#define PLF_BLD_ID_ADDR (PLF_MMIO_BASE + 0)
// FPGA sysclk, MHz
#define PLF_SYSCLK_MHZ_ADDR (PLF_MMIO_BASE + 0x1000)
#define PLF_SYS_CLK (*(const uint32_t*)(PLF_SYSCLK_MHZ_ADDR) * 1000000)
// FPGA cpuclk, MHz
#define PLF_CPUCLK_MHZ_ADDR PLF_SYSCLK_MHZ_ADDR
// FPGA UART port
#define PLF_UART0_BASE  (PLF_MMIO_BASE + 0x10000)
#define PLF_UART0_16550
// Quad SPI interface
#define PLF_XQSPI_BASE  (PLF_MMIO_BASE + 0x30000)

#if PLF_XGMAC_SUPPORT && !PLF_AIA_SUPPORT
// QEMU xgmac addr
#define PLF_XGMAC_BASE  (PLF_MMIO_BASE + 0x20000)
#endif // PLF_XGMAC_SUPPORT && !PLF_AIA_SUPPORT

//----------------------
// cache configuration
//----------------------

// min cache line
#define PLF_CACHELINE_SIZE 16

// global configuration: cachable
#define PLF_CACHE_CFG CACHE_GLBL_ENABLE

#define PLF_AUTOSTART_DELAY 10
#define PLF_AUTOSTART_NVRAM

#define PLF_APLIC_NUM_SOURCE 96
// IRQ delegation configuration. Each line corresponds to one domain with its
// index matching the line number. Format is following:
// { first IRQ, last IRQ, child domain index }
// TODO: child domain index for SCR9 S-mode is 1 in docs
#define PLF_APLIC_DELEGATE         \
    {1, PLF_APLIC_NUM_SOURCE, 0},

#define PLF_APLIC_LHXW_M 3
#define PLF_APLIC_LHXS_M 0
#define PLF_APLIC_HHXW_M 0
#define PLF_APLIC_LHXW_S 3
#define PLF_APLIC_LHXS_S 0
#define PLF_APLIC_HHXW_S 0

#define PLF_IMSIC_NUM_IDS 127

#define PLF_PMU_SUPPORT 1
#define PLF_L2_PMU_SUPPORT 1
#define PLF_PMU_STRICT_EVENTS 0

#endif // PLATFORM_VCU118_SCR9_L2_CONFIG_H
