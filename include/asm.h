/*
 * Copyright (C) 2015, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* infra
///
/// @copyright Copyright (C) 2015-2020, Syntacore Ltd. All Rights Reserved.
/// @author mn-sc
///
/// @brief useful assembler macros

#ifndef SCR_ASM_H
#define SCR_ASM_H

#ifdef __ASSEMBLER__

.altmacro

#if __riscv_xlen == 32
#define xreg_len 4
#define load_xreg lw
#define save_xreg sw
#elif __riscv_xlen == 64 // __riscv_xlen
#define xreg_len 8
#define load_xreg ld
#define save_xreg sd
#elif __riscv_xlen == 128 // __riscv_xlen
#define xreg_len 16
#define load_xreg lq
#define save_xreg sq
#else // __riscv_xlen
#error load_xreg: unsupported or unknown __riscv_xlen
#endif // __riscv_xlen

.macro memop_reg_offs memop, reg, offs, mem_base=zero
    \memop \reg, \offs(\mem_base)
.endm

.macro save_reg_offs reg, offs, mem_base=zero
    memop_reg_offs save_xreg, \reg, \offs*xreg_len, \mem_base
.endm

.macro load_reg_offs reg, offs, mem_base=zero
    memop_reg_offs load_xreg, \reg, \offs*xreg_len, \mem_base
.endm

.macro save_reg_idx regn, save_mem_base=zero
    save_reg_offs x\regn, \regn, \save_mem_base
.endm

.macro load_reg_idx regn, load_mem_base=zero
    load_reg_offs x\regn, \regn, \load_mem_base
.endm

.macro save_regs reg_first, reg_last, save_mem_base=zero
    LOCAL regn
    regn = \reg_first
    .rept \reg_last - \reg_first + 1
    save_reg_idx %(regn), \save_mem_base
    regn = regn+1
    .endr
.endm

.macro load_regs reg_first, reg_last, load_mem_base=zero
    LOCAL regn
    regn = \reg_first
    .rept \reg_last - \reg_first + 1
    load_reg_idx %(regn), \load_mem_base
    regn = regn+1
    .endr
.endm

.macro memcpy src_beg, src_end, dst, tmp_reg
    LOCAL memcpy_1, memcpy_2
    j     memcpy_2
    memcpy_1:
    load_xreg \tmp_reg, (\src_beg)
    save_xreg \tmp_reg, (\dst)
    add   \src_beg, \src_beg, xreg_len
    add   \dst, \dst, xreg_len
memcpy_2:
    bltu  \src_beg, \src_end, memcpy_1
.endm

.macro memset dst_beg, dst_end, val_reg
    LOCAL memset_1, memset_2
    j     memset_2
memset_1:
    save_xreg \val_reg, (\dst_beg)
    add   \dst_beg, \dst_beg, xreg_len
memset_2:
    bltu  \dst_beg, \dst_end, memset_1
.endm

.macro read_pcrel_int32 reg, sym
    LOCAL pcrel_addr
    .option push
    .option norelax
pcrel_addr:
    auipc \reg, %pcrel_hi(\sym)
    lw    \reg, %pcrel_lo(pcrel_addr)(\reg)
    .option pop
.endm // read_pcrel_int32

.macro write_pcrel_int32 reg, sym
    LOCAL pcrel_addr
    .option push
    .option norelax
pcrel_addr:
    auipc \reg, %pcrel_hi(\sym)
    sw    \reg, %pcrel_lo(pcrel_addr)(\reg)
    .option pop
.endm // write_pcrel_int32

.macro read_pcrel_addrword reg, sym
    LOCAL pcrel_addr
    .option push
    .option norelax
pcrel_addr:
    auipc \reg, %pcrel_hi(\sym)
    load_xreg \reg, %pcrel_lo(pcrel_addr)(\reg)
    .option pop
.endm // read_pcrel_addrword

.macro write_pcrel_addrword reg, sym
    LOCAL pcrel_addr
    .option push
    .option norelax
pcrel_addr:
    auipc \reg, %pcrel_hi(\sym)
    save_xreg \reg, %pcrel_lo(pcrel_addr)(\reg)
    .option pop
.endm // write_pcrel_addrword

// load 32 bit (sign extended) constant
.macro load_const_int32 reg, sym
    .option push
    .option norelax
    lui \reg, %hi(\sym)
    addi \reg, \reg, %lo(\sym)
    .option pop
.endm // load_const_int32

.macro load_addrword_abs reg, sym
#if __riscv_xlen == 32
    load_const_int32 \reg, \sym
#elif __riscv_xlen == 64
    LOCAL _addrword
    .subsection 8 // subsection for local constants
    .align 3
_addrword:
    .dword \sym
    .previous
     read_pcrel_addrword \reg, _addrword
#else // __riscv_xlen
#error load_addrword_abs is not implemented for xlen
#endif // __riscv_xlen
.endm // load_addrword_abs

.macro load_addrword_pcrel reg, sym
    LOCAL _pcrel_addr
    .option push
    .option norelax
_pcrel_addr:
    auipc \reg, %pcrel_hi(\sym)
    addi \reg, \reg, %pcrel_lo(_pcrel_addr)
    .option pop
.endm // load_addrword_pcrel

.macro load_addrword reg, sym
#if __riscv_xlen == 32
    load_addrword_abs \reg, \sym
#elif __riscv_xlen == 64
#if PLF_SPARSE_MEM
    load_addrword_abs \reg, \sym
#else // PLF_SPARSE_MEM
    load_addrword_pcrel \reg, \sym
#endif // PLF_SPARSE_MEM
#else // __riscv_xlen
#error load_addrword is not implemented for xlen
#endif // __riscv_xlen
.endm // load_addrword

#endif // __ASSEMBLER__

#endif // SCR_ASM_H
