/*
 * Copyright (C) 2015, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* infra
///
/// @copyright Copyright (C) 2015-2019, Syntacore Ltd. All Rights Reserved.
/// @author mn-sc
///
/// @brief Cache CSR's defs and inlines

#ifndef SCR_INFRA_CACHE_H
#define SCR_INFRA_CACHE_H

#include "platform_config.h"
#include "csr.h"

// cache control CSRs
#define SCR_CSR_CACHE_GLBL 0xbd4
// cache info CSRs
#define SCR_CSR_CACHE_DSCR_L1 0xfc3

// global cache's control bits
#define CACHE_GLBL_L1I_EN (1 << 0)
#define CACHE_GLBL_L1D_EN (1 << 1)
#define CACHE_GLBL_L1I_INV (1 << 2)
#define CACHE_GLBL_L1D_INV (1 << 3)
#define CACHE_GLBL_ENABLE (CACHE_GLBL_L1I_EN | CACHE_GLBL_L1D_EN)
#define CACHE_GLBL_DISABLE 0
#define CACHE_GLBL_INV (CACHE_GLBL_L1I_INV | CACHE_GLBL_L1D_INV)

#define L1_CSR_DSCR_WAYS_OFFS    (0)
#define L1_CSR_DSCR_LINESZ_OFFS  (4)
#define L1_CSR_DSCR_LINES_OFFS   (8)
#define L1_CSR_DSCR_WAYS_MASK    (0x7)
#define L1_CSR_DSCR_LINESZ_MASK  (0xf)
#define L1_CSR_DSCR_LINES_MASK   (0x1f)

#define L1_CSR_DSCR_L1_MASK      (0xffff)
#define L1_CSR_DSCR_L1D_OFFS     (16)

#define CACHE_INFO_L1I \
	(read_csr(SCR_CSR_CACHE_DSCR_L1) & L1_CSR_DSCR_L1_MASK)
#define CACHE_INFO_L1D \
	((read_csr(SCR_CSR_CACHE_DSCR_L1) >> L1_CSR_DSCR_L1D_OFFS) & \
			L1_CSR_DSCR_L1_MASK)

// L2$ registers
#ifdef PLF_L2CTL_BASE
#define L2_CSR_VER_IDX   0
#define L2_CSR_DESCR_IDX 1
#define L2_CSR_EN_IDX    4
#define L2_CSR_FLUSH_IDX 5
#define L2_CSR_INV_IDX   6
#define L2_CSR_BUSY_IDX  11
#define L2_CSR_SYSCO_IDX 40

#define L2_CSR_VER_OFFS   (L2_CSR_VER_IDX * 4)
#define L2_CSR_DESCR_OFFS (L2_CSR_DESCR_IDX * 4)
#define L2_CSR_EN_OFFS    (L2_CSR_EN_IDX * 4)
#define L2_CSR_FLUSH_OFFS (L2_CSR_FLUSH_IDX * 4)
#define L2_CSR_INV_OFFS   (L2_CSR_INV_IDX * 4)
#define L2_CSR_BUSY_OFFS  (L2_CSR_BUSY_IDX * 4)
#define L2_CSR_SYSCO_OFFS (L2_CSR_SYSCO_IDX * 4)

#define L2_CSR_VER   (PLF_L2CTL_BASE + L2_CSR_VER_OFFS)
#define L2_CSR_DESCR (PLF_L2CTL_BASE + L2_CSR_DESCR_OFFS)
#define L2_CSR_EN    (PLF_L2CTL_BASE + L2_CSR_FLUSH_OFFS)
#define L2_CSR_FLUSH (PLF_L2CTL_BASE + L2_CSR_FLUSH_OFFS)
#define L2_CSR_INV   (PLF_L2CTL_BASE + L2_CSR_INV_OFFS)

#define L2_CSR_DESCR_OFFS_BANKS      (16)
#define L2_CSR_DESCR_OFFS_WAYS       (0)
#define L2_CSR_DESCR_OFFS_LINESZ_LG2 (4)
#define L2_CSR_DESCR_OFFS_LINES_LG2  (8)
#define L2_CSR_DESCR_OFFS_CORES      (28)
#define L2_CSR_DESCR_OFFS_TYPE       (13)

#define L2_CSR_DESCR_MASK_BANKS      (0xf)
#define L2_CSR_DESCR_MASK_WAYS       (0x7)
#define L2_CSR_DESCR_MASK_LINESZ_LG2 (0xf)
#define L2_CSR_DESCR_MASK_LINES_LG2  (0x1f)
#define L2_CSR_DESCR_MASK_CORES      (0xf)
#define L2_CSR_DESCR_MASK_TYPE       (0x7)

#endif // PLF_L2CTL_BASE

// L3$ registers
#ifdef PLF_L3CTL_BASE

#define L3_REG_SIZE          (8)

#define L3C_VID_IDX          (0)
#define L3C_DESCR_CACHE_IDX  (1)
#define L3C_DESCR_BANK_IDX   (2)
#define L3C_PCE_CTRL_IDX     (19)

#define L3C_CMD_CTRL_IDX     (8)
#define L3C_CMD_ADDR_IDX     (9)
#define L3C_CMD_DATA0_IDX    (10)
#define L3C_CMD_DATA1_IDX    (11)
#define L3C_CMD_DATA2_IDX    (12)
#define L3C_CMD_DATA3_IDX    (13)
#define L3C_CMD_DATA4_IDX    (14)
#define L3C_CMD_DATA5_IDX    (15)
#define L3C_CMD_DATA6_IDX    (16)
#define L3C_CMD_DATA7_IDX    (17)

#define L3C_VID_OFFS         (L3C_VID_IDX * L3_REG_SIZE)
#define L3C_DESCR_CACHE_OFFS (L3C_DESCR_CACHE_IDX * L3_REG_SIZE)
#define L3C_DESCR_BANK_OFFS  (L3C_DESCR_BANK_IDX * L3_REG_SIZE)
#define L3C_PCE_CTRL_OFFS    (L3C_PCE_CTRL_IDX * L3_REG_SIZE)

#define L3C_CMD_CTRL_OFFS    (L3C_CMD_CTRL_IDX * L3_REG_SIZE)
#define L3C_CMD_ADDR_OFFS    (L3C_CMD_ADDR_IDX * L3_REG_SIZE)
#define L3C_CMD_DATA0_OFFS   (L3C_CMD_DATA0_IDX * L3_REG_SIZE)
#define L3C_CMD_DATA1_OFFS   (L3C_CMD_DATA1_IDX * L3_REG_SIZE)
#define L3C_CMD_DATA2_OFFS   (L3C_CMD_DATA2_IDX * L3_REG_SIZE)
#define L3C_CMD_DATA3_OFFS   (L3C_CMD_DATA3_IDX * L3_REG_SIZE)
#define L3C_CMD_DATA4_OFFS   (L3C_CMD_DATA4_IDX * L3_REG_SIZE)
#define L3C_CMD_DATA5_OFFS   (L3C_CMD_DATA5_IDX * L3_REG_SIZE)
#define L3C_CMD_DATA6_OFFS   (L3C_CMD_DATA6_IDX * L3_REG_SIZE)
#define L3C_CMD_DATA7_OFFS   (L3C_CMD_DATA7_IDX * L3_REG_SIZE)

#define L3C_VID              (PLF_L3CTL_BASE + L3C_VID_OFFS)
#define L3C_DESCR_CACHE      (PLF_L3CTL_BASE + L3C_DESCR_CACHE_OFFS)
#define L3C_DESCR_BANK       (PLF_L3CTL_BASE + L3C_DESCR_BANK_OFFS)
#define L3C_PCE_CTRL_N(N)    (PLF_L3CTL_BASE + L3C_PCE_CTRL_OFFS + (N) * L3_REG_SIZE)

#define L3C_CMD_CTRL         (PLF_L3CTL_BASE + L3C_CMD_CTRL_OFFS)
#define L3C_CMD_ADDR         (PLF_L3CTL_BASE + L3C_CMD_ADDR_OFFS)
#define L3C_CMD_DATA0        (PLF_L3CTL_BASE + L3C_CMD_DATA0_OFFS)
#define L3C_CMD_DATA1        (PLF_L3CTL_BASE + L3C_CMD_DATA1_OFFS)
#define L3C_CMD_DATA2        (PLF_L3CTL_BASE + L3C_CMD_DATA2_OFFS)
#define L3C_CMD_DATA3        (PLF_L3CTL_BASE + L3C_CMD_DATA3_OFFS)
#define L3C_CMD_DATA4        (PLF_L3CTL_BASE + L3C_CMD_DATA4_OFFS)
#define L3C_CMD_DATA5        (PLF_L3CTL_BASE + L3C_CMD_DATA5_OFFS)
#define L3C_CMD_DATA6        (PLF_L3CTL_BASE + L3C_CMD_DATA6_OFFS)
#define L3C_CMD_DATA7        (PLF_L3CTL_BASE + L3C_CMD_DATA7_OFFS)

#define L3C_DESCR_CACHE_OFFS_CPU_NUM    (0)
#define L3C_DESCR_CACHE_OFFS_BANK_NUM   (8)
#define L3C_DESCR_CACHE_OFFS_MEM_NUM    (16)
#define L3C_DESCR_CACHE_OFFS_IO_NUM     (24)

#define L3C_DESCR_CACHE_MASK_CPU_NUM    (0xff)
#define L3C_DESCR_CACHE_MASK_BANK_NUM   (0xff)
#define L3C_DESCR_CACHE_MASK_MEM_NUM    (0xff)
#define L3C_DESCR_CACHE_MASK_IO_NUM     (0xff)

#define L3C_DESCR_BANK_OFFS_IDX_NUM    (0)
#define L3C_DESCR_BANK_OFFS_WAY_NUM    (8)
#define L3C_DESCR_BANK_OFFS_LINE_WIDTH (16)
#define L3C_DESCR_BANK_OFFS_DAT_WIDTH  (24)
#define L3C_DESCR_BANK_OFFS_ADDR_WIDTH (32)

#define L3C_DESCR_BANK_MASK_IDX_NUM    (0xff)
#define L3C_DESCR_BANK_MASK_WAY_NUM    (0xff)
#define L3C_DESCR_BANK_MASK_LINE_WIDTH (0xff)
#define L3C_DESCR_BANK_MASK_DAT_WIDTH  (0xff)
#define L3C_DESCR_BANK_MASK_ADDR_WIDTH (0xff)

#define L3C_OPCODE_NOP                  (0x00)
#define L3C_OPCODE_FLUASH               (0x01)
#define L3C_OPCODE_INVALID              (0x02)
#define L3C_OPCODE_CLEAN                (0x03)
#define L3C_OPCODE_LOAD_ADDR            (0x04)
#define L3C_OPCODE_LOAD_DIRECT          (0x05)

#define L3C_CMD_CTRL_VECTOR_SHIFT       (8)
#define L3C_CMD_CTRL_ALL_BANKS          (0xFF)

#define L3C_CMD_INVALIDATE_ALL          ( (L3C_CMD_CTRL_ALL_BANKS << L3C_CMD_CTRL_VECTOR_SHIFT) | L3C_OPCODE_INVALID )

#define L3B_STEP                        (0x100)
#define L3B_PRIVATE_0_BANK              (PLF_L3CTL_BASE + L3B_STEP)
#define L3B_PRIVATE_N_BANK(N)           (L3B_PRIVATE_0_BANK + (N) * L3B_STEP)

#define L3B_STS_IDX                     (0)
#define L3B_STS_OFFS                    (L3B_STS_IDX * L3_REG_SIZE)
#define L3B_STS_N(N)                    (L3B_PRIVATE_N_BANK(N) + L3B_STS_OFFS)
#define L3B_MCP_STS_IDX                 (1)
#define L3B_MCP_STS_OFFS                (L3B_MCP_STS_IDX * L3_REG_SIZE)
#define L3B_MCP_STS_N(N)                (L3B_PRIVATE_N_BANK(N) + L3B_MCP_STS_OFFS)
#define L3B_CMD_STS_IDX                 (2)
#define L3B_CMD_STS_OFFS                (L3B_CMD_STS_IDX * L3_REG_SIZE)
#define L3B_CMD_STS_N(N)                (L3B_PRIVATE_N_BANK(N) + L3B_CMD_STS_OFFS)
#define L3B_PCE_CTRL_IDX                (21)
#define L3B_PCE_CTRL_OFFS               (L3B_PCE_CTRL_IDX * L3_REG_SIZE)
#define L3B_PCE_CTRL_0(BANK)            (L3B_PRIVATE_N_BANK(BANK) + L3B_PCE_CTRL_OFFS)
#define L3B_PCE_CTRL_STEP               (0x10)
#define L3B_PCE_CTRL_N(BANK, N)         (L3B_PCE_CTRL_0(BANK) + (N) * L3B_PCE_CTRL_STEP)
#define L3B_PCE_CNTR_IDX                (22)
#define L3B_PCE_CNTR_OFFS               (L3B_PCE_CNTR_IDX * L3_REG_SIZE)
#define L3B_PCE_CNTR_0(BANK)            (L3B_PRIVATE_N_BANK(BANK) + L3B_PCE_CNTR_OFFS)
#define L3B_PCE_CNTR_STEP               (0x10)
#define L3B_PCE_CNTR_N(BANK, N)         (L3B_PCE_CNTR_0(BANK) + (N) * L3B_PCE_CNTR_STEP)

#define L3B_CMD_STS_OFFS_DONE           (0)
#define L3B_CMD_STS_MASK_DONE           (0x1)
#define L3B_CMD_STS_OFFS_DENY           (4)
#define L3B_CMD_STS_MASK_DENY           (0x1)
#define L3B_CMD_STS_OFFS_MISS           (8)
#define L3B_CMD_STS_MASK_MISS           (0x1)


#endif // PLF_L3CTL_BASE

#include "asm-magic.h"

#ifndef __ASSEMBLER__

#ifdef __GNUC__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "arch.h"
#include "hls.h"
#include "vm.h" // num_harts

struct cache_l1 {
	unsigned int lines;
	unsigned int ways;
	unsigned int line_size;
	unsigned int sets;
	unsigned int size;
};

struct cache_l2 {
	unsigned int banks;
	unsigned int lines;
	unsigned int ways;
	unsigned int line_size;
	unsigned int sets;
	unsigned int size;
	int type;
	unsigned int cpu_num;
};

struct cache_l3 {
	unsigned int banks;
	unsigned int lines;
	unsigned int ways;
	unsigned int line_size;
	unsigned int sets;
	unsigned int size;
	int type;
	unsigned int cpu_num;
	unsigned int mem_num;
	unsigned int io_num;
};

static inline bool cache_l1_available(void)
{
    return read_csr(SCR_CSR_CACHE_DSCR_L1) != 0;
}

// setup global cache policy
static inline void cache_l1_ctrl(unsigned long ctrl_val)
{
    if (cache_l1_available()) {
        write_csr(SCR_CSR_CACHE_GLBL, ctrl_val);
        ifence();
    }
}

// setup global cache policy
static inline bool cache_l1_enabled(void)
{
    if (!cache_l1_available())
        return false;
    return read_csr(SCR_CSR_CACHE_GLBL) & (CACHE_GLBL_L1I_EN | CACHE_GLBL_L1D_EN);
}

static inline void cache_l1_enable(void)
{
    cache_l1_ctrl(CACHE_GLBL_ENABLE);
}

static inline void cache_l1_disable(void)
{
    if (cache_l1_available()) {
        write_csr(SCR_CSR_CACHE_GLBL, CACHE_GLBL_DISABLE | CACHE_GLBL_INV);
        ifence();
        // wait until invalidation complete
        while (read_csr(SCR_CSR_CACHE_GLBL) & (CACHE_GLBL_INV))
            fence();
    }
}

static inline void scr_l1cache_init(void)
{
#ifdef PLF_CACHE_CFG
    // setup global cache policy
    cache_l1_ctrl(PLF_CACHE_CFG);
#endif // PLF_CACHE_CFG
}

// returns false if some harts have different L1 configurations, true otherwise
static inline bool scr_l1caches_identical()
{
#if PLF_SMP_SUPPORT
    unsigned long l1i = hart_hls(0)->l1i_info;
    unsigned long l1d = hart_hls(0)->l1d_info;
    for (unsigned i = 1; i < num_harts; ++i)
        if (hart_hls(i)->l1i_info != l1i || hart_hls(i)->l1d_info != l1d)
            return false;
#endif // PLF_SMP_SUPPORT
    return true;
}

static inline struct cache_l1 parse_l1_info(unsigned info)
{
    struct cache_l1 res = { 0 };

    res.ways = 1 << (info & L1_CSR_DSCR_WAYS_MASK);
    res.line_size = 1 << ((info >> L1_CSR_DSCR_LINESZ_OFFS) & L1_CSR_DSCR_LINESZ_MASK);
    res.lines = 1 << ((info >> L1_CSR_DSCR_LINES_OFFS) & L1_CSR_DSCR_LINES_MASK);
    res.size = res.lines * res.line_size * res.ways;
    res.sets = res.lines;

    return res;
}

static inline void show_l1cache_info(const char *prefix, unsigned info, bool enabled) {
    const char *state = enabled ? "enabled" : "disabled";
    if (info) {
        struct cache_l1 c = parse_l1_info(info);

        printf("%-4s[%08x] %uK, %u-way, %u-byte line, %s\n",
               prefix, info, c.size / 1024, c.ways, c.line_size, state);
    } else {
        printf("%-4s: not available\n", prefix);
    }
}

#ifdef PLF_CACHE_CFG

static inline struct cache_l1 get_l1i_cache(unsigned hart)
{
	return parse_l1_info(hart_hls(hart)->l1i_info);
}

static inline struct cache_l1 get_l1d_cache(unsigned hart)
{
	return parse_l1_info(hart_hls(hart)->l1d_info);
}

#endif

// GAS macro: custom op
// clflush <reg> (cache line flush & invalidate)
asm (".macro clflush reg;"
     "__scr_reg2num \\reg;"
     ".word 0x10900073 + (__scr_macro_regn << 15);"
     ".endm;"
    );

// GAS macro: custom op
// clinvd <reg> (cache line invalidate)
asm (".macro clinvd reg;"
     "__scr_reg2num \\reg;"
     ".word 0x10800073 + (__scr_macro_regn << 15);"
     ".endm;"
    );

static inline void cache_invalidate(void *vaddr, unsigned long size)
{
    fence();

#if PLF_CACHELINE_SIZE > 0
    if (cache_l1_available() && size) {
        unsigned long cnt;
        unsigned long a0 = (unsigned long)vaddr;

        size += a0 & (PLF_CACHELINE_SIZE - 1);
        cnt = (size - 1) / PLF_CACHELINE_SIZE;

        do {
            asm ("clinvd %0" :: "r"(a0) : "memory");
            a0 += PLF_CACHELINE_SIZE;
        } while (cnt--);
    }
#endif // PLF_CACHELINE_SIZE > 0
}

static inline void cache_flush(void *vaddr, unsigned long size)
{
#if PLF_CACHELINE_SIZE > 0
    if (cache_l1_available() && size) {
        unsigned long cnt;
        unsigned long a0 = (unsigned long)vaddr;

        size += a0 & (PLF_CACHELINE_SIZE - 1);
        cnt = (size - 1) / PLF_CACHELINE_SIZE;

        do {
            asm ("clflush %0" :: "r"(a0) : "memory");
            a0 += PLF_CACHELINE_SIZE;
        } while (cnt--);
    }
#endif // PLF_CACHELINE_SIZE > 0

    fence();
}

static inline void scr_l2cache_enable(void)
{
#if PLF_L2CTL_BASE
    // init L2$: disable, confirm  state, invalidate, confirm, enable, confirm
    volatile uint32_t *l2ctl = (uint32_t*)PLF_L2CTL_BASE;
    uint32_t cbmask;

    if (!l2ctl[L2_CSR_VER_IDX])
        return; // cache not exists or not supported

    cbmask = (1 << (((l2ctl[L2_CSR_DESCR_IDX] >> 16) & 0xf) + 1)) - 1;

    // disable L2$
    l2ctl[L2_CSR_EN_IDX] = 0;
    // confirm state
    while (l2ctl[L2_CSR_BUSY_IDX]);
    // invalidate
    l2ctl[L2_CSR_INV_IDX] = cbmask;
    // confirm state
    while (l2ctl[L2_CSR_BUSY_IDX]);
#if PLF_L3CTL_BASE
    // enable external coherency only if there is L3
    l2ctl[L2_CSR_SYSCO_IDX] = 1;
    // confirm state
    while (l2ctl[L2_CSR_BUSY_IDX]);
#endif
    // enable
    l2ctl[L2_CSR_EN_IDX] = cbmask;
    // confirm state
    while (l2ctl[L2_CSR_BUSY_IDX]);
#endif // PLF_L2CTL_BASE
}

static inline void scr_l2cache_disable(void)
{
#if PLF_L2CTL_BASE
    volatile uint32_t *l2ctl = (uint32_t*)PLF_L2CTL_BASE;

    if (!l2ctl[L2_CSR_VER_IDX])
        return; // cache not exists or not supported

    // sync (flush) on-chip RAM
    cache_flush((void*)PLF_OCRAM_BASE, PLF_OCRAM_SIZE);

    // disable
    l2ctl[L2_CSR_EN_IDX] = 0;
    // confirm state
    while (l2ctl[L2_CSR_BUSY_IDX]);
    // flush overall
    l2ctl[L2_CSR_FLUSH_IDX] = ~0;
    // confirm state
    while (l2ctl[L2_CSR_BUSY_IDX]);
    // invalidate
    l2ctl[L2_CSR_INV_IDX] = ~0;
    // confirm state
    while (l2ctl[L2_CSR_BUSY_IDX]);
#if PLF_L3CTL_BASE
    // disable external coherency only if there is L3
    l2ctl[L2_CSR_SYSCO_IDX] = 0;
    // confirm state
    while (l2ctl[L2_CSR_BUSY_IDX]);
#endif
#endif // PLF_L2CTL_BASE
}

static inline bool scr_l2cache_is_enabled(void)
{
#if PLF_L2CTL_BASE
    volatile uint32_t *l2ctl = (uint32_t*)PLF_L2CTL_BASE;

    return l2ctl[L2_CSR_VER_IDX] && l2ctl[L2_CSR_EN_IDX];
#else
    return false;
#endif // PLF_L2CTL_BASE
}

static inline void scr_l2cache_init(void)
{
#ifndef PLF_L2_NOINIT
    scr_l2cache_enable();
#endif // PLF_L2_NOINIT
}

static inline unsigned scr_l2cache_description(void)
{
#if PLF_L2CTL_BASE
    volatile uint32_t *l2ctl = (volatile uint32_t*)PLF_L2CTL_BASE;
    if (!l2ctl[L2_CSR_VER_IDX])
        return 0; // cache not exists or not supported
    return l2ctl[L2_CSR_DESCR_IDX];
#else
    return 0;
#endif // PLF_L2CTL_BASE
}

// returns false if some harts have different L2 configurations, true otherwise
static inline bool scr_l2caches_identical()
{
#if PLF_SMP_SUPPORT && PLF_CACHE_L2_DEDICATED
    unsigned long l2 = hart_hls(0)->l2_descr;
    for (unsigned i = 1; i < num_harts; ++i)
        if (hart_hls(i)->l2_descr != l2)
            return false;
#endif // PLF_SMP_SUPPORT && PLF_CACHE_L2_DEDICATED
    return true;
}

#if PLF_L2CTL_BASE

static inline struct cache_l2 parse_cache_l2_info(unsigned l2dscr)
{
    struct cache_l2 res = { 0 };

    res.banks = 1 + ((l2dscr >> L2_CSR_DESCR_OFFS_BANKS) & L2_CSR_DESCR_MASK_BANKS);
    res.ways = 1 << ((l2dscr >> L2_CSR_DESCR_OFFS_WAYS) & L2_CSR_DESCR_MASK_WAYS);
    res.line_size = 1 << ((l2dscr >> L2_CSR_DESCR_OFFS_LINESZ_LG2) & L2_CSR_DESCR_MASK_LINESZ_LG2);
    res.lines = 1 << ((l2dscr >> L2_CSR_DESCR_OFFS_LINES_LG2) & L2_CSR_DESCR_MASK_LINES_LG2);
    res.type = (l2dscr >> L2_CSR_DESCR_OFFS_TYPE) & L2_CSR_DESCR_MASK_TYPE;
    res.cpu_num = 1 + ((l2dscr >> L2_CSR_DESCR_OFFS_CORES) & L2_CSR_DESCR_MASK_CORES);
    res.size = res.lines * res.line_size * res.ways * res.banks;
    res.sets = res.banks * res.lines;

    return res;
}

#endif

static inline int scr_l2cache_info(unsigned hartn, char *buf, unsigned len)
{
#if PLF_L2CTL_BASE
    volatile uint32_t *l2ctl = (volatile uint32_t*)PLF_L2CTL_BASE;

    unsigned l2ver = l2ctl[L2_CSR_VER_IDX];
    unsigned l2dscr = hart_hls(hartn)->l2_descr;

    if (l2ver) {
        struct cache_l2 c = parse_cache_l2_info(l2dscr);
        unsigned size_kb = c.size / 1024;
        unsigned status = l2ctl[L2_CSR_EN_IDX];
        const char *state = status ? "enabled" : "disabled";


        /* static const char *cache_types[8] = { */
        /*     [0] = "RO", */
        /*     [1] = "Write-through, no allocate", */
        /*     [7] = "Write-back, allocate, L1 inclusive" */
        /* }; */

        int ssz = snprintf(buf, len,
                           "[%08x %08x] %uK, %u-way, %u-byte line, %s",
                           l2ver, l2dscr,
                           size_kb, c.ways, c.line_size,
                           (c.cpu_num > 1 ? "shared" : "dedicated")
            );

        if (c.cpu_num > 1 && ssz < len)
            ssz += snprintf(buf + ssz, len - ssz, " (%u cores)", c.cpu_num);

        if (ssz < len)
            ssz += snprintf(buf + ssz, len - ssz, ", status: 0x%x (%s)",
                            status, state);

        return ssz;
    }
#endif // PLF_L2CTL_BASE
    if (buf && len)
        buf[0] = 0;

    return 0;
}

static inline struct cache_l2 get_l2_cache(unsigned hart)
{
#if PLF_L2CTL_BASE
    volatile uint32_t *l2ctl = (volatile uint32_t*)PLF_L2CTL_BASE;

    unsigned l2ver = l2ctl[L2_CSR_VER_IDX];
    unsigned l2dscr = hart_hls(hart)->l2_descr;
    bool enabled = !!l2ctl[L2_CSR_EN_IDX];

    return (l2ver && enabled) ? parse_cache_l2_info(l2dscr) : (struct cache_l2){0};
#else
    return (struct cache_l2){0};
#endif
}

static inline void show_l2cache_info(unsigned hartn) {
    char ci_buf[128];
    if (scr_l2cache_info(hartn, ci_buf, sizeof(ci_buf)) > 0) {
        printf("L2  %s\n", ci_buf);
    }
}

#if PLF_L3CTL_BASE

static inline struct cache_l3 parse_cache_l3_info(uint64_t l3c_dscr, uint64_t l3b_dscr)
{
    struct cache_l3 res = { 0 };

    res.cpu_num = (l3c_dscr >> L3C_DESCR_CACHE_OFFS_CPU_NUM) & L3C_DESCR_CACHE_MASK_CPU_NUM;
    res.io_num = (l3c_dscr >> L3C_DESCR_CACHE_OFFS_IO_NUM) & L3C_DESCR_CACHE_MASK_IO_NUM;
    res.mem_num = (l3c_dscr >> L3C_DESCR_CACHE_OFFS_MEM_NUM) & L3C_DESCR_CACHE_MASK_MEM_NUM;
    res.banks = (l3c_dscr >> L3C_DESCR_CACHE_OFFS_BANK_NUM) & L3C_DESCR_CACHE_MASK_BANK_NUM;
    res.ways = 1 << ((l3b_dscr >> L3C_DESCR_BANK_OFFS_WAY_NUM) & L3C_DESCR_BANK_MASK_WAY_NUM);
    res.lines = 1 << ((l3b_dscr >> L3C_DESCR_BANK_OFFS_IDX_NUM) & L3C_DESCR_BANK_MASK_IDX_NUM);
    res.line_size = (1 << ((l3b_dscr >> L3C_DESCR_BANK_OFFS_LINE_WIDTH) & L3C_DESCR_BANK_MASK_LINE_WIDTH)) / 8;
    res.size = (res.lines * res.line_size * res.ways * res.banks);
    res.sets = res.banks * res.lines;

    return res;
}

#endif

static inline int scr_l3cache_info(char *buf, unsigned len)
{
#if PLF_L3CTL_BASE
    volatile uint64_t *l3ctl = (volatile uint64_t*)PLF_L3CTL_BASE;

    uint64_t l3ver = l3ctl[L3C_VID_IDX];

    if (l3ver) {
        uint64_t l3c_dscr = l3ctl[L3C_DESCR_CACHE_IDX];
        uint64_t l3b_dscr = l3ctl[L3C_DESCR_BANK_IDX];
        struct cache_l3 c = parse_cache_l3_info(l3c_dscr, l3b_dscr);
        unsigned size_kb, size_mb;
        int ssz;

        size_kb = ((uint64_t) c.size) / 1024;
        size_mb = size_kb / 1024;

        ssz = snprintf(buf, len,
                       "[%016lx %08x %016lx] %u%c, %u-way, %u-byte line",
                       l3ver, (uint32_t) l3c_dscr, l3b_dscr,
                       size_mb ? size_mb : size_kb,
                       size_mb ? 'M' : 'K',  c.ways, c.line_size
            );

        if (ssz < len)
            ssz += snprintf(buf + ssz, len - ssz, " (%u cores)", c.cpu_num);

        if (ssz < len)
            ssz += snprintf(buf + ssz, len - ssz, " (%u IOs)", c.io_num);

        return ssz;
    }
#endif // PLF_L3CTL_BASE
    if (buf && len)
        buf[0] = 0;

    return 0;
}

static inline struct cache_l3 get_l3_cache(void)
{
#if PLF_L3CTL_BASE
    volatile uint64_t *l3ctl = (volatile uint64_t*)PLF_L3CTL_BASE;

    uint64_t l3ver = l3ctl[L3C_VID_IDX];

    if (l3ver) {
        uint64_t l3c_dscr, l3b_dscr;

        l3c_dscr = l3ctl[L3C_DESCR_CACHE_IDX];
        l3b_dscr = l3ctl[L3C_DESCR_BANK_IDX];

        return parse_cache_l3_info(l3c_dscr, l3b_dscr);
    }
#endif
    return (struct cache_l3){0};
}

static inline void show_l3cache_info() {
    char ci_buf[128];
    if (scr_l3cache_info(ci_buf, sizeof(ci_buf)) > 0) {
        printf("L3 %s\n", ci_buf);
    }
}


#endif // __GNUC__
#else // !__ASSEMBLER__

    .altmacro

// assembler macros

// reset L1 and disable caching
.macro cache_reset_nc
#ifdef PLF_CACHE_CFG
    LOCAL cache_reset_done
    /* is cache avail? */
    csrr t0, SCR_CSR_CACHE_DSCR_L1
    beqz t0, cache_reset_done
    // setup caches: disabled, flush&invalidate
    csr_write_imm SCR_CSR_CACHE_GLBL, %(CACHE_GLBL_DISABLE | CACHE_GLBL_INV)
    fence.i
cache_reset_done:
#endif // PLF_CACHE_CFG
.endm

.macro cache_l1_init
#ifdef PLF_CACHE_CFG
    LOCAL cache_l1_init_done
    /* is cache avail? */
    csrr t0, SCR_CSR_CACHE_DSCR_L1
    beqz t0, cache_l1_init_done
    /* setup global region */
    csr_write_imm SCR_CSR_CACHE_GLBL, %(PLF_CACHE_CFG)
    fence.i
cache_l1_init_done:
#endif // PLF_CACHE_CFG
.endm // cache_l1_init

.macro cache_l1l2_flush addr, size
#if PLF_CACHELINE_SIZE
    LOCAL cache_l1l2_flush_done, cache_l1l2_flush_loop
    /* is cache avail? */
    csrr t0, SCR_CSR_CACHE_DSCR_L1
    beqz t0, cache_l1l2_flush_done
cache_l1l2_flush_loop:
    clflush \addr
    addi \size, \size, -PLF_CACHELINE_SIZE
    addi \addr, \addr, PLF_CACHELINE_SIZE
    bgez \size, cache_l1l2_flush_loop
cache_l1l2_flush_done:
#endif // PLF_CACHELINE_SIZE
.endm // cache_l1l2_flush


// init L2$: disable, confirm  state, invalidate, confirm, enable, confirm
.macro cache_l2_init
#ifndef PLF_L2_NOINIT
#if PLF_L2CTL_BASE
    LOCAL l2cache_wait_dis, l2cache_wait_inv, l2cache_wait_en, l2cache_init_done, l2cache_wait_coh_en
    li   t1, PLF_L2CTL_BASE
    lw   t0, L2_CSR_VER_OFFS(t1)
    beqz t0, l2cache_init_done
    // calc L2banks' mask
    lw   t2, L2_CSR_DESCR_OFFS(t1)
    srli t2, t2, 16
    andi t2, t2, 0xf
    addi t2, t2, 1
    li   t0, 1
    sll  t2, t0, t2
    addi t2, t2, -1

    // disable all banks
    sw   zero, L2_CSR_EN_OFFS(t1)
l2cache_wait_dis:
    fence
    lw   t0, L2_CSR_BUSY_OFFS(t1)
    bnez t0, l2cache_wait_dis

    // invalidate all banks
    sw   t2, L2_CSR_INV_OFFS(t1)
l2cache_wait_inv:
    lw   t0, L2_CSR_BUSY_OFFS(t1)
    bnez t0, l2cache_wait_inv

#if PLF_L3CTL_BASE
    // enable external coherency only if there is L3
#ifndef PLF_L2C_DISABLE
    li   t0, 1
#else
    li   t0, 0
#endif // PLF_L2C_DISABLE
    sw   t0, L2_CSR_SYSCO_OFFS(t1)
l2cache_wait_coh_en:
    lw   t0, L2_CSR_BUSY_OFFS(t1)
    bnez t0, l2cache_wait_coh_en
#endif // PLF_L3CTL_BASE

#ifndef PLF_L2C_DISABLE
    // enable all banks
    sw   t2, L2_CSR_EN_OFFS(t1)
l2cache_wait_en:
    lw   t0, L2_CSR_BUSY_OFFS(t1)
    bnez t0, l2cache_wait_en
#endif // PLF_L2C_DISABLE
    fence.i
l2cache_init_done:
#endif // PLF_L2CTL_BASE
#endif // PLF_L2_NOINIT
.endm // cache_l2_init

// init L3$: invalidate, confirm
.macro cache_l3_init
#if PLF_L3CTL_BASE
    LOCAL l3cache_wait_inv, l3cache_wait_inv_loop, l3cache_init_done
    li   t1, PLF_L3CTL_BASE
    ld   t0, L3C_VID_OFFS(t1)
    beqz t0, l3cache_init_done
    // invalidate all banks
    li   t2, L3C_CMD_INVALIDATE_ALL
    sd   t2, L3C_CMD_CTRL_OFFS(t1)
    // polling in the loop for a done signal on L3B_CMD_STS register for each bank
l3cache_wait_inv:
    li   t1, PLF_L3CTL_BASE
    ld   t2, L3C_DESCR_CACHE_OFFS(t1)
    srli t2, t2, L3C_DESCR_CACHE_OFFS_BANK_NUM
    andi t2, t2, L3C_DESCR_CACHE_MASK_BANK_NUM
    li   t1, L3B_PRIVATE_0_BANK
l3cache_wait_inv_loop:
    ld   t0, L3B_CMD_STS_OFFS(t1)
    andi t0, t0, L3B_CMD_STS_MASK_DONE
    beqz t0, l3cache_wait_inv_loop
    addi t2, t2, -1
    addi t1, t1, L3B_STEP
    bnez t2, l3cache_wait_inv_loop
l3cache_init_done:
#endif // PLF_L3CTL_BASE
.endm // cache_l3_init

// read L2$ description
.macro cache_l2_descr dst_reg
#if PLF_L2CTL_BASE
    LOCAL l2cache_descr_done
    li   \dst_reg, PLF_L2CTL_BASE
    lw   \dst_reg, L2_CSR_VER_OFFS(\dst_reg)
    beqz \dst_reg, l2cache_descr_done
    li   \dst_reg, PLF_L2CTL_BASE
    lw   \dst_reg, L2_CSR_DESCR_OFFS(\dst_reg)
l2cache_descr_done:
#else
    li \dst_reg, 0
#endif // PLF_L2CTL_BASE
.endm // cache_l2_init


#endif // !__ASSEMBLER__
#endif // SCR_INFRA_CACHE_H
