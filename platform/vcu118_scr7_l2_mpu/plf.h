/*
 * Copyright (C) 2015, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCRx framework
///
/// @copyright Copyright (C) 2015-2020, Syntacore Ltd. All Rights Reserved.
/// @author mn-sc
///
/// @brief platform specific configurations

#ifndef PLATFORM_VCU118_SCR7_L2_MPU_CONFIG_H
#define PLATFORM_VCU118_SCR7_L2_MPU_CONFIG_H

#define PLF_RTC_TIMEBASE 1000000

#define PLF_CORE_VARIANT_SCR7 1
#define PLF_IMPL_STR "Syntacore FPGA"

#define PLF_SUPPORT_CACHE_INFO 1

//----------------------
// memory configuration
//----------------------
#define PLF_MEM_BASE    EXPAND32ADDR(0)
#if __riscv_xlen == 32
#define PLF_MEM_SIZE    (2*1024UL*1024UL*1024UL)
#else
#define PLF_MEM_SIZE    (4*1024UL*1024UL*1024UL)
#endif
#define PLF_MEM_ATTR    (SCR_MPU_CTRL_MT_WEAKLY | SCR_MPU_CTRL_ALL)
#define PLF_MEM_NAME    "DDR"

#define PLF_CMEM_SIZE   (16*1024*1024)
#define PLF_CMEM_BASE   (PLF_MEM_BASE + PLF_MEM_SIZE - PLF_CMEM_SIZE)
#define PLF_CMEM_ATTR   (SCR_MPU_CTRL_MT_STRONG |  \
                         SCR_MPU_CTRL_MR |         \
                         SCR_MPU_CTRL_MW |         \
                         SCR_MPU_CTRL_SR |         \
                         SCR_MPU_CTRL_SW)
#define PLF_CMEM_NAME   "DMA RAM"

#define PLF_MMCFG_BASE  EXPAND32ADDR(0xf0040000)
#define PLF_MMCFG_SIZE (8*1024)
#define PLF_MMCFG_ATTR (SCR_MPU_CTRL_MT_CFG | \
                        SCR_MPU_CTRL_MR |     \
                        SCR_MPU_CTRL_MW)
#define PLF_MMCFG_NAME "MMCFG"

#define PLF_MTIMER_BASE (PLF_MMCFG_BASE)
#define PLF_L2CTL_BASE  (PLF_MMCFG_BASE + 0x1000)

#define PLF_MMIO_BASE   EXPAND32ADDR(0xff000000)
#define PLF_MMIO_SIZE   (8*1024*1024)
#define PLF_MMIO_ATTR   (SCR_MPU_CTRL_MT_STRONG |  \
                         SCR_MPU_CTRL_MR |         \
                         SCR_MPU_CTRL_MW |         \
                         SCR_MPU_CTRL_SR |         \
                         SCR_MPU_CTRL_SW)
#define PLF_MMIO_NAME   "MMIO"

#define PLF_PLIC_BASE   EXPAND32ADDR(0xfe000000)
#define PLF_PLIC_SIZE   (16*1024*1024)
#define PLF_PLIC_ATTR   (SCR_MPU_CTRL_MT_STRONG |  \
                         SCR_MPU_CTRL_MR |         \
                         SCR_MPU_CTRL_MW |         \
                         SCR_MPU_CTRL_SR |         \
                         SCR_MPU_CTRL_SW)
#define PLF_PLIC_NAME   "PLIC"

#define PLF_OCRAM_BASE  EXPAND32ADDR(0xffff0000)
#define PLF_OCRAM_SIZE  (64*1024)
#define PLF_OCRAM_ATTR  (SCR_MPU_CTRL_MT_WEAKLY | \
                         SCR_MPU_CTRL_MA | \
                         SCR_MPU_CTRL_SR)
#define PLF_OCRAM_NAME  "On-Chip RAM"

#define PLF_KERNEL_ENTRY PLF_MEM_BASE
#define PLF_NVRAM_KERNEL_BASE (0x4000000)
#define PLF_NVRAM_KERNEL_SIZE (64*1024*1024)

#ifdef PLF_CMEM_SIZE
#define PLF_CMEM_REGION {PLF_CMEM_BASE, PLF_CMEM_SIZE, PLF_CMEM_ATTR, PLF_CMEM_NAME},
#else
#define PLF_CMEM_REGION
#endif

#define PLF_MEM_MAP                                                     \
    {PLF_MEM_BASE, PLF_MEM_SIZE, PLF_MEM_ATTR, PLF_MEM_NAME},           \
    PLF_CMEM_REGION                                                     \
    {PLF_MMCFG_BASE, PLF_MMCFG_SIZE, PLF_MMCFG_ATTR, PLF_MMCFG_NAME},   \
    {PLF_PLIC_BASE, PLF_PLIC_SIZE, PLF_PLIC_ATTR, PLF_PLIC_NAME},       \
    {PLF_MMIO_BASE, PLF_MMIO_SIZE, PLF_MMIO_ATTR, PLF_MMIO_NAME},       \
    {PLF_OCRAM_BASE, PLF_OCRAM_SIZE, PLF_OCRAM_ATTR, PLF_OCRAM_NAME}

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

//----------------------
// cache configuration
//----------------------

// min cache line
#define PLF_CACHELINE_SIZE 16

// global configuration: cachable
#define PLF_CACHE_CFG CACHE_GLBL_ENABLE

#define PLF_SMP_SUPPORT 1

#define PLF_AUTOSTART_DELAY 10
#define PLF_AUTOSTART_NVRAM

#define PLF_PMU_SUPPORT 1
#define PLF_L2_PMU_SUPPORT 1
#define PLF_PMU_STRICT_EVENTS 0

#endif // PLATFORM_VCU118_SCR7_L2_MPU_CONFIG_H
