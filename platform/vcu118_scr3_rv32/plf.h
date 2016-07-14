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

#ifndef PLATFORM_VCU118_SCR3_RV32_CONFIG_H
#define PLATFORM_VCU118_SCR3_RV32_CONFIG_H

#define PLF_RTC_TIMEBASE 1000000

#define PLF_CORE_VARIANT_SCR3 1
#define PLF_IMPL_STR "Syntacore FPGA"

//----------------------
// memory configuration
//----------------------
#define PLF_MEM_BASE    EXPAND32ADDR(0)
#if __riscv_xlen > 32
#define PLF_MEM_SIZE    (4UL*1024*1024*1024)
#else
#define PLF_MEM_SIZE    (2UL*1024*1024*1024)
#endif
#define PLF_MEM_ATTR    (SCR_MPU_CTRL_MT_WEAKLY | SCR_MPU_CTRL_ALL)
#define PLF_MEM_NAME    "DDR"

#define PLF_TCM_BASE    EXPAND32ADDR(0xf0000000)
#define PLF_TCM_SIZE    (128*1024)
#define PLF_TCM_ATTR    (SCR_MPU_CTRL_MT_WEAKLY | SCR_MPU_CTRL_ALL)
#define PLF_TCM_NAME    "TCM"

#define PLF_TCM_EXT_PORT_BASE    EXPAND32ADDR(0xfd000000)
#define PLF_TCM_EXT_PORT_SIZE    (PLF_TCM_SIZE)
#define PLF_TCM_EXT_PORT_ATTR    (PLF_TCM_ATTR)
#define PLF_TCM_EXT_PORT_NAME    "TCM (Ext port)"


#define PLF_MMCFG_BASE  EXPAND32ADDR(0xf0040000)
#define PLF_MMCFG_SIZE  (8*1024)
#define PLF_MMCFG_ATTR  (SCR_MPU_CTRL_MT_CFG | \
                         SCR_MPU_CTRL_MR |     \
                         SCR_MPU_CTRL_MW)
#define PLF_MMCFG_NAME  "MMCFG"

#define PLF_MTIMER_BASE (PLF_MMCFG_BASE)

#define PLF_MMIO_BASE   EXPAND32ADDR(0xff000000)
#define PLF_MMIO_SIZE   (8*1024*1024)
#define PLF_MMIO_ATTR   (SCR_MPU_CTRL_MT_STRONG |  \
                         SCR_MPU_CTRL_MR |         \
                         SCR_MPU_CTRL_MW |         \
                         SCR_MPU_CTRL_SR |         \
                         SCR_MPU_CTRL_SW)
#define PLF_MMIO_NAME   "MMIO"

#define PLF_OCRAM_BASE  EXPAND32ADDR(0xffff0000)
#define PLF_OCRAM_SIZE  (64*1024)
#define PLF_OCRAM_ATTR  (SCR_MPU_CTRL_MT_WEAKLY | \
                         SCR_MPU_CTRL_MA | \
                         SCR_MPU_CTRL_SR)
#define PLF_OCRAM_NAME  "On-Chip RAM"

#define PLF_NVRAM_KERNEL_BASE (0x4000000)
#define PLF_NVRAM_KERNEL_SIZE (64*1024*1024)

#define PLF_MEM_MAP                                                     \
    {PLF_MEM_BASE, PLF_MEM_SIZE, PLF_MEM_ATTR, PLF_MEM_NAME},           \
    {PLF_TCM_BASE, PLF_TCM_SIZE, PLF_TCM_ATTR, PLF_TCM_NAME},           \
    {PLF_TCM_EXT_PORT_BASE, PLF_TCM_EXT_PORT_SIZE, PLF_TCM_EXT_PORT_ATTR, PLF_TCM_EXT_PORT_NAME},\
    {PLF_MMCFG_BASE, PLF_MMCFG_SIZE, PLF_MMCFG_ATTR, PLF_MMCFG_NAME},   \
    {PLF_MMIO_BASE, PLF_MMIO_SIZE, PLF_MMIO_ATTR, PLF_MMIO_NAME},       \
    {PLF_OCRAM_BASE, PLF_OCRAM_SIZE, PLF_OCRAM_ATTR, PLF_OCRAM_NAME}

//----------------------
// MMIO configuration
//----------------------

// FPGA UART port
#define PLF_UART0_BASE   (PLF_MMIO_BASE + 0x10000)
#define PLF_UART0_16550

// FPGA and System ID
#ifndef PLF_SKIP_BLD_ID
    // FPGA build ID
    #define PLF_BLD_ID_ADDR  (PLF_MMIO_BASE + 0)
    // FPGA system ID
    #define PLF_SYS_ID_ADDR  (PLF_MMIO_BASE + 0x1000)
#endif

// FPGA sysclk, MHz
#define PLF_SYSCLK_MHZ_ADDR (PLF_MMIO_BASE + 0x2000)
#define VarClkMHz (*(const uint32_t*)(PLF_SYSCLK_MHZ_ADDR) * 1000000)

// FPGA cpuclk, MHz
#define PLF_CPUCLK_MHZ_ADDR PLF_SYSCLK_MHZ_ADDR

#ifndef PLF_SYS_CLK
    #define PLF_SYS_CLK VarClkMHz
#endif

// Quad SPI interface
#define PLF_XQSPI_BASE  (PLF_MMIO_BASE + 0x30000)

//----------------------
// cache configuration
//----------------------

// min cache line
#define PLF_CACHELINE_SIZE 16

// global configuration: cachable
#define PLF_CACHE_CFG CACHE_GLBL_ENABLE

//----------------------
// misc configuration
//----------------------

#define PLF_MEM_TEST_ENABLED 1
#define PLF_XMODEM_ENABLED 1


// external interrupt lines
#define PLF_INTLINE_UART0 0

#define PLF_IPIC_IRQ_MAP                 \
        [0 ... 31] = ~0,                 \
        [12] = PLF_INTLINE_UART0

#endif // PLATFORM_VCU118_SCR3_RV32_CONFIG_H
