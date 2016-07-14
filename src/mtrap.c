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

/// Syntacore SCR* infra
///
/// @copyright Copyright (C) 2015-2023, Syntacore Ltd. All rights reserved.
/// @author mn-sc
///
/// @brief traps handling

#include "platform_config.h"
#include "vm.h"
#include "hls.h"
#include "arch.h"
#include "spinlock.h"
#include "uart.h"
#include "cache.h"
#include "smp.h"
#include "pmu.h"
#include "rtc.h"
#include "con_utils.h"
#include "l2_pmu.h"
#include "l3_pmu.h"
#if PLF_HSM_SUPPORT
#include "cluster_control.h"
#endif // PLF_HSM_SUPPORT

#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

#define MISEL_ACCESS    BIT(__riscv_xlen - 1) /* indirect CSR access */

#ifdef PLF_DTS_FILE
extern char dt_blob_start, dt_blob_end;
#endif // PLF_NO_DTS

#ifdef PAGE_OFFSET
#define __pa(vaddr) ((uintptr_t)(vaddr) - (PAGE_OFFSET - PLF_MEM_BASE))
#define __va(paddr) ((uintptr_t)(paddr) + (PAGE_OFFSET - PLF_MEM_BASE))
#else // PAGE_OFFSET
#define __pa(vaddr) ((uintptr_t)(vaddr))
#define __va(paddr) ((uintptr_t)(paddr))
#endif // PAGE_OFFSET

__thread hart_storage hart_local_storage;

#if PLF_SMP_SUPPORT
static arch_spinlock_t printk_lock = SPINLOCK_INIT(0);
#endif // PLF_SMP_SUPPORT

void print_bad_trap(const char *hdr_str, uintptr_t cause, uintptr_t* regs, uintptr_t cycle, uintptr_t instr)
{
    static const char* regnames[] = {
        "z  ", "ra ", "sp ", "gp ", "tp ", "t0 ", "t1 ", "t2 ",
        "s0 ", "s1 ", "a0 ", "a1 ", "a2 ", "a3 ", "a4 ", "a5 ",
        "a6 ", "a7 ", "s2 ", "s3 ", "s4 ", "s5 ", "s6 ", "s7 ",
        "s8 ", "s9 ", "s10", "s11", "t3 ", "t4 ", "t5 ", "t6 "
    };

#if PLF_SMP_SUPPORT
    arch_spin_lock(&printk_lock);
#endif // PLF_SMP_SUPPORT
    printk("\n\n%s trap 0x%lu @ 0x%lu\n"
           "hart#%lu status %lu mtval %lu\n"
           "instr %lu cycle %lu\n",
           hdr_str, (unsigned long)cause, read_csr(mepc), arch_hartid(),
           read_csr(mstatus), arch_mtval(), (unsigned long)instr,
           (unsigned long)cycle);

#ifdef __riscv_32e
    const int regs_num = 16;
#else
    const int regs_num = 32;
#endif

    if (regs) {
        for (int i = 0; i < regs_num; ++i) {
#if __riscv_xlen == 32
            uart_putc(' ');
            uart_putc(' ');
            uart_putc('0' + i / 10);
            uart_putc('0' + i % 10);
            uart_putc(' ');
#endif // __riscv_xlen == 32
            uart_puts(regnames[i]);
            uart_putc(' ');
            uart_puthex(regs[i]);
            uart_putc(((i & 0x3) == 0x3) ? '\n' : ' ');
        }
    }

#if PLF_SMP_SUPPORT
    arch_spin_unlock(&printk_lock);
#endif // PLF_SMP_SUPPORT
}

void __attribute__((noreturn)) bad_trap(void)
{
    print_bad_trap("Bad", read_csr(mcause), 0, read_csr(cycle), read_csr(instret));

    hart_halt();
}

void __attribute__((noreturn))
bad_trap_handler(uintptr_t mcause, uintptr_t* regs, uintptr_t cycle, uintptr_t instr)
{
    print_bad_trap("+Unhandlable", mcause, regs, cycle, instr);

    hart_halt();
}

#if PLF_MMODE_ONLY == 0
void __attribute__((noreturn)) bad_stvec(void)
{
    print_bad_trap("STVEC=0 at", read_csr(mcause), 0, read_csr(cycle), read_csr(instret));

    hart_halt();
}
#endif // !PLF_MMODE_ONLY

#define NOINLINE_ATTR  __attribute__((noinline))

#if defined(PLF_KERNEL_ENTRY) && !defined(PLF_UART0_SCR_RTL)
#define KERNEL_LOG_COLORIZER_SUPPORTED 1
#endif // PLF_KERNEL_ENTRY && !PLF_UART0_SCR_RTL

#if KERNEL_LOG_COLORIZER_SUPPORTED
static int skip_con_colorize = 0;
#endif // KERNEL_LOG_COLORIZER_SUPPORTED

static uintptr_t NOINLINE_ATTR mcall_console_putchar(int ch)
{
#if PLF_SMP_SUPPORT
    arch_spin_lock(&printk_lock);
#endif // PLF_SMP_SUPPORT

#if KERNEL_LOG_COLORIZER_SUPPORTED
    if (!skip_con_colorize) {
        con_colorize_putchar(ch);
    }else {
        uart_putc(ch);
    }
#else // KERNEL_LOG_COLORIZER_SUPPORTED
    uart_putc(ch);
#endif // KERNEL_LOG_COLORIZER_SUPPORTED

#if PLF_SMP_SUPPORT
    arch_spin_unlock(&printk_lock);
#endif // PLF_SMP_SUPPORT

    return 0;
}

static int NOINLINE_ATTR mcall_console_getchar(void)
{
#if KERNEL_LOG_COLORIZER_SUPPORTED
    skip_con_colorize = 1;
#endif // KERNEL_LOG_COLORIZER_SUPPORTED
    return uart_getc_nowait();
}

#if PLF_SMP_SUPPORT

#if PLF_HSM_SUPPORT
// mask for linux ready harts
volatile unsigned long harts_online;
#endif // PLF_HSM_SUPPORT

// actual number of ready harts
static inline unsigned long actual_mask (unsigned long mask)
{
#if PLF_HSM_SUPPORT
    return (mask & harts_online);
#else
    return mask;
#endif // PLF_HSM_SUPPORT
}

static unsigned long mcall_pmask2mask(uintptr_t pmask)
{
    unsigned long hart_mask = ~0;

    if (pmask && __pa(pmask) >= PLF_MEM_BASE && __pa(pmask) < (PLF_MEM_BASE + PLF_MEM_SIZE))
        hart_mask = *(uintptr_t*)__pa(pmask);
    hart_mask = actual_mask(hart_mask);
    return hart_mask;
}

static unsigned long mcall_maskoff2mask(unsigned long mask, unsigned long offs)
{
    if (offs == -1UL)
        return actual_mask(-1UL);

    if (offs < __riscv_xlen)
        return actual_mask(mask << offs);

    return 0;
}

// external handlers (asm funcs)
extern void ipi_handler_sfence_vma(ipi_msg *msg);
extern void ipi_handler_sfence_vma_asid(ipi_msg *msg);

#if PLF_IPI_ASM_HANDLERS == 0
static void mcall_signal(unsigned long hart_mask)
{
    ipi_send_signal(hart_mask, SCR_IPI_CMD_SIGNAL);
}

static void mcall_clear_ipi(void)
{
    clear_csr(mip, (1 << INT_SM_SOFTWARE)); // MIP_SSIP
}
#endif // PLF_IPI_ASM_HANDLERS == 0

static void mcall_fence_i(unsigned long hart_mask)
{
    ipi_send_signal(hart_mask, SCR_IPI_CMD_FENCE_I);
}

#define TLB_FLUSH_THRESHOLD (4 * PGSIZE)
#define SFENCE_ALL_SIZE (-1)

static void mcall_sfence_vma(unsigned long hart_mask, unsigned long addr, unsigned long size)
{
    ipi_msg *msg = ipi_alloc_msg();

    msg->handler_msg = (uintptr_t)ipi_handler_sfence_vma;
    msg->payload.rfence.addr = addr;
    msg->payload.rfence.size = size > TLB_FLUSH_THRESHOLD ? SFENCE_ALL_SIZE : size;

    ipi_send_msg(hart_mask, msg);
    ipi_free_msg(msg);
}

static void mcall_sfence_vma_asid(unsigned long hart_mask, unsigned long addr, unsigned long size, unsigned long asid)
{
    ipi_msg *msg = ipi_alloc_msg();

    msg->handler_msg = (uintptr_t)ipi_handler_sfence_vma_asid;
    msg->payload.rfence.addr = addr;
    msg->payload.rfence.size = size > TLB_FLUSH_THRESHOLD ? SFENCE_ALL_SIZE : size;
    msg->payload.rfence.asid = asid;

    ipi_send_msg(hart_mask, msg);
    ipi_free_msg(msg);
}

#endif // PLF_SMP_SUPPORT

static void NOINLINE_ATTR mcall_shutdown(void)
{
    extern void _start(void);
#if PLF_SMP_SUPPORT
    if (arch_hartid() != PLF_MASTER_HART) {
        // repost shutdown to master hart
        // master hart always has logical#0
        unsigned long msg = PACK_SCIPI_MSG(SCR_IPI_CMD_RESET, 0);
        unsigned receiver = hart_hls(0)->ipi_n;
        ipi_wait_receiver_avail(receiver, 1000);
        ipi_send_data(receiver, msg);
    }
#endif // PLF_SMP_SUPPORT
    _start();
    __builtin_unreachable();
}

static void NOINLINE_ATTR mcall_set_timer(uint64_t when)
{
    scr_rtc_setcmp(when);
    clear_csr(mip, (1 << 5)/* MIP_STIP */);
    set_csr(mie, (1 << 7)/* MIP_MTIP */);
}

static unsigned long NOINLINE_ATTR mcall_ext_base(uintptr_t* regs)
{
    unsigned long ret = SBI_ERROR_SUCCESS;
    unsigned long retval = 0;
    unsigned long fn = regs[16];

    switch (fn) {
    case SBI_EXT_BASE_FN_GET_SPEC_VERSION:
        retval = SCR_SBI_SPEC;
        break;
    case SBI_EXT_BASE_FN_GET_IMP_ID:
        retval = SCR_SBI_IMP_ID;
        break;
    case SBI_EXT_BASE_FN_GET_IMP_VERSION:
        retval = SCR_SBI_IMP_VER;
        break;
    case SBI_EXT_BASE_FN_PROBE_EXT:
        // check exts
        switch (regs[10]) {
        case SBI_EXT_TIME:
#if PLF_SMP_SUPPORT
        case SBI_EXT_IPI:
        case SBI_EXT_RFENCE:
#if PLF_HSM_SUPPORT
        case SBI_EXT_HSM:
#endif // PLF_HSM_SUPPORT
#endif // PLF_SMP_SUPPORT
#if PLF_PMU_SUPPORT
        case SBI_EXT_PMU:
#endif // PLF_PMU_SUPPORT
        case SBI_EXT_SCR:
            retval = 1;
            break;
        default:
            if ((SBI_EXT_VENDOR_START <= regs[10]) && (regs[10] <= SBI_EXT_VENDOR_END)) {
                printk("Invalid  vendor extension 0x%lx: update kernel\n", (unsigned long)regs[10]);
            }
            retval = 0;
            break;
        }
        break;
    case SBI_EXT_BASE_FN_GET_MVENDORID:
        retval = read_csr(mvendorid);
        break;
    case SBI_EXT_BASE_FN_GET_MARCHID:
        retval = read_csr(marchid);
        break;
    case SBI_EXT_BASE_FN_GET_MIMPID:
        retval = read_csr(mimpid);
        break;
    default:
        ret = SBI_ERROR_NOT_SUPPORTED;
        break;
    }

    regs[11] = retval;

    return ret;
}

#if PLF_PMU_SUPPORT
static unsigned long NOINLINE_ATTR mcall_ext_pmu(uintptr_t* regs)
{
    unsigned long fn = regs[16];
    unsigned long ret;
    unsigned long out = 0;

    switch (fn) {
    case SBI_EXT_PMU_NUM_COUNTERS:
        ret = sbi_pmu_num_ctr(&out);
        break;
    case SBI_EXT_PMU_COUNTER_GET_INFO:
        ret = sbi_pmu_ctr_get_info(regs[10], &out);
        break;
    case SBI_EXT_PMU_COUNTER_CFG_MATCH:
        ret = sbi_pmu_ctr_cfg_match(regs[10], regs[11], regs[12], regs[13], regs[14], regs[15], &out);
        break;
    case SBI_EXT_PMU_COUNTER_START:
        ret = sbi_pmu_ctr_start(regs[10], regs[11], regs[12], regs[13], regs[14]);
        break;
    case SBI_EXT_PMU_COUNTER_STOP:
        ret = sbi_pmu_ctr_stop(regs[10], regs[11], regs[12]);
        break;
    case SBI_EXT_PMU_COUNTER_FW_READ:
        ret = sbi_pmu_fw_ctr_read(regs[10], &out);
        break;
    default:
        ret = SBI_ERROR_NOT_SUPPORTED;
        break;
    }

    regs[11] = out;

    return ret;
}
#endif

#if PLF_L2_PMU_SUPPORT
static unsigned long NOINLINE_ATTR mcall_ext_l2_pmu(uintptr_t* regs)
{
    unsigned long ret = SBI_ERROR_SUCCESS;
    unsigned long fn = regs[10];
    unsigned long out = 0;

    switch (fn) {
    case SBI_EXT_SCR_PMU_PROBE:
        out = sbi_l2_pmu_features_flags();
        break;
    case SBI_EXT_PMU_NUM_COUNTERS:
        ret = sbi_l2_pmu_num_ctr(&out);
        break;
    case SBI_EXT_PMU_COUNTER_GET_INFO:
        ret = sbi_l2_pmu_ctr_get_info(regs[11], &out);
        break;
    case SBI_EXT_PMU_COUNTER_CFG_MATCH:
        ret = sbi_l2_pmu_ctr_cfg_match(regs[11], regs[12], regs[13], regs[14], regs[15], &out);
        break;
    case SBI_EXT_PMU_COUNTER_START:
        ret = sbi_l2_pmu_ctr_start(regs[11], regs[12], regs[13], regs[14], regs[15]);
        break;
    case SBI_EXT_PMU_COUNTER_STOP:
        ret = sbi_l2_pmu_ctr_stop(regs[11], regs[12], regs[13]);
        break;
    case SBI_EXT_SCR_PMU_COUNTER_HW_READ:
        ret = sbi_l2_pmu_hw_ctr_read(regs[11], &out);
        break;
    case SBI_EXT_SCR_PMU_VID:
        ret = sbi_l2_pmu_get_vid(regs[11], &out);
        break;
    default:
        ret = SBI_ERROR_NOT_SUPPORTED;
        break;
    }

    regs[11] = out;

    return ret;
}
#endif

#if PLF_L3_PMU_SUPPORT
static unsigned long NOINLINE_ATTR mcall_ext_l3_pmu(uintptr_t* regs)
{
    unsigned long ret = SBI_ERROR_SUCCESS;
    unsigned long fn = regs[10];
    unsigned long out = 0;

    switch (fn) {
    case SBI_EXT_SCR_PMU_PROBE:
        out = sbi_l3_pmu_features_flags();
        break;
    case SBI_EXT_PMU_NUM_COUNTERS:
        ret = sbi_l3_pmu_num_ctr(&out);
        break;
    case SBI_EXT_PMU_COUNTER_GET_INFO:
        ret = sbi_l3_pmu_ctr_get_info(regs[11], &out);
        break;
    case SBI_EXT_PMU_COUNTER_CFG_MATCH:
        ret = sbi_l3_pmu_ctr_cfg_match(regs[11], regs[12], regs[13], regs[14], regs[15], &out);
        break;
    case SBI_EXT_PMU_COUNTER_START:
        ret = sbi_l3_pmu_ctr_start(regs[11], regs[12], regs[13], regs[14], regs[15]);
        break;
    case SBI_EXT_PMU_COUNTER_STOP:
        ret = sbi_l3_pmu_ctr_stop(regs[11], regs[12], regs[13]);
        break;
    case SBI_EXT_SCR_PMU_COUNTER_HW_READ:
        ret = sbi_l3_pmu_hw_ctr_read(regs[11], &out);
        break;
    case SBI_EXT_SCR_PMU_VID:
        ret = sbi_l3_pmu_get_vid(regs[11], &out);
        break;
    default:
        ret = SBI_ERROR_NOT_SUPPORTED;
        break;
    }

    regs[11] = out;

    return ret;
}
#endif

#if PLF_HSM_SUPPORT
volatile sbi_hsm_start_regs hsm_start_arr [PLF_SMP_HART_NUM];
#if PLF_HSM_HOTPLUG
extern void ipi_handler_hsm_stop_callback (ipi_msg *msg);
static void hsm_send_ipi_stop (void)
{
    ipi_msg *msg = ipi_alloc_msg();

    msg->handler_msg = (uintptr_t)ipi_handler_hsm_stop_callback;
    msg->payload.hsm.priv = read_csr(mhartid);
    ipi_send_msg((1UL << PLF_MASTER_HART), msg);
    ipi_free_msg(msg);
}
#endif // PLF_HSM_HOTPLUG

static unsigned long NOINLINE_ATTR mcall_ext_hsm(uintptr_t* regs)
{
    unsigned long fn = regs[16];
    unsigned long ret, out = 0;

    switch (fn) {
    case SBI_EXT_HSM_FN_HART_START:
        // a0 - hartid, a1 - start_addr, a2 - priv (sp, tp)
        // when start secondary: pc -> start_addr, a0->hartid, a1->priv
        if ((regs[10] == 0) || (regs[10] >= PLF_SMP_HART_NUM)) {
            // Master hart doesn't need startup, also avoid incorrect hart num
            ret = SBI_ERROR_INVALID_PARAM;
        } else {
            volatile sbi_hsm_start_regs *hsm = &hsm_start_arr[regs[10]];

            // Completing startup table for Linux boot
            hsm->hartid = regs[10];
            hsm->start_addr = regs[11];
            hsm->opaque = regs[12];
            mb();
            hsm->initialized = true;
#if PLF_HSM_HOTPLUG
            if (!((*(volatile uint64_t*)CL_CPU_N_CTRL(regs[10])) & CL_CPU_CTRL_RESET_N_BIT)) {
                // Full startup sequence if target CPU is in reset, otherwise skip this part
                // Activate other cpu
                plf_ext_cpu_activate(regs[10]);
                // Wait while IPI is ready for receive (should come to wfi)
                while (!(*(volatile uint64_t*)CL_CPU_N_CTRL(regs[10]) & CL_CPU_CTRL_RESET_N_BIT));
                while (!(*(volatile uint64_t*)CL_CPU_N_CTRL(regs[10]) & CL_CPU_CTRL_WFI_BIT));
                // Send setup message and wait until process finish
                ipi_data_t msg = PACK_SCIPI_MSG(SCR_IPI_CMD_SETUP, regs[10]);
                hls_t *hls = hart_hls(regs[10]);
                ipi_send_data(hls->ipi_n, msg);
                do {
                    rtc_delay_us(1);
#if PLF_SMP_NON_COHERENT
                    cache_invalidate(hls, sizeof(*hls));
#endif // PLF_SMP_NON_COHERENT
                } while (((volatile hls_t *)hls)->state != SBI_HSM_STATE_START_PENDING);
                // Send start message and wait until it processed
                msg = PACK_SCIPI_MSG(SCR_IPI_CMD_START, regs[10]);
                ipi_send_data(hls->ipi_n, msg);
                do {
                    rtc_delay_us(1);
#if PLF_SMP_NON_COHERENT
                    cache_invalidate(hls, sizeof(*hls));
#endif // PLF_SMP_NON_COHERENT
                } while (((volatile hls_t *)hls)->state != SBI_HSM_STATE_START);
                while (!(harts_online & (1 << regs[10])));
            }
#endif // PLF_HSM_HOTPLUG
            ret = SBI_ERROR_SUCCESS;
        }
        break;
    case SBI_EXT_HSM_FN_HART_STOP:
#if PLF_HSM_HOTPLUG
        if (read_csr(mhartid) != PLF_MASTER_HART) {
            // write STOP to hls->state and update harts_online mask
            hls_t *hls = hart_hls(logical_hart());
            hls->state = SBI_HSM_STATE_STOP;
            harts_online &= ~ (1 << logical_hart());
            // send STOP message to master hart
            hsm_send_ipi_stop();
            // prepare cpu to stop
            plf_local_cpu_deactivation_prepare();
            ret = SBI_ERROR_SUCCESS;
        } else {
            ret = SBI_ERROR_FAILURE;
        }

#else// PLF_HSM_HOTPLUG
        ret = SBI_ERROR_NOT_SUPPORTED;
#endif // PLF_HSM_HOTPLUG
        break;
    case SBI_EXT_HSM_FN_HART_STATUS:
#if PLF_HSM_HOTPLUG
        if (regs[10] < PLF_SMP_HART_NUM) {
            ret = ((*(volatile uint64_t*)CL_CPU_N_CTRL(regs[10])) & CL_CPU_CTRL_RESET_N_BIT);
            if (ret) {
                // If hart is powered up, getting state from hls structure
                hls_t *hls = hart_hls(regs[10]);
                out = hls->state;
            } else {
                out = SBI_HSM_STATE_STOP;
            }
            ret = SBI_ERROR_SUCCESS;
        } else {
            ret = SBI_ERROR_INVALID_PARAM;
        }
#else// PLF_HSM_HOTPLUG
        ret = SBI_ERROR_NOT_SUPPORTED;
#endif // PLF_HSM_HOTPLUG
        break;
    default:
        ret = SBI_ERROR_NOT_SUPPORTED;
        break;
    }

    regs[11] = out;

    return ret;
}
#endif // PLF_HSM_SUPPORT

#if PLF_VCS_XCHG_SUPPORT
static uintptr_t NOINLINE_ATTR mcall_ext_custom_fn(uintptr_t* regs)
{
    uintptr_t ret = SBI_ERROR_SUCCESS;
    uintptr_t fn = regs[10];
    uintptr_t out = 0;

    switch (fn) {
    case SBI_EXT_SCR_VCS_PROBE:
        break;
    case SBI_EXT_SCR_VCS_WRITE:
        *(volatile uintptr_t *) PLF_VCS_XCHG_ADDR = regs[11];
        break;
    case SBI_EXT_SCR_VCS_READ:
        out = *(volatile uintptr_t *) PLF_VCS_XCHG_ADDR;
        break;
    default:
        ret = SBI_ERROR_NOT_SUPPORTED;
        break;
    }

    regs[11] = out;

    return ret;
}
#endif // PLF_VCS_XCHG_SUPPORT

static uintptr_t NOINLINE_ATTR mcall_ext_plfinfo(uintptr_t* regs)
{
    uintptr_t ret = SBI_ERROR_SUCCESS;
    uintptr_t fn = regs[10];
    uintptr_t out = 0;

    switch (fn) {
    case SBI_EXT_SCR_PLF_GET_FEAT_EN:
        if (csr_is_available(FEAT_EN_CSR)) {
            out = (uint32_t)read_csr(FEAT_EN_CSR);
        } else {
            ret = SBI_ERROR_NOT_SUPPORTED;
        }
        break;
    case SBI_EXT_SCR_PLF_GET_VCR_SCHEMA:
        if (csr_is_available(SCR_MISELECT_CSR)) {
           write_csr(SCR_MISELECT_CSR, MISEL_ACCESS);
           if (csr_is_available(SCR_VCR_SCHEMA_CSR)) {
               out = read_csr(SCR_VCR_SCHEMA_CSR);
           } else {
               ret = SBI_ERROR_NOT_SUPPORTED;
           }
        } else {
            ret = SBI_ERROR_NOT_SUPPORTED;
        }
        break;
    case SBI_EXT_SCR_PLF_GET_CORE_TIMESTAMP_ID:
        if (csr_is_available(SCR_MISELECT_CSR)) {
           write_csr(SCR_MISELECT_CSR, MISEL_ACCESS);
           if (csr_is_available(SCR_CORE_TIMESTAMP_ID_CSR)) {
               out = read_csr(SCR_CORE_TIMESTAMP_ID_CSR);
           } else {
               ret = SBI_ERROR_NOT_SUPPORTED;
           }
        } else {
            ret = SBI_ERROR_NOT_SUPPORTED;
        }
        break;
    case SBI_EXT_SCR_PLF_GET_CORE_BUILD_TARGET_ID:
        if (csr_is_available(SCR_MISELECT_CSR)) {
           write_csr(SCR_MISELECT_CSR, MISEL_ACCESS);
           if (csr_is_available(SCR_CORE_BUILD_TARGET_ID_CSR)) {
               out = read_csr(SCR_CORE_BUILD_TARGET_ID_CSR);
           } else {
               ret = SBI_ERROR_NOT_SUPPORTED;
           }
        } else {
            ret = SBI_ERROR_NOT_SUPPORTED;
        }
        break;
    case SBI_EXT_SCR_PLF_GET_CORE_CFG_ID:
        if (csr_is_available(SCR_MISELECT_CSR)) {
           write_csr(SCR_MISELECT_CSR, MISEL_ACCESS);
           if (csr_is_available(SCR_CORE_CFG_ID_CSR)) {
               out = read_csr(SCR_CORE_CFG_ID_CSR);
           } else {
               ret = SBI_ERROR_NOT_SUPPORTED;
           }
        } else {
            ret = SBI_ERROR_NOT_SUPPORTED;
        }
        break;
    case SBI_EXT_SCR_PLF_GET_CL_VID:
#if PLF_L3CTL_BASE
        if (read_safe_m(PLF_L3CTL_BASE, &out)) {
            ret = SBI_ERROR_NOT_SUPPORTED;
        }
#else
        ret = SBI_ERROR_NOT_SUPPORTED;
#endif
        break;
    case SBI_EXT_SCR_PLF_GET_L1D_PF_CTRL0:
        if (csr_is_available(L1D_PF_CTRL0)) {
            out = read_csr(L1D_PF_CTRL0);
        } else {
            ret = SBI_ERROR_NOT_SUPPORTED;
        }
        break;
    default:
        ret = SBI_ERROR_NOT_SUPPORTED;
        break;
    }

    regs[11] = out;

    return ret;
}

uintptr_t emu_misaligned_load(uintptr_t* regs);
uintptr_t emu_misaligned_store(uintptr_t* regs);

uintptr_t mmode_trap_handler(uintptr_t cause, uintptr_t* regs, uintptr_t cycle, uintptr_t instr)
{
    uintptr_t call_num = regs[17], arg0 = regs[10], retval;

    if (cause == EXC_MIS_LOAD) {
        return emu_misaligned_load(regs);
    } else if (cause == EXC_MIS_STORE) {
        return emu_misaligned_store(regs);
    } else if (cause < EXC_UM_ECALL || cause > EXC_MM_ECALL) {
        bad_trap_handler(cause, regs, cycle, instr);
    }

    retval = SBI_ERROR_SUCCESS;

    switch (call_num) {
    case SBI_CONSOLE_PUTCHAR:
        mcall_console_putchar(arg0);
        break;
    case SBI_CONSOLE_GETCHAR:
        retval = mcall_console_getchar();
        break;
#if PLF_SMP_SUPPORT
#if PLF_IPI_ASM_HANDLERS == 0
    case SBI_SEND_IPI: // SBI v0.1 ipi
        mcall_signal(mcall_pmask2mask(arg0));
        break;
    case SBI_CLEAR_IPI:
        mcall_clear_ipi();
        break;
#endif // PLF_IPI_ASM_HANDLERS == 0
    case SBI_REMOTE_FENCE_I:
        mcall_fence_i(mcall_pmask2mask(arg0));
        break;
    case SBI_REMOTE_SFENCE_VMA:
        mcall_sfence_vma(mcall_pmask2mask(arg0), regs[11], regs[12]);
        break;
    case SBI_REMOTE_SFENCE_VMA_ASID:
        mcall_sfence_vma_asid(mcall_pmask2mask(arg0), regs[11], regs[12], regs[13]);
        break;
#endif // PLF_SMP_SUPPORT
    case SBI_SHUTDOWN:
        mcall_shutdown();
        break;
    case SBI_EXT_TIME: // SBI v0.2 time
        if (regs[16] != SBI_EXT_TIME_FN_SET) {
            retval = SBI_ERROR_NOT_SUPPORTED;
            break;
        }
        /* FALLTHRU */
    case SBI_SET_TIMER:
#if __riscv_xlen == 32
        mcall_set_timer(arg0 | ((uint64_t)regs[11] << 32));
#else  // __riscv_xlen == 32
        mcall_set_timer(arg0);
#endif // __riscv_xlen == 32
        break;
    case SBI_EXT_BASE: // SBI v0.2 base
        retval = mcall_ext_base(regs);
        break;
#if PLF_SMP_SUPPORT
#if PLF_IPI_ASM_HANDLERS == 0
    case SBI_EXT_IPI: // // SBI v0.2 ipi
        if (regs[16] == SBI_EXT_IPI_FN_SEND) {
            // hartbits == logical harts bits
            mcall_signal(mcall_maskoff2mask(arg0, regs[11]));
        } else {
            retval = SBI_ERROR_INVALID_PARAM;
        }
        break;
#endif // PLF_IPI_ASM_HANDLERS == 0
    case SBI_EXT_RFENCE: // SBI v0.2 rfence
        switch (regs[16]) {
        case SBI_EXT_RFENCE_FN_FENCE_I:
            mcall_fence_i(mcall_maskoff2mask(arg0, regs[11]));
            break;
        case SBI_EXT_RFENCE_FN_SFENCE_VMA:
            mcall_sfence_vma(mcall_maskoff2mask(arg0, regs[11]), regs[12], regs[13]);
            break;
        case SBI_EXT_RFENCE_FN_SFENCE_VMA_ASID:
            mcall_sfence_vma_asid(mcall_maskoff2mask(arg0, regs[11]), regs[12], regs[13], regs[14]);
            break;
        default:
            retval = SBI_ERROR_NOT_SUPPORTED;
            break;
        }
        break;
#if PLF_HSM_SUPPORT
    case SBI_EXT_HSM:
        retval = mcall_ext_hsm(regs);
        break;
#endif // PLF_HSM_SUPPORT
#endif // PLF_SMP_SUPPORT
#if PLF_PMU_SUPPORT
    case SBI_EXT_PMU: // SBI v0.3 PMU
        retval = mcall_ext_pmu(regs);
        break;
#endif // PLF_PMU_SUPPORT
    case SBI_EXT_SCR:
        switch (regs[16]) {
#if PLF_L2_PMU_SUPPORT
        case SBI_SCR7_L2_PMU_FN:
            retval = mcall_ext_l2_pmu(regs);
            break;
#endif // PLF_L2_PMU_SUPPORT
#if PLF_L3_PMU_SUPPORT
        case SBI_SCR7_L3_PMU_FN:
            retval = mcall_ext_l3_pmu(regs);
            break;
#endif // PLF_L3_PMU_SUPPORT
#if PLF_VCS_XCHG_SUPPORT
	case SBI_SCR_VCS_FN:
            retval = mcall_ext_custom_fn(regs);
            break;
#endif // PLF_VCS_XCHG_SUPPORT
        case SBI_SCR_PLFINFO:
            retval = mcall_ext_plfinfo(regs);
            break;
        default:
            retval = SBI_ERROR_NOT_SUPPORTED;
            break;
        }
        break;
    default:
        retval = SBI_ERROR_NOT_SUPPORTED; //-ENOSYS;
        break;
    case 255:
        while (1);
        break;
    }
    regs[10] = retval;
    write_csr(mepc, read_csr(mepc) + 4);
    return 0;
}
