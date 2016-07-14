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

/// Syntacore SCR* framework
///
/// @copyright Copyright (C) 2015-2023, Syntacore Ltd. All rights reserved.
/// @author mn-sc
///
/// @brief machine mmode startup code, exception handlers

#include "platform_config.h"
#include "csr.h"
#include "hls.h"
#include "cache.h"
#include "mmu.h"
#if PLF_PMP_SUPPORT
#include "pmp.h"
#endif // PLF_PMP_SUPPORT
#include "smp.h"
#include "sbi.h"
#include "rtc.h"
#include "asm.h"
#if PLF_HSM_HOTPLUG
#include "cluster_control.h"
#endif // PLF_HSM_HOTPLUG

    .section .data
    .option norvc

    .global _start, main
    .global mmode_trap_handler, bad_trap, bad_trap_handler, plf_init, plf_mem_init, plf_hart_mem_init, hart_halt
#if PLF_MMODE_ONLY == 0
    .global bad_stvec
#endif // !PLF_MMODE_ONLY
#if PLF_SMP_SUPPORT
    .global hls_init
#endif // PLF_SMP_SUPPORT
    .global __SCBL_END, __SCBL_START
#ifdef PLF_L3CTL_BASE
    .global wrong_startup_address, __SCBL_RELOC_FLAG
#endif // PLF_L3CTL_BASE

    .section ".vectors","ax",@progbits


_crt_start_relocated:
    jal   hart_low_reset

    // setup gp
    load_addrword gp, __global_pointer$

    // init HART0 sp, tp
    load_addrword sp, __TLS0_BASE__
    mv    tp, sp
    csrw  mscratch, sp

    // configure caches
#ifdef PLF_CACHE_CFG
    jal init_caches_master
#endif // PLF_CACHE_CFG

    // boot hart 0

    jal   plf_init

    mv    a0, zero
    mv    a1, zero
    jal   main

# halt hart
hart_halt:
    // disable timer interrupts
    li    t0, (1 << INT_MM_TIMER)
    csrc  mie, t0
.Lhart_halt_loop:
    wfi
    j     .Lhart_halt_loop

#if PLF_SMP_SUPPORT
secondary_reset:
    // setup mstatus, mie, mip
    csrw  mstatus, zero
    jal   hart_low_reset
    csrw  mip, zero
    csrw  mie, (1 << INT_MM_SOFTWARE)
    ipi_wait_cmd SCR_IPI_CMD_SETUP

secondary_setup:
    // setup gp
    load_addrword gp, __global_pointer$
#ifdef PLF_CACHE_CFG
    jal   init_caches_secondary
#endif // PLF_CACHE_CFG
    // setup sp, HLS
    // get hls addr to a1
    ipi_get_hls_addr
    lui   tp, %hi(__TLS_SIZE__)
    addi  tp, tp, %lo(__TLS_SIZE__)
    sub   sp, a1, tp
    mv    tp, sp
    csrw  mscratch, sp

    jal   plf_hart_mem_init
    jal   hart_fill_hls
    ipi_wait_cmd SCR_IPI_CMD_START

secondary_start:
    // boot this hart
    j     machine_init
#endif // PLF_SMP_SUPPORT

hart_low_reset:
    // setup mtvec
    load_addrword t0, __trap_hander
    csrw  mtvec, t0
    // preconfigure MPU (global NC and MMCFG)
#if PLF_MPU_SUPPORT
    mpu_reset_init
#elif PLF_PMP_SUPPORT
    pmp_early_init
#endif
    // reset and disable L1$
    cache_reset_nc
    ret

// init caches
#ifdef PLF_CACHE_CFG
init_caches_master:
#ifdef PLF_CACHE_INIT_MASTER_ASM
    PLF_CACHE_INIT_MASTER_ASM
#endif
    cache_l3_init
#if PLF_CACHE_L2_DEDICATED
init_caches_secondary:
#endif
    cache_l2_init
#if PLF_CACHE_L2_DEDICATED == 0
init_caches_secondary:
#endif
    cache_l1_init
    ret
#endif // PLF_CACHE_CFG

#if PLF_SMP_SUPPORT
    // IPI interrupt handlers
    .pushsection .text
    .global ipi_handler_sfence_vma
    .global ipi_handler_sfence_vma_asid
#if PLF_HSM_HOTPLUG
    .global ipi_handler_hsm_stop_callback
#endif // PLF_HSM_HOTPLUG

ipi_handler_sfence_vma:
    // a0: ipi_msg*
    .cfi_startproc
    load_xreg a1, SCIPI_MSG_RFENCE_SIZE_OFF(a0)
    bltz    a1, .Lsfence_vma_all // if size < 0 -> sfence all
    load_xreg a0, SCIPI_MSG_RFENCE_ADDR_OFF(a0)
    // if addr == 0 & size == 0 -> sfence all
    or      t0, a0, a1
    beqz    t0, .Lsfence_vma_all
    // align addr and fix size (for page-aligned loop)
    li      t0, PGSIZE - 1
    and     t0, a0, t0  // page_offset
    sub     a0, a0, t0  // a0: aligned addr
    add     a1, a1, t0  // a1: size + page_offset
    li      t0, PGSIZE
.Lsfence_vma_loop:
    sfence.vma a0, zero
    sub     a1, a1, t0
    add     a0, a0, t0
    bgtz    a1, .Lsfence_vma_loop
    ret
.Lsfence_vma_all:
    sfence.vma
    ret
    .cfi_endproc
    .size ipi_handler_sfence_vma, .-ipi_handler_sfence_vma

ipi_handler_sfence_vma_asid:
    .cfi_startproc
    load_xreg a1, SCIPI_MSG_RFENCE_SIZE_OFF(a0)
    load_xreg t1, SCIPI_MSG_RFENCE_ASID_OFF(a0)
    bltz    a1, .Lsfence_vma_asid_all // if size < 0 -> sfence all
    load_xreg a0, SCIPI_MSG_RFENCE_ADDR_OFF(a0)

    // if addr == 0 & size == 0 -> sfence all
    or      t0, a0, a1
    beqz    t0, .Lsfence_vma_asid_all
    // align addr and fix size (for page-aligned loop)
    li      t0, PGSIZE - 1
    and     t0, a0, t0  // page_offset
    sub     a0, a0, t0  // a0: aligned addr
    add     a1, a1, t0  // a1: size + page_offset
    li      t0, PGSIZE
.Lsfence_vma_asid_loop:
    sfence.vma a0, t1
    sub     a1, a1, t0
    add     a0, a0, t0
    bgtz    a1, .Lsfence_vma_asid_loop
    ret
.Lsfence_vma_asid_all:
    sfence.vma zero, t1
    ret
    .cfi_endproc
    .size ipi_handler_sfence_vma_asid, .-ipi_handler_sfence_vma_asid

#if PLF_HSM_HOTPLUG
ipi_handler_hsm_stop_callback:
    // a0: ipi_msg*
    .cfi_startproc
    load_xreg a0, SCIPI_MSG_HSM_PRIV_OFF(a0) // load hartid to turn off
    // hart id must not match cpu id (from ipi_msg)
    csrr    a1, mhartid                  // read mhartid
    slli    t0, a0, 0x20                 // l/r shift for input argument
    srli    t0, t0, 0x20
    beq     t0, a1, 1f                   // if cpu id matches hartid, nothing to do here
    // read spin until wfi
    slliw   a1, a0, 0x6                  // calculate register offset from core 0: (N) * 0x40
    slli    a1, a1, 0x20                 // l/r shift
    srli    a1, a1, 0x20
    li      t0, CL_CPU_0_CTRL            // load CL_CPU_0_CTRL register address
    add     a1, a1, t0                   // add core offset: a1 <- CL_CPU_N_CTRL(N) 
.Lread_wfi:
    ld      t0, 0(a1)                    // read register CL_CPU_N_CTRL(cpu_id)
    andi    t0, t0, CL_CPU_CTRL_WFI_BIT  // extract wfi bit
    beqz    t0, .Lread_wfi               // if not in wfi, wait more
    // wait until cpu side activity stop
.Lread_act:
    ld      t0, 0(a1)                    // read register CL_CPU_N_CTRL(cpu_id)
    andi    t0, t0, CL_CPU_CTRL_ACT_BIT  // extract activity bit
    bnez    t0, .Lread_act               // if active, wait more
    // turn off chi link
    ld      t1, 0(a1)                    // read register CL_CPU_N_CTRL(cpu_id)
    li      t0, ~CL_CPU_CTRL_LINKUP_BIT  // mask: ~CL_CPU_CTRL_LINKUP_BIT
    and     t0, t0, t1                   // apply mask to CL_CPU_N_CTRL(cpu_id)
    sd      t0, 0(a1)                    // save register
    // wait until bus activity stop
    li      t1, CL_CPU_CTRL_LINK_STS_MASK  // link status mask
.Lread_sts:
    ld      t0, 0(a1)                    // read register CL_CPU_N_CTRL(cpu_id)
    and     t0, t0, t1                   // apply mask
    bnez    t0, .Lread_sts               // activity not stopped: wait more
    // assert cpu reset
    ld	    t0, 0(a1)                    // read register CL_CPU_N_CTRL(cpu_id)
    li      t1, ~CL_CPU_CTRL_RESET_N_BIT // mask: ~CL_CPU_CTRL_RESET_N_BIT
    and     t0, t0, t1                   // apply mask to CL_CPU_N_CTRL(cpu_id)
    sd      t0, 0(a1)                    // save registers
1:
    ret
    .cfi_endproc
    .size ipi_handler_hsm_stop_callback, .-ipi_handler_hsm_stop_callback
#endif // PLF_HSM_HOTPLUG

    .popsection
#endif // PLF_SMP_SUPPORT

#if PLF_MMODE_ONLY == 0
    .balign 0x20
    .global tlb_miss_trap_handler
tlb_miss_trap_handler:
    .cfi_startproc
#if __riscv_xlen == 32

    // Sv32 TLB miss handler
    csrr    a0, CSR_TLB_PTE1_ADDR

#else  // __riscv_xlen == 32

    // Sv39 TLB miss handler
    csrr    a0, CSR_TLB_PTE2_ADDR
    load_reg_offs a0, 0, a0
    // check gigapage (1G)
    andi    a1, a0, 0xf
    addi    a1, a1, -1
    beqz    a1, 2f

tlb_miss_trap_giga_page:
    // gigapage (1G page) or invalid PTE
    // mask pattr by propagated V-bit
    slli    a1, a0, (__riscv_xlen - 1)
    srai    a1, a1, (__riscv_xlen - 1)
    and     a0, a0, a1
    ori     a0, a0, 1

    csrw    CSR_MMU_PATTR, a0
    csrw    CSR_MMU_UPDATE, CSR_MMU_UPDATE_GPAGE

    load_regs 10, 11, sp
	csrrw   sp, mscratch, sp
	mret

2:
    // next level PTD ptr (2M pages)
    srli    a0, a0, 10
    csrr    a1, CSR_TLB_PTE1_OFFS
    slli    a0, a0, 12
    add     a0, a0, a1
#endif // __riscv_xlen == 32

    load_reg_offs a0, 0, a0
    // check megapage
    andi    a1, a0, 0xf
    addi    a1, a1, -1
    beqz    a1, tlb_miss_trap_4k_page

tlb_miss_trap_mega_page:
    // megapage (2M/4M page) or invalid PTE
    // mask pattr by propagated V-bit
    slli    a1, a0, (__riscv_xlen - 1)
    srai    a1, a1, (__riscv_xlen - 1)
    and     a0, a0, a1
    ori     a0, a0, 1

    csrw    CSR_MMU_PATTR, a0
    csrw    CSR_MMU_UPDATE, CSR_MMU_UPDATE_MPAGE

    load_regs 10, 11, sp
	csrrw   sp, mscratch, sp
	mret

tlb_miss_trap_4k_page:
    // next level PTD ptr (4K page)
    srli    a0, a0, 10
    csrr    a1, CSR_TLB_PTE0_OFFS
    slli    a0, a0, 12
    add     a0, a0, a1
    load_reg_offs a0, 0, a0

    // mask pattr by propagated V-bit
    slli    a1, a0, (__riscv_xlen - 1)
    srai    a1, a1, (__riscv_xlen - 1)
    and     a0, a0, a1
    ori     a0, a0, 1

    csrw    CSR_MMU_PATTR, a0
    csrw    CSR_MMU_UPDATE, 0

    load_regs 10, 11, sp
	csrrw   sp, mscratch, sp
	mret
    .cfi_endproc
    .size tlb_miss_trap_handler, .-tlb_miss_trap_handler
#endif // !PLF_MMODE_ONLY

    .balign 0x20

interrupt_handler:
    .cfi_startproc
#if PLF_MMODE_ONLY == 0
    slli  a0, a0, 1    // discard MSB
    addi  a0, a0, -INT_MM_SOFTWARE * 2
    bnez  a0, 2f

#if PLF_SMP_SUPPORT
    // read IPI command (a1)
    ipi_read_cmd
    addi  a1, a1, -SCR_IPI_CMD_RESET
    beqz  a1, _crt_start

    addi  a1, a1, -(SCR_IPI_CMD_SIGNAL - SCR_IPI_CMD_RESET)
    bnez  a1, 1f
    csrs  mip, (1 << INT_SM_SOFTWARE) // MIP_SSIP
    j     .Lmret
1:
    addi  a1, a1, -(SCR_IPI_CMD_FENCE_I - SCR_IPI_CMD_SIGNAL)
    bnez  a1, 1f
    fence.i
    j     .Lmret
1:
    addi  a1, a1, -(SCR_IPI_CMD_MSG_BASE - SCR_IPI_CMD_FENCE_I)
    bltz  a1, unexpected_int_handler //.Lmret
    li    a0, SCIPI_MSG_POOL_SIZE
    bgeu  a1, a0, unexpected_int_handler //.Lmret
    // IPI message with handler
    save_regs 8, 9, sp // save s0, s1
    save_regs 5, 6, sp // save t0, t1
    mv    s1, ra // save ra
    .global ipi_msg_pool
    la    a0, ipi_msg_pool
    slli  a1, a1, SCIPI_MSG_SIZE_LOG // index -> offset
    add   a0, a0, a1 // a0: ipi_msg*
    mv    s0, a0
    load_xreg a1, SCIPI_MSG_HANDLER_OFF(a0)
    jalr  a1
    li    a1, -1
    mv    ra, s1
    amoadd.w zero, a1, (s0)
    load_regs 8, 9, sp // restore s0, s1
    load_regs 5, 6, sp // restore t0, t1
    j     .Lmret

#else // PLF_SMP_SUPPORT
    // spurious IPI interrupt
    // ignore and disable it
    csrc  mie, (1 << INT_MM_SOFTWARE)
    j     .Lmret
#endif // PLF_SMP_SUPPORT
2:
    addi  a0, a0,  -(INT_MM_TIMER - INT_MM_SOFTWARE) * 2
    bnez  a0, unexpected_int_handler
	li    a0, (1 << INT_MM_TIMER)
	csrc  mie, a0
	li    a0, (1 << INT_SM_TIMER)
	csrs  mip, a0

.Lmret:
    load_regs 10, 11, sp
	csrrw sp, mscratch, sp
	mret

#endif // !PLF_MMODE_ONLY

unexpected_int_handler:
    j     handle_trap_in_machine_mode //bad_trap
    .cfi_endproc
    .size interrupt_handler, .-interrupt_handler

handle_trap_in_machine_mode:
    // Save context
    save_regs  12, 13, sp
    csrr  a2, cycle
    csrr  a3, instret

    save_reg_idx 1, sp

    csrrw a1, mscratch, sp   // a1 <- user sp
    save_reg_offs a1, 2, sp  // save user sp
    save_regs 3, 9, sp
    save_regs 14, 15, sp

#ifndef __riscv_32e
    save_regs 16, 31, sp
#endif // !__riscv_32e

    load_addrword gp, __global_pointer$
    mv    a1, sp    // a1 <- regs
    mv    tp, sp    // setup TLS

    // call trap handler
    jal   mmode_trap_handler

    // Restore context
    load_reg_idx 1, sp
    load_regs 3, 9, sp
    load_regs 12, 15, sp
#ifndef __riscv_32e
    load_regs 16, 31, sp
#endif // !__riscv_32e

#if PLF_MMODE_ONLY == 0
    // bnez a0, 1f
    beqz  a0, 1f // Go back whence we came

    // Jump to S mode if no H-extension
    csrr  a0, misa
    andi  a0, a0, (1 << ('H' - 'A'))
    beqz  a0, .Lto_s
    // Jump to S if previous mode is not V
#if __riscv_xlen == 32
    csrr  a0, CSR_MSTATUSH
    andi  a0, a0, (1 << CSR_MSTATUSH_MPV_SHIFT)
    beqz  a0, .Lfill_h_to_s
#else // __riscv_xlen == 32
    csrr  a0, mstatus
    srli  a0, a0, CSR_MSTATUS_MPV_SHIFT
    andi  a0, a0, 1
    beqz  a0, .Lfill_h_to_s
#endif // __riscv_xlen == 32
    // Jump to S if mcause >= 64 (not an exception)
    csrr  a0, mcause
    srli  a0, a0, 6
    bnez  a0, .Lfill_h_to_s
    // Jump to S if not delegated
    li    a0, 1
    csrr  a1, mcause
    sll   a1, a0, a1
    csrr  a0, hedeleg
    and   a0, a1, a0
    beqz  a0, .Lfill_h_to_s

    // Redirect the trap to the VS
    // When a trap is taken into VS-mode, vsstatus.SPP is set according to Table 8.10. Register hstatus
    // and the HS-level sstatus are not modified, and the virtualization mode V remains 1. A trap into
    // VS-mode also writes fields SPIE and SIE in vsstatus and writes CSRs vsepc, vscause, and
    // vstval.
.Lto_vs:
    // mepc -> vsepc
    csrr  a0, mepc
    csrw  CSR_VSEPC, a0
    // vstvec -> mepc
    csrr  a0, CSR_VSTVEC
    csrw  mepc, a0
    // mcause -> vscause
    csrr  a0, mcause
    csrw  CSR_VSCAUSE, a0
    // mtval -> vstval
    csrr  a0, mtval
    csrw  CSR_VSTVAL, a0

    // clear MPP (12:11)
    li    a0, (3 << CSR_MSTATUS_MPP_SHIFT)
    csrrc a0, mstatus, a0
    // convert MPP (12:11) to SPP (8)
    srli  a0, a0, CSR_MSTATUS_MPP_SHIFT - CSR_MSTATUS_SPP_SHIFT
    andi  a0, a0, (1 << CSR_MSTATUS_SPP_SHIFT)
    csrrs a0, CSR_VSSTATUS, a0
    // set SPIE (5) if SIE (1)
    andi  a0, a0, (1 << CSR_MSTATUS_SIE_SHIFT)
    slli  a0, a0, CSR_MSTATUS_SPIE_SHIFT - CSR_MSTATUS_SIE_SHIFT
    csrs  CSR_VSSTATUS, a0
    // clear SIE (1)
    csrci CSR_VSSTATUS, (1 << CSR_MSTATUS_SIE_SHIFT)

    // set MPP (12:11) to S (1) unconditionaly
    // since trap can be process only in VS/S modes
    li    a0, (1 << CSR_MSTATUS_MPP_SHIFT)
    csrs  mstatus, a0
    j 1f

    // Redirect the trap to the supervisor
    // When a trap is taken into HS-mode, virtualization mode V is set to 0, and hstatus.SPV and
    // sstatus.SPP are set according to Table 8.9. If V was 1 before the trap, field SPVP in hstatus is
    // set the same as sstatus.SPP; otherwise, SPVP is left unchanged. A trap into HS-mode also writes
    // field GVA in hstatus, fields SPIE and SIE in sstatus, and CSRs sepc, scause, stval, htval,
    // and htinst.
.Lfill_h_to_s:
    // mtval2 -> htval
    csrr a0, CSR_MTVAL2
    csrw CSR_HTVAL, a0
    // mtinst -> htinst
    csrr a0, CSR_MTINST
    csrw CSR_HTINST, a0

    // hstatus fixups

    // clear SPV (7) and GVA (6)
    li    a0, (1 << CSR_HSTATUS_SPV_SHIFT) | (1 << CSR_HSTATUS_GVA_SHIFT)
    csrc  hstatus, a0
    // clear MPV (7 for rv32 39 for rv64)
#if __riscv_xlen == 32
    li    a0, (1 << CSR_MSTATUSH_MPV_SHIFT)
    csrrc a0, CSR_MSTATUSH, a0
    // mstatus MPV:GVA (7:6) position is equal to hstatus (7:6)
#else // __riscv_xlen == 32
    li    a0, (1 << CSR_MSTATUS_MPV_SHIFT)
    csrrc a0, mstatus, a0
    // convert mstatus MPV:GVA (39:38) to hstatus (7:6)
    srli  a0, a0, (CSR_MSTATUS_GVA_SHIFT - CSR_HSTATUS_GVA_SHIFT)
#endif // __riscv_xlen == 32
    // set SPV (7) and GVA (6) if MPV:GVA in mstatus
    andi  a0, a0, (1 << CSR_HSTATUS_SPV_SHIFT) | (1 << CSR_HSTATUS_GVA_SHIFT)
    csrs  hstatus, a0

    // SPVP should be changed only if MPV (we just set SPV in hstatus if MPV)
    csrr  a0, hstatus
    andi  a0, a0, (1 << CSR_HSTATUS_SPV_SHIFT)
    bnez  a0, .Lto_s
    // clear SPVP (8)
    li    a0, (1 << CSR_HSTATUS_SPVP_SHIFT)
    csrc  hstatus, a0
    // MPV is set, so prev mode is VS or VU
    // set SPVP (8) if MPP == PRV_S (11 bit is set)
    csrr  a0, mstatus
    srli  a0, a0, (CSR_MSTATUS_MPP_SHIFT - CSR_HSTATUS_SPVP_SHIFT)
    andi  a0, a0, (1 << CSR_HSTATUS_SPVP_SHIFT)
    csrs  hstatus, a0
.Lto_s:
    csrr  a0, mepc
    csrw  sepc, a0
    csrr  a0, stvec
    beqz  a0, .Lbad_stvec
    csrw  mepc, a0
    csrr  a0, mcause
    csrw  scause, a0
    csrr  a0, mtval
    csrw  stval, a0
    // MPP -> SPP, SMODE -> MPP, SIE -> SPIE, 0 -> SIE
    // [11] -> [8], 0x01 -> [12:11], [1] -> [5], 0 -> [1]
    li    a0, (3 << CSR_MSTATUS_MPP_SHIFT) | (1 << CSR_MSTATUS_SPP_SHIFT) | (1 << CSR_MSTATUS_SPIE_SHIFT)
    csrrc a0, mstatus, a0
    srli  a0, a0, CSR_MSTATUS_MPP_SHIFT - CSR_MSTATUS_SPP_SHIFT
    andi  a0, a0, (1 << CSR_MSTATUS_SPP_SHIFT)
    csrrs a0, mstatus, a0 // set SPP
    andi  a0, a0, (1 << CSR_MSTATUS_SIE_SHIFT)
    ori   a0, a0, (1 << (CSR_MSTATUS_MPP_SHIFT - (CSR_MSTATUS_SPIE_SHIFT - CSR_MSTATUS_SIE_SHIFT))) // set [11]
    slli  a0, a0, CSR_MSTATUS_SPIE_SHIFT - CSR_MSTATUS_SIE_SHIFT
    csrs  mstatus, a0 // set SPIE
    csrci mstatus, (1 << CSR_MSTATUS_SIE_SHIFT) // clr SIE
1:
#endif // !PLF_MMODE_ONLY

    load_reg_idx 10, sp
    load_reg_idx 11, sp
    load_reg_idx 2, sp
    mret

#if PLF_MMODE_ONLY == 0
.Lbad_stvec:
    j bad_stvec
#endif // !PLF_MMODE_ONLY

    .pushsection .text

.globl memset, memset_range

// memset(a0:dst, a1:pattern, a2:size)
memset:
    .cfi_startproc
    // convert a2:size -> a2:end
    add     a2, a2, a0
// memset_range(a0:dst, a1:pattern, a2:end)
memset_range:
    // fill bytes if len < 16
    sub     a3, a2, a0
    addi    a3, a3, -16
    blez    a3, 1f
    // expand pattern byte -> word
    slli    a3, a1, 16
    or      a1, a3, a1
    slli    a3, a1, 8
    or      a1, a1, a3
    // fill till unaligned word dst
2:  andi    a3, a0, 3
    beqz    a3, 3f
    sb      a1, 0(a0)
    addi    a0, a0, 1
    j       2b
3:  andi    a3, a2, 3           // a3 = unaligned tail size
    andi    a2, a2, ~3
    // fill with aligned words
4:  beq     a0, a2, 5f
    sw      a1, 0(a0)
    addi    a0, a0, 4
    j       4b
5:  or      a2, a2, a3
    j       1f
    // fill tail
.Lmemset_tail_bytes:
    sb      a1, 0(a0)
    addi    a0, a0, 1
1:  bne     a0, a2, .Lmemset_tail_bytes
.Lmemset_end:
    ret
    .cfi_endproc
    .size memset, .-memset

.global __mmio_detect_trap
    .balign 0x20
__mmio_detect_trap:
    // regs assumptions:
    //   - a3 - pointer to store mcause
    //   - a4 - temporary
    csrr	a4, mcause
    save_xreg   a4, 0(a3)
    csrr	a4, mepc
    addi	a4, a4, 4
    csrw	mepc, a4
    mret

    .popsection

    .balign 0x20
__trap_hander:
    .cfi_startproc
    csrrw   sp, mscratch, sp
    save_regs 10, 11, sp
    csrr    a0, mcause
    bltz    a0, interrupt_handler

#if PLF_MMODE_ONLY
    j       handle_trap_in_machine_mode
#else // PLF_MMODE_ONLY

    li      a1, EXC_TLB_MISS
    beq     a1, a0, tlb_miss_trap_handler

    // hanlde supervisor syscalls SBI_SEND_IPI, SBI_SET_TIMER,...
#if PLF_IPI_ASM_HANDLERS
    li    a1, EXC_SM_ECALL
    bne   a1, a0, handle_trap_in_machine_mode

#if PLF_SMP_SUPPORT
    li    a1, SBI_SEND_IPI
    bne   a1, a7, .Lmtrap_scall_test2

    // scall handler: SBI_SEND_IPI
sbi_call_send_ipi:
    save_regs 12, 13, sp

    load_reg_idx 10, sp         // arg0 = hart_mask
#if PLF_HSM_SUPPORT
    // mask correction: only for ready harts
    .global harts_online
    load_addrword a3, harts_online
    load_xreg a3, 0(a3)
#else
    li    a3, ~0                // hart_mask:a3 = ~0
#endif // PLF_HSM_SUPPORT
#ifdef PAGE_OFFSET
    li    a2, (PAGE_OFFSET - PLF_MEM_BASE)
    sub   a0, a0, a2            // __pa(pmask):a0
#endif // PAGE_OFFSET
    //  check mem region bounds for linear Linux kernel memory translation
    li    a2, PLF_MEM_BASE
    bltu  a0, a2, .Lipi_send_masked
    li    a2, (PLF_MEM_BASE + PLF_MEM_SIZE)
    bgeu  a0, a2, .Lipi_send_masked
    load_reg_offs a3, 0, a0     // hart_mask:a3 = *__pa(pmask)
#if PLF_HSM_SUPPORT
    // mask correction: only for ready harts
    .global harts_online
    load_addrword a2, harts_online
    load_xreg a2, 0(a2)
    and a3, a3, a2
#endif // PLF_HSM_SUPPORT
.Lipi_send_masked:
    // prepare packed_msg = SCR_IPI_CMD_SIGNAL << (SCR_IPI_SEND_CSHIFT + slot_bits_shift)
    li    a1, (SCR_IPI_CMD_SIGNAL << SCR_IPI_SEND_CSHIFT)
#if !PLF_SMP_ICCM_SUPPORT
    read_pcrel_int32 a2, slot_bits_shift
    sll   a1, a1, a2            // packed_msg:a1 = msg:a1 << (SCR_IPI_SEND_CSHIFT + slot_bits_shift):a2
#endif // !PLF_SMP_ICCM_SUPPORT
    // loop for all harts
    read_pcrel_int32 a2, num_harts

    // prepare hart-mask for l-shift testing
    addi  a0, a2, (1 - __riscv_xlen)
    neg   a0, a0
    sll   a3, a3, a0            // MSB hart_mask:a3

    //  calc TLS addr
    sll   a0, a2, HLSHIFT
    load_addrword a2, __TLS0_BASE__
    sub   a2, a2, a0            // end TLS#

.Lipi_mask_loop:
    slli  a3, a3, 1             // hart_mask:a3 <<= 1
    li    a0, HLSIZE
    add   a2, a2, a0
    bgez  a3, .Lipi_mask_loop_continue // skip if cpumask.bit[i] == 0
    // process local IPI signal
    bne   a2, sp, 1f
    csrs  mip, (1 << INT_SM_SOFTWARE)
    j     .Lipi_mask_loop_continue
1:
    // send IPI
    // a0 = hls:a2.ipi_n
#ifndef __riscv_32e
    load_reg_offs a0, 32, a2
#else // ! __riscv_32e
    load_reg_offs a0, 16, a2
#endif // !__riscv_32e
    ipi_send_wait

.Lipi_mask_loop_continue:
    bnez  a3, .Lipi_mask_loop    // loop: hart_mask:a3 != 0

    // restore regs
    load_regs 12, 13, sp
    // do mret to mepc+4
#else // PLF_SMP_SUPPORT
    li    a1, SBI_REMOTE_SFENCE_VMA
    bne   a1, a7, 1f
    sfence.vma
    j     mtrap_scall_ret
1:
    li    a1, SBI_REMOTE_SFENCE_VMA_ASID
    bne   a1, a7, 1f
    sfence.vma
    j     mtrap_scall_ret
1:
    li    a1, SBI_REMOTE_FENCE_I
    bne   a1, a7, .Lmtrap_scall_test2
    fence.i
#endif // PLF_SMP_SUPPORT

mtrap_scall_ret:
    csrr  a1, mepc
    addi  a1, a1, 4
    csrw  mepc, a1
    li    a0, SBI_ERROR_SUCCESS
    load_reg_idx 11, sp
	csrrw sp, mscratch, sp
	mret

.Lmtrap_scall_test2:
    li    a1, SBI_SET_TIMER
    bne   a1, a7, .Lmtrap_scall_test3

    // scall handler: SBI_SET_TIMER
    // rv32 input: regs[a0]= low(when), regs[a1]= high(when)
    // rv64 input: regs[a0]= when
sbi_call_set_timer:
#if __riscv_xlen == 32
    li    a7, PLF_MTIMER_BASE
    load_reg_idx 10, sp
    li    a1, ~0
    sw    a1, SCR_RTC_CMPH_OFF(a7)
    load_reg_idx 11, sp
    sw    a0, SCR_RTC_CMP_OFF(a7)
    sw    a1, SCR_RTC_CMPH_OFF(a7)
    li    a7, SBI_SET_TIMER     // restore sbi func code
#endif // __riscv_xlen == 32
.Lsbi_call_timer_ret:
#if __riscv_xlen > 32
    li    a1, PLF_MTIMER_BASE
    load_reg_idx 10, sp
    sd    a0, SCR_RTC_CMP_OFF(a1)
#endif // __riscv_xlen > 32
    // clear STIP
    li    a1, (1 << INT_SM_TIMER)
    csrc  mip, a1
    // enable mtimer interrupts
    li    a1, (1 << INT_MM_TIMER)
    csrs  mie, a1
    j     mtrap_scall_ret

.Lmtrap_scall_test3:
    // SBI v0.2 timer: fn.0 = set
    li    a1, SBI_EXT_TIME
    bne   a1, a7, .Lmtrap_scall_test4
sbi02_call_set_timer:
    bnez  a6, sbi02_call_ret_err
    // input: regs[a0]= low(when), regs[a1]= high(when)
#if __riscv_xlen == 32
    li    a7, PLF_MTIMER_BASE
    load_reg_idx 10, sp
    li    a1, ~0
    sw    a1, SCR_RTC_CMPH_OFF(a7)
    load_reg_idx 11, sp
    sw    a0, SCR_RTC_CMP_OFF(a7)
    sw    a1, SCR_RTC_CMPH_OFF(a7)
    li    a7, SBI_EXT_TIME      // restore sbi func code
#endif // __riscv_xlen == 32
    j     .Lsbi_call_timer_ret

.Lmtrap_scall_test4:
#if PLF_SMP_SUPPORT
    // SBI v0.2 ipi send: fn.0 = set
    li    a1, SBI_EXT_IPI
    bne   a1, a7, .Lmtrap_scall_test5

sbi02_call_send_ipi:
    bnez  a6, sbi02_call_ret_err
    load_reg_idx 11, sp // arg1 = hart_offs
    li    a0, __riscv_xlen
    // not supported logical hart# >= __riscv_xlen (32 or 64)
    // if mask_offs >= __riscv_xlen, do nothing
    bge   a1, a0, mtrap_scall_ret
    save_regs 12, 13, sp
#if PLF_HSM_SUPPORT
    // mask correction: only for ready harts
    .global harts_online
    load_addrword a3, harts_online
    load_xreg a3, 0(a3)
#else
    li    a3, ~0      // hart_mask:a3 = ~0 - all harts
#endif // PLF_HSM_SUPPORT
    bltz  a1, .Lipi_send_masked
    load_reg_idx 10, sp // arg0 = hart_mask
    sll   a0, a0, a1    // apply harts_online correction if possible
    and   a3, a3, a0
    j     .Lipi_send_masked
#endif // PLF_SMP_SUPPORT

.Lmtrap_scall_test5:
    li    a1, SBI_CLEAR_IPI
    bne   a1, a7, .Lmtrap_scall_test6
sbi_call_clear_ipi:
    csrc  mip, (1 << INT_SM_SOFTWARE)
    j     mtrap_scall_ret

sbi02_call_ret_err:
    csrr  a1, mepc
    addi  a1, a1, 4
    csrw  mepc, a1
    li    a0, SBI_ERROR_NOT_SUPPORTED
    load_reg_idx 11, sp
	csrrw sp, mscratch, sp
	mret

.Lmtrap_scall_test6:
#endif // PLF_IPI_ASM_HANDLERS

    j     handle_trap_in_machine_mode

#endif // PLF_MMODE_ONLY
    .cfi_endproc
    .size __trap_hander, .-__trap_hander

    .section ".reloc_l3_fixup", "ax", @progbits

#ifdef PLF_L3CTL_BASE
_crt_wrong_start:
    li t1, WRONG_STARTUP_ADDRESS_POISON
    load_addrword_pcrel t0, wrong_startup_address
    save_xreg t1, (t0)
#endif

_crt_start:
#ifdef PLF_INIT_REGS
    mv x1, zero
    mv x2, zero
    mv x3, zero
    mv x4, zero
    mv x5, zero
    mv x6, zero
    mv x7, zero
    mv x8, zero
    mv x9, zero
    mv x10, zero
    mv x11, zero
    mv x12, zero
    mv x13, zero
    mv x14, zero
    mv x15, zero
#ifndef __riscv_32e
    mv x16, zero
    mv x17, zero
    mv x18, zero
    mv x19, zero
    mv x20, zero
    mv x21, zero
    mv x22, zero
    mv x23, zero
    mv x24, zero
    mv x25, zero
    mv x26, zero
    mv x27, zero
    mv x28, zero
    mv x29, zero
    mv x30, zero
    mv x31, zero
#endif // !__riscv_32e
#endif // PLF_INIT_REGS

    // reset mstatus
    csrw  mstatus, zero
    // split master/slave
    csrr  a0, mhartid
#if PLF_MASTER_HART == 0
#define MASTER_HART_REG zero
#else
    li    t0, PLF_MASTER_HART
#define MASTER_HART_REG t0
#endif // PLF_MASTER_HART == 0
#if PLF_SMP_SUPPORT
    bne   a0, MASTER_HART_REG, secondary_reset_wait
#else
    bne   a0, MASTER_HART_REG, smp_hart_halt
#endif // PLF_SMP_SUPPORT
#undef MASTER_HART_REG
    // auto relocation
#ifdef PLF_L3CTL_BASE

    load_addrword_abs t0, __SCBL_RELOC_FLAG
    ld t0, (t0)
    li t1, 0xDDDDDDDDDDDDDDDD
    bne t0, t1, .Ldo_relocation

    load_addrword_abs t0, _crt_start_relocated
    jr t0

.Ldo_relocation:
    load_addrword_abs t1, __SCBL_START
    load_addrword_pcrel t0, __SCBL_START
    beq   t0, t1, _crt_start_relocated

    load_addrword_pcrel t2, __SCBL_END
    j _relocate
#else
    load_addrword_abs t1, _crt_start
    load_addrword_pcrel t0, _crt_start
    beq   t0, t1, _crt_start_relocated
    // align at 64K
    srli  t1, t1, 16
    srli  t0, t0, 16
    addi  t2, t0, 1
    slli  t1, t1, 16
    slli  t0, t0, 16
    slli  t2, t2, 16
    mv    s0, t1
    memcpy t0, t2, t1, tp
    li    t1, 64 * 1024
    cache_l1l2_flush s0, t1
    // jump to relocated code
    load_addrword_abs t0, _crt_start_relocated
    jr    t0
#endif

#if PLF_SMP_SUPPORT
secondary_reset_wait:
    // setup mtvec
    load_addrword_abs t0, secondary_reset
    csrw  mtvec, t0
    // clear IPI mailbox
    ipi_clear_mailbox
    // enable IPI
    csrw  mip, zero
    csrw  mie, (1 << INT_MM_SOFTWARE)
    csrw  mstatus, (1 << 3)
    j     .Lsmp_hart_halt_loop
#endif

# halt hart
smp_hart_halt:
    // disable timer interrupts
    li    t0, (1 << INT_MM_TIMER)
    csrc  mie, t0
.Lsmp_hart_halt_loop:
    wfi
    j     .Lsmp_hart_halt_loop

#ifdef PLF_L3CTL_BASE
_relocate:
    mv    s0, t1
    sub s1, t2, t0
    // memcpy t0, t2, t1, tp

    sub t3, t2, t0
    add t1, t1, t3
    j _relocate_loop_cond

_relocate_loop:
    add t1, t1, -xreg_len
    add t2, t2, -xreg_len
    load_xreg tp, (t2)
    save_xreg tp, (t1)

_relocate_loop_cond:
    bltu t0, t2, _relocate_loop

    cache_l1l2_flush s0, s1
    // jump to relocated code
    load_addrword_abs t0, _crt_start_relocated
    jr    t0
#endif

    .section ".reset_vector","ax",@progbits
    .option push
    .option norelax
    .option norvc
_start:
    j     _crt_start
#if PLF_MMODE_ONLY == 0
_start_kernel:
    j     kernel_start
#endif // !PLF_MMODE_ONLY
    .fill 14, 4, 0 // offset 0x40
    // Entry point for NMIs
__nmi_trap_entry:
    // Not implemented
    j bad_trap
    .fill 15, 4, 0 // offset 0x80
    // TRAP default entry
__trap_default_entry:
    j __trap_hander
    .option pop

#ifdef PLF_L3CTL_BASE
.section ".wrong_reset_vector", "ax", @progbits
.option push
.option norelax
.option norvc
_wrong_start:
    j     _crt_wrong_start
.option pop
#endif
