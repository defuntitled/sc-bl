/*
 * Copyright (C) 2015, Syntacore Ltd.
 * All Rights Reserved.
 */

/*
 * Copyright (c) 2013, The Regents of the University of California (Regents).
 * All Rights Reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Regents nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
 * SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
 * OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
 * BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
 * HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE
 * MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#include "platform_config.h"
#include "vm.h"
#include "hls.h"
#include "cache.h"
#include "csr.h"
#include "ipic.h"
#include "plic.h"
#include "uart.h"
#include "rtc.h"
#include "smp.h"
#include "mmu.h"
#if PLF_PMP_SUPPORT
#include "pmp.h"
#endif // PLF_PMP_SUPPORT
#include "cluster_control.h"
#if PLF_MRT_SUPPORT
#include "mrt.h"
#endif // PLF_MRT_SUPPORT
#include "con_utils.h"
#include "pmu.h"
#include "l2_pmu.h"
#include "l3_pmu.h"
#include "aplic.h"
#include "imsic.h"
#include "cpu_features.h"
#include "stringify.h"
#include "reg_detect.h"
#include "clk.h"

#include <stdio.h>

#if PLF_SMP_SUPPORT
volatile unsigned num_harts = 1;
#if !PLF_SMP_ICCM_SUPPORT
volatile unsigned slot_bits_shift = 0;
#endif
volatile bool smp_l1_en;
#if PLF_CACHE_L2_DEDICATED
volatile bool smp_l2_en;
#endif // PLF_CACHE_L2_DEDICATED
#endif // PLF_SMP_SUPPORT

#define MIP_MMFIP 0x00008000 /* memory fault */

//----------------------
// For MRT and PMP, the last entry in this table is the most prioritized
// (during PMP configuration entries are written in reverse order to match the
// hardware policy). For MPU, attributes for overlapping chunks are merged
// into the least permissive ones
//----------------------
#if defined(PLF_MEM_MAP)
const scr_mem_region_info mem_regions[] = {
    PLF_MEM_MAP,
};
const unsigned mem_region_count = ARRAY_SIZE(mem_regions);
#endif // PLF_MEM_MAP

#if defined(PLF_MEM_MAP_OPT)
const scr_mem_region_info mem_regions_opt[] = {
    PLF_MEM_MAP_OPT
};
#endif // PLF_MEM_MAP_OPT

extern bool enable_dbg_out;

void plf_hart_mem_init(void)
{
#if PLF_MPU_SUPPORT
#if defined(PLF_MEM_MAP_OPT)
    scr_mpu_setup(mem_regions_opt, ARRAY_SIZE(mem_regions_opt));
#elif defined(PLF_MEM_MAP)
    scr_mpu_setup(mem_regions, ARRAY_SIZE(mem_regions));
#endif // PLF_MEM_MAP_OPT || PLF_MEM_MAP
#endif // PLF_MPU_SUPPORT
#if PLF_PMP_SUPPORT
#if defined(PLF_MEM_MAP_OPT)
    setup_pmp_regions(mem_regions_opt, ARRAY_SIZE(mem_regions_opt));
#elif defined(PLF_MEM_MAP)
    setup_pmp_regions(mem_regions, ARRAY_SIZE(mem_regions));
#endif // PLF_MEM_MAP_OPT || PLF_MEM_MAP
#endif // PLF_PMP_SUPPORT

#if PLF_MRT_SUPPORT
#if defined(PLF_MEM_MAP_OPT)
    setup_mrt_regions(mem_regions_opt, ARRAY_SIZE(mem_regions_opt));
#elif defined(PLF_MEM_MAP)
    setup_mrt_regions(mem_regions, ARRAY_SIZE(mem_regions));
#else
    scr_mrt_default_setup();
#endif // PLF_MEM_MAP_OPT || PLF_MEM_MAP

#endif // PLF_MRT_SUPPORT

#if PLF_TCM_ECC_INIT
    for (uintptr_t p = PLF_TCM_BASE; p < PLF_TCM_BASE + PLF_TCM_SIZE; p += sizeof(long))
        *(volatile long *)p = 0;
#endif // PLF_TCM_ECC_INIT
}

void plf_mem_init(void)
{
    plf_hart_mem_init();

#if PLF_SHMEM_ECC_INIT
    for (uintptr_t p = PLF_SHMEM_BASE; p < PLF_SHMEM_BASE + PLF_SHMEM_SIZE; p += sizeof(long))
        *(volatile long *)p = 0;
#endif // PLF_TCM_ECC_INIT

    extern char __reldata_start[] __attribute__((weak));
    extern char __reldata_end[] __attribute__((weak));
    extern char __reldata_load_start[] __attribute__((weak));

    if (&__reldata_start[0] != &__reldata_load_start[0]) {
        memcpy(__reldata_start, __reldata_load_start, __reldata_end - __reldata_start);
    }
}

#define HLS_CPUID_INIT 0xdeabbabe
#define HLS_HARTID_INIT 0xdeadbeef

void hls_init(unsigned hartn, unsigned ipin, unsigned int active)
{
    size_t tdata_size = _tdata_end - _tdata_begin;
    size_t tbss_size = _tbss_end - _tdata_end;
    void *other_tls = hart_tls(hartn);
    hls_t *hls = hart_hls(hartn);

    // init TLS
    memcpy(other_tls, _tdata_begin, tdata_size);
    memset(other_tls + tdata_size, 0, tbss_size);
    // init HLS
    hls->ipi_n = ipin;
    hls->active = active;
    hls->cpuid = HLS_CPUID_INIT;
    hls->hartid = HLS_HARTID_INIT;
    hls->state = SBI_HSM_STATE_STOP;
    cache_flush(hls, sizeof(hls_t));
    cache_flush(other_tls, tdata_size + tbss_size);
}

void hart_fill_hls(void)
{
    hls_t *hls = HLS();
    hls->hartid = arch_hartid();
    hls->cpuid = arch_cpuid();
    hls->state = SBI_HSM_STATE_START_PENDING;
#ifdef PLF_CACHE_CFG
    hls->l1i_info = CACHE_INFO_L1I;
    hls->l1d_info = CACHE_INFO_L1D;
#endif // PLF_CACHE_CFG
#if PLF_L2CTL_BASE
    hls->l2_descr = scr_l2cache_description();
#endif // PLF_L2CTL_BASE
#if PLF_SMP_NON_COHERENT
    cache_flush(hls, sizeof(*hls));
#endif // PLF_SMP_NON_COHERENT
}

void plf_init(void)
{
    plf_mem_init();

#ifdef PLF_CLUSTER_CFG_BASE
    plf_cluster_cpu_init();
#endif

    /* init BSS */
    extern char __BSS_START__[];
    extern char __BSS_END__[];
    memset(__BSS_START__, 0, __BSS_END__ - __BSS_START__);

    /* init master (logical hart#0) TLS/HLS */
#if PLF_SMP_SUPPORT && PLF_SMP_ICCM_SUPPORT
    hls_init(0, iccm_get_own_id(), 1);
#else
    hls_init(0, PLF_MASTER_HART, 1);
#endif
    hart_fill_hls();

    clk_header_init();
}

uintptr_t mem_size;
volatile pte_t* root_page_table = 0;

static void hart_init(void)
{
    write_csr(mstatus, 0);
    // Enable software interrupts
    write_csr(mie, (1 << 3)/*MIP_MSIP*/);
    // Enable perf counters
    write_csr(mcounteren, -1);
#if PLF_PMU_SUPPORT
    // Start cycle/instret counters on boot
    write_csr(CSR_MCOUNTINHIBIT, PMU_DFLT_EN_MINH(-1));
#endif

#if PLF_MMODE_ONLY == 0
    // setup supervisor mode
    if (supports_extension('S')) {
        // Enable user/supervisor to use of perf counters
        write_csr(scounteren, -1);
        // Disable paging
        write_csr(satp, 0);

        // setup delegations
        uintptr_t interrupts = (1 << 1) | (1 << 5) | (1 << 9) | (1 << 13); //MIP_SSIP | MIP_STIP | MIP_SEIP | MIP_LCOFIP;

        uintptr_t exceptions = 0
            | (1U << 0 /* MISALIGNED_FETCH */)
            | (1U << 2 /* ILLEGAL_INSTRUCTION */)
            | (1U << 3 /* BREAKPOINT */)
#if 0 // misaligned load/store/amo software emulation is supported
            | (1U << 4 /* MISALIGNED_LOAD */)
            | (1U << 6 /* MISALIGNED_STORE_AMO */)
#endif // 0/1
            | (1U << 8 /* USER_ECALL */)
            | (1U << 12 /* PAGE_FAULT_FETCH */)
            | (1U << 13 /* PAGE_FAULT_LOAD */)
            | (1U << 15 /* PAGE_FAULT_STORE */)
            ;

        // setup hypervisor mode (requires supervisor mode supported)
        if (supports_extension('H')) {
            exceptions |= 0
                | (1U << 10 /* VS_ECALL */ )
                | (1U << 20 /* G_FETCH_PAGE */ )
                | (1U << 21 /* G_LOAD_PAGE */)
                | (1U << 22 /* G_VIRT_INST */)
                | (1U << 23 /* G_STORE_PAGE */)
                ;
        }

        write_csr(mideleg, interrupts);
        write_csr(medeleg, exceptions);

        // setup STVEC, SIE, SIP
        write_csr(stvec, 0);
        write_csr(sie, 0);
        write_csr(sip, 0);

#if PLF_RVA22_SUPPORT
        uint64_t menvcfg_val = read_csr(CSR_MENVCFG);

        /* menvcfg is WARL, so enable everything (zicbom/zicboz/svpbmte/sstc) */
        menvcfg_val |= 0
            | (CSR_MENVCFG_CBIE_INV << CSR_MENVCFG_CBIE_SHIFT)
            | (1UL << CSR_MENVCFG_CBCFE_SHIFT)
            | (1UL << CSR_MENVCFG_CBZE_SHIFT)
            | (1UL << CSR_MENVCFG_PBMTE_SHIFT)
            | (1UL << CSR_MENVCFG_STCE_SHIFT);

        write_csr(CSR_MENVCFG, menvcfg_val);

        uint64_t mstateen_val = read_csr(CSR_MSTATEEN0);

        /* mstateen is WARL, allow access to s-hcontext/s-henvcfg/AIA/s-hstateen */
        mstateen_val |= 0
            | (1UL << CSR_MSTATEEN0_CONTEXT_SHIFT)
            | (1UL << CSR_MSTATEEN0_HSENVCFG_SHIFT)
#if PLF_AIA_SUPPORT
            | (1UL << CSR_MSTATEEN0_AIA_SHIFT)
            | (1UL << CSR_MSTATEEN0_IMSIC_SHIFT)
            | (1UL << CSR_MSTATEEN0_SVSLCT_SHIFT)
#endif
            | (1UL << CSR_MSTATEEN_STATEN_SHIFT);

        write_csr(CSR_MSTATEEN0, mstateen_val);
#endif /* PLF_RVA22_SUPPORT */
    }
#endif

#if PLF_MISALIGN_ACCESS_SUPPORT
    plf_set_misalign();
#endif /* PLF_MISALIGN_ACCESS_SUPPORT */

#if PLF_PMU_SUPPORT
    pmu_hart_init();
#endif

#if PLF_L2_PMU_SUPPORT
    scr_fdt_l2_pmu_init();
#endif
#if PLF_L3_PMU_SUPPORT
    scr_fdt_l3_pmu_init();
#endif
}

static void fp_init(void)
{
    if (supports_extension('F')) {
        // set FS=1 (init)
        unsigned long ms = read_csr(mstatus) & ~(3 << 13);
        ms |= (1 << 13);
        write_csr(mstatus, ms);
        write_csr(fcsr, 0);
#ifdef PLF_INIT_REGS
        asm ("fcvt.d.wu f0, zero");
        asm ("fcvt.d.wu f1, zero");
        asm ("fcvt.d.wu f2, zero");
        asm ("fcvt.d.wu f3, zero");
        asm ("fcvt.d.wu f4, zero");
        asm ("fcvt.d.wu f5, zero");
        asm ("fcvt.d.wu f6, zero");
        asm ("fcvt.d.wu f7, zero");
        asm ("fcvt.d.wu f8, zero");
        asm ("fcvt.d.wu f9, zero");
        asm ("fcvt.d.wu f10, zero");
        asm ("fcvt.d.wu f11, zero");
        asm ("fcvt.d.wu f12, zero");
        asm ("fcvt.d.wu f13, zero");
        asm ("fcvt.d.wu f14, zero");
        asm ("fcvt.d.wu f15, zero");
        asm ("fcvt.d.wu f16, zero");
        asm ("fcvt.d.wu f17, zero");
        asm ("fcvt.d.wu f18, zero");
        asm ("fcvt.d.wu f19, zero");
        asm ("fcvt.d.wu f20, zero");
        asm ("fcvt.d.wu f21, zero");
        asm ("fcvt.d.wu f22, zero");
        asm ("fcvt.d.wu f23, zero");
        asm ("fcvt.d.wu f24, zero");
        asm ("fcvt.d.wu f25, zero");
        asm ("fcvt.d.wu f26, zero");
        asm ("fcvt.d.wu f27, zero");
        asm ("fcvt.d.wu f28, zero");
        asm ("fcvt.d.wu f29, zero");
        asm ("fcvt.d.wu f30, zero");
        asm ("fcvt.d.wu f31, zero");
#endif // PLF_INIT_REGS
    }
}

static void plf_irq_map(void)
{
#if PLF_AIA_SUPPORT == 0
#ifndef PLF_PLIC_BASE
    // preconfigure IPIC
#ifdef PLF_IPIC_IRQ_MAP
    // mapping
    static const unsigned VECTOR_IRQ_MAP[] = { PLF_IPIC_IRQ_MAP };
    for (unsigned i = 0; i < ARRAY_SIZE(VECTOR_IRQ_MAP); ++i) {
        if (VECTOR_IRQ_MAP[i] != ~0)
            irq_setup(i, VECTOR_IRQ_MAP[i], IRQ_PRIV_SMODE, IRQ_TYPE_LEVEL_HIGH);
    }
    if (arch_hartid() == PLF_MASTER_HART && enable_dbg_out) {
        printk("init: IPIC IRQ mapping (vector:line):");
        for (unsigned i = 0; i < ARRAY_SIZE(VECTOR_IRQ_MAP); ++i) {
            if (VECTOR_IRQ_MAP[i] != ~0)
                printk(" %02d:%02d", i, VECTOR_IRQ_MAP[i]);
        }
        printk("\n");
    }
#else // PLF_IPIC_IRQ_MAP
    if (arch_hartid() == PLF_MASTER_HART && enable_dbg_out)
        printk("init: IPIC IRQ mapping is not configured\n");
#endif // PLF_IPIC_IRQ_MAP
#else // !PLF_PLIC_BASE
    // PLIC initialization
    // only master HART
    if (arch_hartid() != PLF_MASTER_HART)
        return;

    plic_init(enable_dbg_out);

#endif // !PLF_PLIC_BASE
#else // PLF_AIA_SUPPORT
	if (arch_hartid() == PLF_MASTER_HART) {
		aplic_init();
	}
	imsic_init();
#endif
}

#ifdef PLF_IPIC_IRQ_MAP_SECONDARY
static void plf_irq_map_secondary(void)
{
    // preconfigure IPIC for secondary cores
    // mapping
    static const unsigned VECTOR_IRQ_MAP[] = { PLF_IPIC_IRQ_MAP_SECONDARY };
    for (unsigned i = 0; i < ARRAY_SIZE(VECTOR_IRQ_MAP); ++i) {
        if (VECTOR_IRQ_MAP[i] != ~0)
            irq_setup(i, VECTOR_IRQ_MAP[i], IRQ_PRIV_SMODE, IRQ_TYPE_LEVEL_HIGH);
    }
}
#else
#define plf_irq_map_secondary plf_irq_map
#endif

#if defined(PLF_DTS_FILE)
#define DTB_KERNEL_ADDR_OFFS (4*1024*1024)
#ifdef PLF_CMEM_SIZE
#define DTB_KERNEL_COPY_OFFS (PLF_MEM_SIZE - PLF_CMEM_SIZE - DTB_KERNEL_ADDR_OFFS)
#else
#define DTB_KERNEL_COPY_OFFS (PLF_MEM_SIZE - DTB_KERNEL_ADDR_OFFS)
#endif
// DTB must exists in kernel logical address space: PAGE_OFFSET ... 0xFF...FFFF
// for RV32: 1G (0x40000000)
// for RV64 cmodel low: 2G (0x8000_0000)
// for RV64 cmodel any: 128G (0x20_0000_0000)
// check it and move dtb copy addr into this region
#if (DTB_KERNEL_COPY_OFFS + DTB_KERNEL_ADDR_OFFS) > KERNEL_VIRT_SIZE
#undef DTB_KERNEL_COPY_OFFS
#define DTB_KERNEL_COPY_OFFS (KERNEL_VIRT_SIZE - DTB_KERNEL_ADDR_OFFS)
#endif // DTB_KERNEL_COPY_OFFS > KERNEL_VIRT_SIZE
#define DTB_KERNEL_COPY_ADDR (PLF_MEM_BASE + DTB_KERNEL_COPY_OFFS)
#endif /* PLF_DTS_FILE */

static void enter_entry_point(void)
{
    // set MPP=M/S, MPIE=0, SIE=0, SPP=U
    unsigned long ms = read_csr(mstatus) & ~((3 << 11) | (1 << 8) | (1 << 7) | (1 << 1));
#if PLF_MMODE_ONLY
    ms |= (3 << 11); // MPP=M
#else
    ms |= (1 << 11); // MPP=S
#endif // PLF_MMODE_ONLY
    write_csr(mstatus, ms);
#ifndef PLF_KERNEL_ENTRY
    extern uintptr_t kernel_entry;
#if PLF_SMP_NON_COHERENT
    cache_invalidate(&kernel_entry, sizeof(kernel_entry));
#endif // PLF_SMP_NON_COHERENT
    write_csr(mepc, kernel_entry);
#else
    write_csr(mepc, PLF_KERNEL_ENTRY);
#endif
#if PLF_HSM_SUPPORT
    if (!logical_hart()) {
        extern volatile unsigned long harts_online;
        harts_online |= (1 << logical_hart());
#endif // PLF_HSM_SUPPORT
        register uintptr_t a0 asm ("a0") = logical_hart();
        register uintptr_t a1 asm ("a1") = (uintptr_t)
#if defined(PLF_DTS_FILE)
#ifdef DTB_KERNEL_COPY_ADDR
        DTB_KERNEL_COPY_ADDR
#else
        &dt_blob_start
#endif /* DTB_KERNEL_COPY_ADDR */
#else /* PLF_DTS_FILE */
        0
#endif /* PLF_DTS_FILE */
        ;
        asm volatile ("mret" : : "r" (a0), "r" (a1));
#if PLF_HSM_SUPPORT
    } else {
        // HSM variant
        extern volatile sbi_hsm_start_regs hsm_start_arr [];
        volatile sbi_hsm_start_regs *hsm = &hsm_start_arr[logical_hart()];
        uintptr_t hartid;
        uintptr_t opaque;

        // Waiting for HSM start command
        while (!hsm->initialized) {
            cpu_relax();
        }
        // Implementing parameters from HSM
        write_csr(mepc, hsm->start_addr);
        hartid = hsm->hartid;
        opaque = hsm->opaque;
        // Cleaning table for next HSM startup
        hsm->hartid = 0;
        hsm->opaque = 0;
        hsm->start_addr = 0;
        hsm->initialized = 0;

        extern volatile unsigned long harts_online;
        harts_online |= (1 << logical_hart());

        register uintptr_t a0 asm ("a0") = hartid;
        register uintptr_t a1 asm ("a1") = opaque;
        // memory to make sure that all memory operations are finished
        asm volatile ("mret" : : "r" (a0), "r" (a1) : "memory");
    }
#endif // PLF_HSM_SUPPORT
    __builtin_unreachable();
}

#if defined(PLF_DTS_FILE)
static void dtb_freq(uint8_t* dst, unsigned long clk) {
		dst[0] = ((unsigned)clk >> 24) & 0xff;
		dst[1] = ((unsigned)clk >> 16) & 0xff;
		dst[2] = ((unsigned)clk >> 8) & 0xff;
		dst[3] = (unsigned)clk & 0xff;
}
#endif

static void init_dtb(void)
{
#if defined(PLF_DTS_FILE)
    static const char DTB_STR_OKAY[] = "okay";
    static const char DTB_STR_FAIL[] = "fail";
    static const unsigned DTB_STR_OKAY_SZ = sizeof(DTB_STR_OKAY);
    static const unsigned DTB_STR_FAIL_SZ = sizeof(DTB_STR_FAIL);

    extern char cpu0_status[8] __attribute__((weak));
    extern char cpu1_status[8] __attribute__((weak));
    extern char cpu2_status[8] __attribute__((weak));
    extern char cpu3_status[8] __attribute__((weak));
    extern char cpu4_status[8] __attribute__((weak));
    extern char cpu5_status[8] __attribute__((weak));
    extern char cpu6_status[8] __attribute__((weak));
    extern char cpu7_status[8] __attribute__((weak));
    extern char cpu8_status[8] __attribute__((weak));
    extern char cpu9_status[8] __attribute__((weak));
    extern char cpu10_status[8] __attribute__((weak));
    extern char cpu11_status[8] __attribute__((weak));
    extern char cpu12_status[8] __attribute__((weak));
    extern char cpu13_status[8] __attribute__((weak));
    extern char cpu14_status[8] __attribute__((weak));
    extern char cpu15_status[8] __attribute__((weak));
    char *hart_status_str[] = {
        cpu0_status,
        cpu1_status,
        cpu2_status,
        cpu3_status,
        cpu4_status,
        cpu5_status,
        cpu6_status,
        cpu7_status,
        cpu8_status,
        cpu9_status,
        cpu10_status,
        cpu11_status,
        cpu12_status,
        cpu13_status,
        cpu14_status,
        cpu15_status
    };

    extern uint8_t cpu0id[4] __attribute__((weak));
    extern uint8_t cpu1id[4] __attribute__((weak));
    extern uint8_t cpu2id[4] __attribute__((weak));
    extern uint8_t cpu3id[4] __attribute__((weak));
    extern uint8_t cpu4id[4] __attribute__((weak));
    extern uint8_t cpu5id[4] __attribute__((weak));
    extern uint8_t cpu6id[4] __attribute__((weak));
    extern uint8_t cpu7id[4] __attribute__((weak));
    extern uint8_t cpu8id[4] __attribute__((weak));
    extern uint8_t cpu9id[4] __attribute__((weak));
    extern uint8_t cpu10id[4] __attribute__((weak));
    extern uint8_t cpu11id[4] __attribute__((weak));
    extern uint8_t cpu12id[4] __attribute__((weak));
    extern uint8_t cpu13id[4] __attribute__((weak));
    extern uint8_t cpu14id[4] __attribute__((weak));
    extern uint8_t cpu15id[4] __attribute__((weak));

    uint8_t *cpuid_dt[] = {
        cpu0id,
        cpu1id,
        cpu2id,
        cpu3id,
        cpu4id,
        cpu5id,
        cpu6id,
        cpu7id,
        cpu8id,
        cpu9id,
        cpu10id,
        cpu11id,
        cpu12id,
        cpu13id,
        cpu14id,
        cpu15id
    };

    extern uint8_t cpu0_freq[4] __attribute__((weak));
    extern uint8_t cpu1_freq[4] __attribute__((weak));
    extern uint8_t cpu2_freq[4] __attribute__((weak));
    extern uint8_t cpu3_freq[4] __attribute__((weak));
    extern uint8_t cpu4_freq[4] __attribute__((weak));
    extern uint8_t cpu5_freq[4] __attribute__((weak));
    extern uint8_t cpu6_freq[4] __attribute__((weak));
    extern uint8_t cpu7_freq[4] __attribute__((weak));
    extern uint8_t cpu8_freq[4] __attribute__((weak));
    extern uint8_t cpu9_freq[4] __attribute__((weak));
    extern uint8_t cpu10_freq[4] __attribute__((weak));
    extern uint8_t cpu11_freq[4] __attribute__((weak));
    extern uint8_t cpu12_freq[4] __attribute__((weak));
    extern uint8_t cpu13_freq[4] __attribute__((weak));
    extern uint8_t cpu14_freq[4] __attribute__((weak));
    extern uint8_t cpu15_freq[4] __attribute__((weak));

    uint8_t *hart_freq[] = {
        cpu0_freq,
        cpu1_freq,
        cpu2_freq,
        cpu3_freq,
        cpu4_freq,
        cpu5_freq,
        cpu6_freq,
        cpu7_freq,
        cpu8_freq,
        cpu9_freq,
        cpu10_freq,
        cpu11_freq,
        cpu12_freq,
        cpu13_freq,
        cpu14_freq,
        cpu15_freq
    };

    extern uint8_t timebase_freq[4] __attribute__((weak));
    // BigEndian representation of CPU "timebase-frequency" integer constant
    static const uint8_t DTB_CPU_TIMEBASE_FREQ[4] = {
#ifdef PLF_MTIMER_BASE
        ((unsigned)PLF_RTC_TIMEBASE >> 24) & 0xff,
        ((unsigned)PLF_RTC_TIMEBASE >> 16) & 0xff,
        ((unsigned)PLF_RTC_TIMEBASE >> 8) & 0xff,
        (unsigned)PLF_RTC_TIMEBASE & 0xff
#else
        0,0,0,0
#endif
    };

    unsigned long plf_rtc_clk = clk_mtimer_get().clk;
    extern uint8_t mtimer_freq[4] __attribute__((weak));
    uint8_t DTB_MTIMER_FREQ[4] = {
#ifdef PLF_MTIMER_BASE
        ((unsigned)plf_rtc_clk >> 24) & 0xff,
        ((unsigned)plf_rtc_clk >> 16) & 0xff,
        ((unsigned)plf_rtc_clk >> 8) & 0xff,
        (unsigned)plf_rtc_clk & 0xff
#else
        0,0,0,0
#endif
    };

    extern uint8_t dev0_freq[4] __attribute__((weak));
    extern uint8_t dev1_freq[4] __attribute__((weak));
    extern uint8_t dev2_freq[4] __attribute__((weak));
    extern uint8_t dev3_freq[4] __attribute__((weak));
    extern uint8_t dev4_freq[4] __attribute__((weak));
    extern uint8_t dev5_freq[4] __attribute__((weak));
    extern uint8_t dev6_freq[4] __attribute__((weak));
    extern uint8_t dev7_freq[4] __attribute__((weak));
    extern uint8_t dev8_freq[4] __attribute__((weak));
    extern uint8_t dev9_freq[4] __attribute__((weak));
    extern uint8_t dev10_freq[4] __attribute__((weak));
    extern uint8_t dev11_freq[4] __attribute__((weak));
    extern uint8_t dev12_freq[4] __attribute__((weak));
    extern uint8_t dev13_freq[4] __attribute__((weak));
    extern uint8_t dev14_freq[4] __attribute__((weak));
    extern uint8_t dev15_freq[4] __attribute__((weak));

    uint8_t *dev_freq[] = {
        dev0_freq,
        dev1_freq,
        dev2_freq,
        dev3_freq,
        dev4_freq,
        dev5_freq,
        dev6_freq,
        dev7_freq,
        dev8_freq,
        dev9_freq,
        dev10_freq,
        dev11_freq,
        dev12_freq,
        dev13_freq,
        dev14_freq,
        dev15_freq
    };
    // BigEndian representation of various "clock-frequency"-like integer constants
    unsigned long plf_sys_clk = clk_sys_get();
    uint8_t DTB_DEV_FREQ[4];
    dtb_freq(DTB_DEV_FREQ, plf_sys_clk);

    // set timebase freq value
    if (timebase_freq != NULL) {
#ifdef PLF_MTIMER_BASE
        if (enable_dbg_out)
            printk("DTB: set \"timebase-frequency\" to %u\n", PLF_RTC_TIMEBASE);
#endif
        memcpy(timebase_freq, DTB_CPU_TIMEBASE_FREQ, 4);
    }

    // set mtimer freq value
    if (mtimer_freq != NULL) {
#ifdef PLF_MTIMER_BASE
        if (enable_dbg_out)
            printk("DTB: set mtimer \"clock-frequency\" to %lu\n", plf_rtc_clk);
#endif
        memcpy(mtimer_freq, DTB_MTIMER_FREQ, 4);
    }

    extern uint8_t dtb_mem_base[8] __attribute__((weak));
    extern uint8_t dtb_mem_size[8] __attribute__((weak));
    // set memory configuration
    if (dtb_mem_base != NULL && dtb_mem_size != NULL) {
#ifdef PLF_CMEM_SIZE
        unsigned long mem_size = PLF_MEM_SIZE - PLF_CMEM_SIZE;
#else
        unsigned long mem_size = PLF_MEM_SIZE;
#endif
        if (enable_dbg_out)
            printk("DTB: set \"memory\" base %p size %lu MiB\n",
                   (void*)PLF_MEM_BASE, mem_size / (1024*1024));
        const uint8_t DTB_MEM_BASE[] = {
#if __riscv_xlen > 32
            ((unsigned long)PLF_MEM_BASE >> 56) & 0xff,
            ((unsigned long)PLF_MEM_BASE >> 48) & 0xff,
            ((unsigned long)PLF_MEM_BASE >> 40) & 0xff,
            ((unsigned long)PLF_MEM_BASE >> 32) & 0xff,
#else
            0, 0, 0, 0,
#endif
            ((unsigned long)PLF_MEM_BASE >> 24) & 0xff,
            ((unsigned long)PLF_MEM_BASE >> 16) & 0xff,
            ((unsigned long)PLF_MEM_BASE >> 8) & 0xff,
            (unsigned long)PLF_MEM_BASE & 0xff
        };
        const uint8_t DTB_MEM_SIZE[] = {
#if __riscv_xlen > 32
            ((unsigned long)mem_size >> 56) & 0xff,
            ((unsigned long)mem_size >> 48) & 0xff,
            ((unsigned long)mem_size >> 40) & 0xff,
            ((unsigned long)mem_size >> 32) & 0xff,
#else
            0, 0, 0, 0,
#endif
            ((unsigned long)mem_size >> 24) & 0xff,
            ((unsigned long)mem_size >> 16) & 0xff,
            ((unsigned long)mem_size >> 8) & 0xff,
            (unsigned long)mem_size & 0xff
        };
        if (dtb_mem_size - dtb_mem_base > 4) {
            memcpy(dtb_mem_base, DTB_MEM_BASE, 8);
            memcpy(dtb_mem_size, DTB_MEM_SIZE, 8);
        } else {
            memcpy(dtb_mem_base, DTB_MEM_BASE + 4, 4);
            memcpy(dtb_mem_size, DTB_MEM_SIZE + 4, 4);
        }
    }

#if PLF_DDR_EXT
    extern uint8_t dtb_mem1_base[8] __attribute__((weak));
    extern uint8_t dtb_mem1_size[8] __attribute__((weak));
    if (dtb_mem1_base != NULL && dtb_mem1_size != NULL) {
        if (enable_dbg_out)
            printk("DTB: set \"memory bank #2\" base %p size %llu MiB\n",
                   (void*)PLF_MEM1_BASE, PLF_MEM1_SIZE / (1024*1024));

        const uint8_t DTB_MEM1_BASE[] = {
#if __riscv_xlen > 32
            ((unsigned long)PLF_MEM1_BASE >> 56) & 0xff,
            ((unsigned long)PLF_MEM1_BASE >> 48) & 0xff,
            ((unsigned long)PLF_MEM1_BASE >> 40) & 0xff,
            ((unsigned long)PLF_MEM1_BASE >> 32) & 0xff,
#else
            0, 0, 0, 0,
#endif
            ((unsigned long)PLF_MEM1_BASE >> 24) & 0xff,
            ((unsigned long)PLF_MEM1_BASE >> 16) & 0xff,
            ((unsigned long)PLF_MEM1_BASE >> 8) & 0xff,
            (unsigned long)PLF_MEM1_BASE & 0xff
        };

        const uint8_t DTB_MEM1_SIZE[] = {
#if __riscv_xlen > 32
            ((unsigned long)PLF_MEM1_SIZE >> 56) & 0xff,
            ((unsigned long)PLF_MEM1_SIZE >> 48) & 0xff,
            ((unsigned long)PLF_MEM1_SIZE >> 40) & 0xff,
            ((unsigned long)PLF_MEM1_SIZE >> 32) & 0xff,
#else
            0, 0, 0, 0,
#endif
            ((unsigned long)PLF_MEM1_SIZE >> 24) & 0xff,
            ((unsigned long)PLF_MEM1_SIZE >> 16) & 0xff,
            ((unsigned long)PLF_MEM1_SIZE >> 8) & 0xff,
            (unsigned long)PLF_MEM1_SIZE & 0xff
        };

        if (dtb_mem1_size - dtb_mem1_base > 4) {
            memcpy(dtb_mem1_base, DTB_MEM1_BASE, 8);
            memcpy(dtb_mem1_size, DTB_MEM1_SIZE, 8);
        } else {
            memcpy(dtb_mem1_base, DTB_MEM1_BASE + 4, 4);
            memcpy(dtb_mem1_size, DTB_MEM1_SIZE + 4, 4);
        }
    }
#endif

    extern uint8_t dtb_dma_mem_base[8] __attribute__((weak));
    extern uint8_t dtb_dma_mem_size[8] __attribute__((weak));
    // set dma memory configuration
    if (dtb_dma_mem_base != NULL && dtb_dma_mem_size != NULL) {
#if PLF_CMEM_SIZE
#if PLF_DMA_COHERENT
#error "Coherent DMA and reserved memory should not be used simultaneously"
#endif
        if (enable_dbg_out)
            printk("DTB: set \"reserved-memory\" base %p size %lu MiB\n",
                   (void*)PLF_CMEM_BASE, (unsigned long)PLF_CMEM_SIZE / (1024*1024));
        const uint8_t DTB_CMEM_BASE[] = {
#if __riscv_xlen > 32
            ((unsigned long)PLF_CMEM_BASE >> 56) & 0xff,
            ((unsigned long)PLF_CMEM_BASE >> 48) & 0xff,
            ((unsigned long)PLF_CMEM_BASE >> 40) & 0xff,
            ((unsigned long)PLF_CMEM_BASE >> 32) & 0xff,
#else
            0, 0, 0, 0,
#endif
            ((unsigned long)PLF_CMEM_BASE >> 24) & 0xff,
            ((unsigned long)PLF_CMEM_BASE >> 16) & 0xff,
            ((unsigned long)PLF_CMEM_BASE >> 8) & 0xff,
            (unsigned long)PLF_CMEM_BASE & 0xff
        };
        const uint8_t DTB_CMEM_SIZE[] = {
#if __riscv_xlen > 32
            ((unsigned long)PLF_CMEM_SIZE >> 56) & 0xff,
            ((unsigned long)PLF_CMEM_SIZE >> 48) & 0xff,
            ((unsigned long)PLF_CMEM_SIZE >> 40) & 0xff,
            ((unsigned long)PLF_CMEM_SIZE >> 32) & 0xff,
#else
            0, 0, 0, 0,
#endif
            ((unsigned long)PLF_CMEM_SIZE >> 24) & 0xff,
            ((unsigned long)PLF_CMEM_SIZE >> 16) & 0xff,
            ((unsigned long)PLF_CMEM_SIZE >> 8) & 0xff,
            (unsigned long)PLF_CMEM_SIZE & 0xff
        };
        if (dtb_dma_mem_size - dtb_dma_mem_base > 4) {
            memcpy(dtb_dma_mem_base, DTB_CMEM_BASE, 8);
            memcpy(dtb_dma_mem_size, DTB_CMEM_SIZE, 8);
        } else {
            memcpy(dtb_dma_mem_base, DTB_CMEM_BASE + 4, 4);
            memcpy(dtb_dma_mem_size, DTB_CMEM_SIZE + 4, 4);
        }
#else // PLF_CMEM_SIZE
        if (enable_dbg_out)
            printk("DTB: set \"reserved-memory\" size 0 MiB\n");
        if (dtb_dma_mem_size - dtb_dma_mem_base > 4) {
            memset(dtb_dma_mem_base, 0, 8);
            memset(dtb_dma_mem_size, 0, 8);
        } else {
            memset(dtb_dma_mem_base, 0, 4);
            memset(dtb_dma_mem_size, 0, 4);
        }
#endif // PLF_CMEM_SIZE
    }

    // set sys clock freq for all devs
    for (unsigned i = 0; i < sizeof(dev_freq) / sizeof(*dev_freq); ++i) {
        if (dev_freq[i] != NULL) {
            if (enable_dbg_out) {
                printk("DTB: set dev#%u clock freq to %lu\n", i, plf_sys_clk);
            }
            memcpy(dev_freq[i], DTB_DEV_FREQ, 4);
        }
    }

    // BigEndian representation of CPU "clock-frequency" integer constant
    uint8_t DTB_CPU_FREQ[4];

    //  TBD
    for (unsigned i = 0; i < num_harts; i++) {
        hls_t *hls = hart_hls(i);
        unsigned long id = hls->ipi_n;
        uint32_t reg = i;

        if (hart_status_str[id] != NULL) {
            if (hls->active && (id < num_harts)) {
                if (enable_dbg_out) {
                    printk("DTB: set cpu#%lu status %p <- \"%s\"\n",
                           id, hart_status_str[id], DTB_STR_OKAY);
                }
                memcpy(hart_status_str[id], DTB_STR_OKAY, DTB_STR_OKAY_SZ);
            } else {
                memcpy(hart_status_str[id], DTB_STR_FAIL, DTB_STR_FAIL_SZ);
                if (enable_dbg_out) {
                    printk("DTB: set cpu#%lu status to fail\n", id);
                }
            }
        }

        if(hart_freq[id] != NULL) {
            unsigned long hart_clk;
            int clk_ret = clk_hart_get_by_id(hls->hartid, &hart_clk);
            if (clk_ret != SCR_CLK_SUCCESS) {
                printk("DTB: failed to get cpu#%lu clk\n", id);
            } else {
                if (enable_dbg_out) {
                    printk("DTB: set cpu#%lu clock freq to %lu\n", id, hart_clk);
                }

                dtb_freq(DTB_CPU_FREQ, hart_clk);
                memcpy(hart_freq[id], DTB_CPU_FREQ, 4);
            }
        }

        if (cpuid_dt[id] != NULL) {
            uint8_t id_for_cpu[] = {
                (reg >> 24) & 0xff,
                (reg >> 16) & 0xff,
                (reg >>  8) & 0xff,
                (reg >>  0) & 0xff,
            };

            if (enable_dbg_out) {
                printk("DTB: set cpu#%lu reg %p <- \"%lu\"\n",
                       id, cpuid_dt[id], (unsigned long)reg);
            }

            memcpy(cpuid_dt[id], id_for_cpu, 4);
        }

    }

#ifdef PLF_NVRAM_HWMAC
    extern uint8_t eth0_mac[6] __attribute__((weak));
    uint8_t *eth_mac[] = {
        eth0_mac,
    };

    if (eth_mac[0] != NULL) {
        memcpy(eth_mac[0], (uint8_t*)PLF_NVRAM_HWMAC, 6);
        if (enable_dbg_out) {
            printk("DTB: setup eth0 hwaddr: ");
            for (int i = 0; i < 6; ++i)
                printk("%s%02x", (i ? ":" : ""), (unsigned)eth_mac[0][i]);
            printk("\n");
        }
    }
#endif // PLF_NVRAM_HWMAC

    extern char dtb_fw_platform[] __attribute__((weak));
    extern char dtb_fw_impid[] __attribute__((weak));
    extern char dtb_fw_bldid[] __attribute__((weak));
    // set fw info: impid, bldid, platform
    if (dtb_fw_platform != NULL) {
        int sz = strlen(dtb_fw_platform);
        memset(dtb_fw_platform, 0, sz);
        snprintf(dtb_fw_platform, sz + 1,
                 "%s", __xstringify(PLATFORM));
    }
    if (dtb_fw_impid != NULL) {
        int sz = strlen(dtb_fw_impid);
        memset(dtb_fw_impid, 0, sz);
        snprintf(dtb_fw_impid, sz + 1,
                 "%lx", arch_impid());
    }
    if (dtb_fw_bldid != NULL) {
        int sz = strlen(dtb_fw_bldid);
        memset(dtb_fw_bldid, 0, sz);
	// Oleg Terentiev has told, that only 4 bytes are valuable
	// Apply mask to support compatible output
        snprintf(dtb_fw_bldid, sz + 1,
                 "%lx", (unsigned long)clk_buildid_get() & 0xFFFFFFFFUL);
    }

    extern const char scbl_version_str[];
    extern const char scbl_ver_tag_str[];
    extern char dtb_fw_scbl_rev[] __attribute__((weak));
    // set loader info
    if (dtb_fw_scbl_rev != NULL) {
        int sz = strlen(dtb_fw_scbl_rev);
        memset(dtb_fw_scbl_rev, 0, sz);
        snprintf(dtb_fw_scbl_rev, sz + 1,
                 "%s (%s)", scbl_version_str, scbl_ver_tag_str);
    }

#if PLF_SUPPORT_CACHE_INFO
    // Fill cache info
#define DTB_FILL_PER_HART_INFO(label, func, field) ({\
        extern uint8_t label##_0[4] __attribute__((weak)); \
        extern uint8_t label##_1[4] __attribute__((weak)); \
        extern uint8_t label##_2[4] __attribute__((weak)); \
        extern uint8_t label##_3[4] __attribute__((weak)); \
        extern uint8_t label##_4[4] __attribute__((weak)); \
        extern uint8_t label##_5[4] __attribute__((weak)); \
        extern uint8_t label##_6[4] __attribute__((weak)); \
        extern uint8_t label##_7[4] __attribute__((weak)); \
        extern uint8_t label##_8[4] __attribute__((weak)); \
        extern uint8_t label##_9[4] __attribute__((weak)); \
        extern uint8_t label##_10[4] __attribute__((weak)); \
        extern uint8_t label##_11[4] __attribute__((weak)); \
        extern uint8_t label##_12[4] __attribute__((weak)); \
        extern uint8_t label##_13[4] __attribute__((weak)); \
        extern uint8_t label##_14[4] __attribute__((weak)); \
        extern uint8_t label##_15[4] __attribute__((weak)); \
        \
        uint8_t * label [] = {                             \
            label##_0, label##_1, label##_2, label##_3,    \
            label##_4, label##_5, label##_6, label##_7,    \
            label##_8, label##_9, label##_10, label##_11,  \
            label##_12, label##_13, label##_14, label##_15,\
        };                                                 \
        \
        for (int i = 0; i < ARRAY_SIZE(label) && i < num_harts ;i++) { \
            hls_t *hls = hart_hls(i);                      \
            unsigned long id = hls->ipi_n;                 \
            if (hls->active && (id < num_harts)) {         \
                const uint32_t _val = func(i).field;       \
                const uint8_t dtb_val[4] = {               \
                    (_val >> 24) & 0xff,                   \
                    (_val >> 16) & 0xff,                   \
                    (_val >> 8) & 0xff,                    \
                    _val & 0xff };                         \
                if (label[i])                              \
                    memcpy(label[id], dtb_val, 4);         \
	    }                                              \
        }                                                  \
    })

#define DTB_FILL_INFO(label, val) ({                   \
        extern uint8_t label[4] __attribute__((weak)); \
        const uint32_t _val = val;                     \
        const uint8_t dtb_val[4] = {                   \
            (_val >> 24) & 0xff,                       \
            (_val >> 16) & 0xff,                       \
            (_val >> 8) & 0xff,                        \
            _val & 0xff };                             \
        if (label) memcpy(label, dtb_val, 4);          \
    })
    // Cache L1 info
#if PLF_CACHE_CFG
    DTB_FILL_PER_HART_INFO(l1i_sz, get_l1i_cache, size);
    DTB_FILL_PER_HART_INFO(l1i_sets, get_l1i_cache, sets);
    DTB_FILL_PER_HART_INFO(l1i_block_sz, get_l1i_cache, line_size);

    DTB_FILL_PER_HART_INFO(l1d_sz, get_l1d_cache, size);
    DTB_FILL_PER_HART_INFO(l1d_sets, get_l1d_cache, lines);
    DTB_FILL_PER_HART_INFO(l1d_block_sz, get_l1d_cache, line_size);
#endif
    // Cache L2 info
#if PLF_L2CTL_BASE
#if PLF_CACHE_L2_DEDICATED
    DTB_FILL_PER_HART_INFO(l2_sz, get_l2_cache, size);
    DTB_FILL_PER_HART_INFO(l2_sets, get_l2_cache, sets);
    DTB_FILL_PER_HART_INFO(l2_block_sz, get_l2_cache, line_size);
#else
    DTB_FILL_INFO(shared_l2_sz, get_l2_cache(0).size);
    DTB_FILL_INFO(shared_l2_sets, get_l2_cache(0).sets);
    DTB_FILL_INFO(shared_l2_block_sz, get_l2_cache(0).line_size);
#endif
#endif
    // Cache L3 info
#if PLF_L3CTL_BASE
    DTB_FILL_INFO(shared_l3_sz, get_l3_cache().size);
    DTB_FILL_INFO(shared_l3_sets, get_l3_cache().sets);
    DTB_FILL_INFO(shared_l3_block_sz, get_l3_cache().line_size);
#endif

#if defined(PLF_PLIC_BASE) && defined(PLF_CLUSTER_CFG_BASE)
    struct plic_info plic_info;

    plic_msi_get_info(&plic_info);

    DTB_FILL_INFO(plic_msi_irq_en_base, plic_info.en_base);
    DTB_FILL_INFO(plic_msi_irq_en_size, plic_info.en_size);
#endif // defined(PLF_PLIC_BASE) && defined(PLF_CLUSTER_CFG_BASE)

#undef DTB_FILL_INFO
#undef DTB_FILL_PER_HART_INFO
#endif

    extern char dt_blob_start, dt_blob_end;

    unsigned num_dtb_pages = (&dt_blob_end - &dt_blob_start + PGSIZE - 1) / PGSIZE;

    // prepare device tree blob
#ifdef DTB_KERNEL_COPY_ADDR
    /* memcpy((void*)DTB_KERNEL_COPY_ADDR, &dt_blob_start, num_dtb_pages * PGSIZE); */
    memset((void*)DTB_KERNEL_COPY_ADDR, 0xff, num_dtb_pages * PGSIZE);
    memcpy((void*)DTB_KERNEL_COPY_ADDR, &dt_blob_start, &dt_blob_end - &dt_blob_start);
#endif

#if PLF_SMP_NON_COHERENT
    // sync DTB data
#ifdef DTB_KERNEL_COPY_ADDR
    cache_flush((void*)DTB_KERNEL_COPY_ADDR, num_dtb_pages * PGSIZE);
#else
    cache_flush(&dt_blob_start, num_dtb_pages * PGSIZE);
#endif
#endif // PLF_SMP_NON_COHERENT

    if (enable_dbg_out) {
#ifdef DTB_KERNEL_COPY_ADDR
        printk("DTB: %u page(s) @ %p (copied from %p)\n",
               num_dtb_pages, (void*)DTB_KERNEL_COPY_ADDR, &dt_blob_start);
#else
        printk("DTB: %u page(s) @ %p\n", num_dtb_pages, &dt_blob_start);
#endif
    }
#endif // PLF_DTS_FILE
}

#if PLF_SMP_SUPPORT
static void dbg_print_hart_info(unsigned long hartn)
{
    hls_t *hls = hart_hls(hartn);
#if PLF_SMP_NON_COHERENT
    cache_invalidate(hls, sizeof(hls_t));
#endif // PLF_SMP_NON_COHERENT
    printk("SMP: [%02lu] hart#%lu @ slot#%lu: isa %08lx HLS %p\n",
           hartn, hls->hartid, hls->ipi_n, hls->cpuid, hls);
}

// check SMP hardware support (ICCM)
static bool smp_hardware_supported(void)
{
#if PLF_SMP_ICCM_SUPPORT || PLF_CACHE_L2_DEDICATED
    return true;
#else // PLF_CACHE_L2_DEDICATED
    // there is no direct test for ICCM support
    // so will check number of connected cores from L2$ info
    unsigned long l2dscr = scr_l2cache_description();
    return  ((l2dscr >> L2_CSR_DESCR_OFFS_CORES) & L2_CSR_DESCR_MASK_CORES) != 0;
#endif
}

static void init_slaves(void)
{
    // wake up secondary harts
    for (unsigned hartn = 1; hartn < num_harts; ++hartn) {
        unsigned long msg = PACK_SCIPI_MSG(SCR_IPI_CMD_START, 0);
        hls_t *hls = hart_hls(hartn);

        if (!hls->active)
            continue;

        ipi_send_data(hls->ipi_n, msg);
        do {
#if !PLF_TIMEOUTS_OFF
            rtc_delay_us(10);
#endif // !PLF_TIMEOUTS_OFF
#if PLF_SMP_NON_COHERENT
            cache_invalidate(hls, sizeof(*hls));
#endif // PLF_SMP_NON_COHERENT
        } while (((volatile hls_t *)hls)->state != SBI_HSM_STATE_START);
    }
}
#endif // PLF_SMP_SUPPORT

static void start_master_hart(void)
{
    // root_page_table is used as spin lock for slaves
    root_page_table = NULL;
    mb();

    mem_size = PLF_MEM_SIZE;

    hls_t *hls = HLS();
    hls->state = SBI_HSM_STATE_START;

#if PLF_SMP_SUPPORT
    smp_l1_en = cache_l1_enabled();
#if PLF_CACHE_L2_DEDICATED
    smp_l2_en = scr_l2cache_is_enabled();
#endif // PLF_CACHE_L2_DEDICATED

#if PLF_SMP_NON_COHERENT
    // sync shared data
    cache_flush((void*)&root_page_table, sizeof(root_page_table));
    cache_flush(&mem_size, sizeof(mem_size));
    cache_flush((void*)&smp_l1_en, sizeof(smp_l1_en));
    cache_flush(hls, sizeof(hls_t));
#if PLF_CACHE_L2_DEDICATED
    cache_flush((void*)&smp_l2_en, sizeof(smp_l2_en));
#endif // PLF_CACHE_L2_DEDICATED
#endif // PLF_SMP_NON_COHERENT

    init_slaves();
#endif // PLF_SMP_SUPPORT

    init_dtb();

    if (enable_dbg_out) {
        printk("init: impid= %08lx\n", arch_impid());
        printk("init: cpuid= %08lx\n", arch_cpuid());
        printk("init: mtvec= %08lx\n", read_csr(mtvec));
        printk("init: mem_size= %luK (%lu of 4K pages)\n",
               (unsigned long)(mem_size / 1024),
               (unsigned long)(mem_size / PGSIZE));
        printk("init: num_harts= %ld\n", (long)num_harts);
    }

    plf_irq_map();
#ifndef PLF_RUN_QUIET_MODE
    con_print_scr_header();
#endif // PLF_RUN_QUIET_MODE
    scr_rtc_init();

    root_page_table = (pte_t*)~0;
    mb();
#if PLF_SMP_NON_COHERENT
    cache_flush((void*)&root_page_table, sizeof(root_page_table));
#endif // PLF_SMP_NON_COHERENT

    enter_entry_point();
}

static void start_slave_hart(void)
{
#if PLF_SMP_SUPPORT

#if PLF_SMP_NON_COHERENT
    // sync shared data
    cache_invalidate((void*)&enable_dbg_out, sizeof(enable_dbg_out));
#endif // PLF_SMP_NON_COHERENT

    // printk uses spinlock/atomic, call it with enabled caches
    if (enable_dbg_out) {
        dbg_print_hart_info(logical_hart());
    }

#if PLF_SMP_NON_COHERENT
    // sync shared data
    cache_invalidate((void*)&root_page_table, sizeof(root_page_table));
    cache_invalidate(&mem_size, sizeof(mem_size));
    cache_invalidate((void*)&num_harts, sizeof(num_harts));
#if !PLF_SMP_ICCM_SUPPORT
    cache_invalidate((void*)&slot_bits_shift, sizeof(slot_bits_shift));
#endif
    cache_invalidate((void*)&smp_l1_en, sizeof(smp_l1_en));
#if PLF_CACHE_L2_DEDICATED
    cache_invalidate((void*)&smp_l2_en, sizeof(smp_l2_en));
#endif // PLF_CACHE_L2_DEDICATED
#endif // PLF_SMP_NON_COHERENT

#if PLF_CACHE_L2_DEDICATED
    // sync L2$ config
    // it is enabled after start, so disable it if needed
    if (!smp_l2_en) {
        scr_l2cache_disable();
    }
#endif // PLF_CACHE_L2_DEDICATED

    // sync L1$ config
    // it is enabled after start, so disable it if needed
    if (!smp_l1_en) {
        cache_l1_disable();
    }

    // set active state (master hart enumeration procedure)
    HLS()->state = SBI_HSM_STATE_START;
#if PLF_SMP_NON_COHERENT
    cache_flush(HLS(), sizeof(hls_t));
#endif // PLF_SMP_NON_COHERENT

    // wait until virtual memory is enabled
    while (root_page_table == NULL) {
        mb();
#if PLF_SMP_NON_COHERENT
        cache_invalidate((void*)&root_page_table, sizeof(root_page_table));
#endif // PLF_SMP_NON_COHERENT
    }
    mb();

    plf_irq_map_secondary();

    // delay to order HARTs startup sequence
    rtc_delay_us(logical_hart() * 1024);

    enter_entry_point();
#else
    hart_halt();
#endif // PLF_SMP_SUPPORT
}

#if PLF_SMP_SUPPORT
void init_smp(void)
{
    if (!smp_hardware_supported())
        return;

    extern char STACK_SIZE;
    const unsigned max_stack_chunks = ((unsigned long) &STACK_SIZE) / HLSIZE;
    const unsigned max_harts = MIN(PLF_SMP_HART_NUM, max_stack_chunks);
    const unsigned num_slots = ipi_get_num_slots();

#if !PLF_TIMEOUTS_OFF
    // wait for secondary harts to reach secondary_reset
    rtc_delay_us(10000);
#endif // !PLF_TIMEOUTS_OFF

#if !PLF_SMP_ICCM_SUPPORT
    // calculate size of slot's num mask
    unsigned slot_mask_bits = 1;
    for (unsigned long i = ~1; (i & (num_slots - 1)) != 0; i <<= 1)
        ++slot_mask_bits;

    slot_bits_shift = SCR_IPI_RECV_SRC_BITS - slot_mask_bits;
#if PLF_SMP_NON_COHERENT
    cache_flush((void*)&slot_bits_shift, sizeof(slot_bits_shift));
#endif // PLF_SMP_NON_COHERENT
#endif // !PLF_SMP_ICCM_SUPPORT

    // count available active slots
    unsigned avail = 0;
    unsigned slot_mask = 0;

    // init hls for max supported number of slots: see max_stack_chunks
    unsigned hartn = 1;

    for (unsigned long slotn = 0; slotn < num_slots; ++slotn) {
        ipi_data_t msg = PACK_SCIPI_MSG(SCR_IPI_CMD_RESET, 0xdeadbeaf);
        bool slot_active = false;

        ipi_send_data(slotn, msg);

        if (ipi_msg_avail()) {
            // check mailbox for self addr
#ifndef PLF_RUN_QUIET_MODE
            ipi_data_t data = ipi_read_data();
            printk("SMP[%lu]: master hart loopback slot\n", slotn);
            if (data != msg)
                printk("SMP: loopback data mismatch: received %lx, expected %lx\n", data, msg);
#else
            ipi_read_data();
#endif
            slot_active = true;
        } else {
            slot_active = ipi_wait_receiver_avail(slotn, 100000);
#ifndef PLF_RUN_QUIET_MODE
            printk("SMP[%lu]: %s\n", slotn, slot_active ? "active" : "empty");
#endif // PLF_RUN_QUIET_MODE
        }

        if (slot_active) {
            // If the maximum number of harts is exceeded, still proceed the
            // detection of active harts, but without filling HLS for them
            // to avoid out of bound memory accesses
            // HLS of master is initialized separately
            if (slotn < max_harts && slotn != PLF_MASTER_HART)
                hls_init(hartn++, slotn, 1);

	    slot_mask |= BIT(slotn);
	    avail++;
        }
    }

    for (unsigned long slotn = 0; slotn < num_slots; ++slotn) {
        if (slot_mask & BIT(slotn))
            continue;

	if (hartn < max_harts)
            hls_init(hartn++, slotn, 0);
    }

#ifndef PLF_RUN_QUIET_MODE
    printk("SMP: Found %u active %s\n", avail, (avail > 1) ? "harts" : "hart");
#endif // PLF_RUN_QUIET_MODE

    num_harts = hartn;
#if PLF_SMP_NON_COHERENT
    cache_flush((void*)&num_harts, sizeof(num_harts));
#endif // PLF_SMP_NON_COHERENT

    if (hartn > PLF_SMP_HART_NUM) {
        printk("SMP: The number of logical harts must not exceed the supported maximum\n"
               "Maximum allowed: %d, Found: %d\n", PLF_SMP_HART_NUM, hartn);
        hart_halt();
    }

    if (hartn > max_stack_chunks) {
        unsigned long stack_size_k = ((unsigned long) &STACK_SIZE) >> 10;
        printk("SMP: Stack size (%luK) is too small for %d harts. At least %luK needed.\n",
               stack_size_k, hartn, (hartn * PGSIZE) >> 10);
        hart_halt();
    }

    if (avail > hartn ) {
        unsigned long stack_size_k = ((unsigned long) &STACK_SIZE) >> 10;
        printk("SMP: Stack size (%luK) is too small for all %d available harts. At least %luK needed.\n",
               stack_size_k, avail, (avail * PGSIZE) >> 10);
        printk("SMP: Activated only %d harts...\n", hartn);
    }

    // send SETUP to all active slots
    for (unsigned hartn = 1; hartn < num_harts; ++hartn) {
#if PLF_SMP_ICCM_SUPPORT
        ipi_data_t msg = PACK_SCIPI_MSG(SCR_IPI_CMD_SETUP, hartn);
#else // PLF_SMP_ICCM_SUPPORT
        unsigned long hls_next_page = hart_stack_page_top(hartn);
        ipi_data_t msg = PACK_SCIPI_MSG(SCR_IPI_CMD_SETUP, hls_next_page);
#endif // PLF_SMP_ICCM_SUPPORT
        hls_t *hls = hart_hls(hartn);

        if (!hls->active)
            continue;

        ipi_send_data(hls->ipi_n, msg);
        do {
#if !PLF_TIMEOUTS_OFF
            rtc_delay_us(1);
#endif // !PLF_TIMEOUTS_OFF
#if PLF_SMP_NON_COHERENT
            cache_invalidate(hls, sizeof(*hls));
#endif // PLF_SMP_NON_COHERENT
        } while (((volatile hls_t *)hls)->state != SBI_HSM_STATE_START_PENDING);
    }
}
#endif // PLF_SMP_SUPPORT

void machine_init(void)
{
    hart_init();
    fp_init();
    if (arch_hartid() == PLF_MASTER_HART)
        start_master_hart();
    else
        start_slave_hart();
}
