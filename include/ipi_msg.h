/*
 * Copyright (C) 2023, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* framework
///
/// @copyright Copyright (C) 2023, Syntacore Ltd. All Rights Reserved.
/// @author mn-sc, dz-sc
///
/// @brief IPI messaging defs and inline funcs

#ifndef SCR_INFRA_IPI_MSG_H
#define SCR_INFRA_IPI_MSG_H

#include "platform_config.h"

#if PLF_SMP_ICCM_SUPPORT
// ICCM command's structure
#define SCR_IPI_CMD_BITS   5 // MSB in received msg
#define SCR_IPI_REG_SZ     32
#define SCR_IPI_DATA_BITS  (SCR_IPI_REG_SZ - SCR_IPI_CMD_BITS - 1)
#define SCR_IPI_DATA_MASK (((1u) << SCR_IPI_DATA_BITS) - 1)

#define SCR_IPI_SEND_CSHIFT    (SCR_IPI_DATA_BITS)

#define PACK_SCIPI_MSG(cmd, data) ((((cmd) + (0u)) << SCR_IPI_SEND_CSHIFT) | ((data) & SCR_IPI_DATA_MASK))
#define EXTRACT_SCIPI_MSG_CMD(msg) (((msg) + (0u)) >> SCR_IPI_DATA_BITS)
#define EXTRACT_SCIPI_MSG_DATA(msg) (((msg) + (0u)) & SCR_IPI_DATA_MASK)

#else // PLF_SMP_ICCM_SUPPORT

// IPI command's structure
#define SCR_IPI_SLOT_BITS 4 // LSB in received message

#define SCR_IPI_CMD_BITS  5 // MSB in received msg (excluding status bit)
#define SCR_IPI_REG_SZ    __riscv_xlen
#define SCR_IPI_DATA_BITS (SCR_IPI_REG_SZ - SCR_IPI_SLOT_BITS - SCR_IPI_CMD_BITS - 1)
#define SCR_IPI_DATA_MASK ((_UL(1) << SCR_IPI_DATA_BITS) - 1)

#define SCR_IPI_SEND_CSHIFT    (SCR_IPI_DATA_BITS)

#define SCR_IPI_RECV_SRC_BITS  (SCR_IPI_SLOT_BITS)
#define SCR_IPI_RECV_SRC_MASK  ((1 << SCR_IPI_RECV_SRC_BITS) - 1)
#define SCR_IPI_RECV_CSHIFT    (SCR_IPI_DATA_BITS + SCR_IPI_RECV_SRC_BITS)

#define PACK_SCIPI_MSG(cmd, data) ((((cmd) + _UL(0)) << SCR_IPI_SEND_CSHIFT) | ((data) & SCR_IPI_DATA_MASK))
#define EXTRACT_SCIPI_MSG_CMD(msg) (((msg) + _UL(0)) >> SCR_IPI_RECV_CSHIFT)
#define EXTRACT_SCIPI_MSG_DATA(msg) ((((msg) + _UL(0)) >> SCR_IPI_RECV_SRC_BITS) & SCR_IPI_DATA_MASK)

#endif // PLF_SMP_ICCM_SUPPORT

// setup: setup HLS
// arg:  HLS page + 1 == stack top (low 20 bit)
#define SCR_IPI_CMD_SETUP     1
// arg: none
#define SCR_IPI_CMD_START     2
// reset: exec after-reset procedure
#define SCR_IPI_CMD_RESET     3
// signal hart
#define SCR_IPI_CMD_SIGNAL    4
#define SCR_IPI_CMD_FENCE_I   5
#define SCR_IPI_CMD_MSG_BASE  8

#define SCIPI_MSG_SIZE_LOG  6 // == 64
#define SCIPI_MSG_SIZE      (1 << SCIPI_MSG_SIZE_LOG)
#define SCIPI_MSG_POOL_SIZE 4 // PLF_SMP_HART_NUM

#ifndef __ASSEMBLER__
#include "spinlock.h"
// ICCM message API

typedef struct ipi_rfence_msg_ {
    uintptr_t addr;
    unsigned long size;
    unsigned long asid; // ASID/VMID/...
} ipi_rfence_msg;

typedef struct ipi_hsm_msg_ {
    uintptr_t addr;
    unsigned long priv;
} ipi_hsm_msg;

typedef struct ipi_msg_ {
    sc_atomic shared_cnt;
    uint32_t pad1;
    uintptr_t handler_msg; // msg: 0 - 0xfff, handler: 0x1000 - max_addr
#if __riscv_xlen == 32
    uint32_t pad0;
#endif // __riscv_xlen == 32
    union {
        uint64_t data[(SCIPI_MSG_SIZE - 16) / 8];
        ipi_rfence_msg rfence;
        ipi_hsm_msg hsm;
    } payload;
} ipi_msg;

typedef void (*ipi_msg_handler)(ipi_msg *msg);

SC_STATIC_ASSERT(sizeof(ipi_msg) == SCIPI_MSG_SIZE, ipi_msg_size_mismatch);
SC_STATIC_ASSERT((1UL << SCR_IPI_CMD_BITS) - SCR_IPI_CMD_MSG_BASE >= SCIPI_MSG_POOL_SIZE, ipi_msg_pool_size_mismatch);

void ipi_free_msg(ipi_msg *msg);
void ipi_share_msg(ipi_msg *msg);
ipi_msg *ipi_alloc_msg(void);
void ipi_send_msg(unsigned long hart_mask, ipi_msg *msg);
void ipi_send_signal(unsigned long hart_mask, unsigned long sig);

#else // __ASSEMBLER__

#if __riscv_xlen <= 64
#define SCIPI_RFENCE_MSG_ADDR_OFF 0
#define SCIPI_RFENCE_MSG_SIZE_OFF (__riscv_xlen / 8)
#define SCIPI_RFENCE_MSG_ASID_OFF (__riscv_xlen / 8 + __riscv_xlen / 8)
#define SCIPI_HSM_MSG_ADDR_OFF    0
#define SCIPI_HSM_MSG_PRIV_OFF    (__riscv_xlen / 8)
#else // __riscv_xlen <= 64
#error unsupported __riscv_xlen
#endif // __riscv_xlen <= 64

#define SCIPI_MSG_CNT_OFF     0
#define SCIPI_MSG_HANDLER_OFF 8
#define SCIPI_MSG_PAYLOAD_OFF 16
#define SCIPI_MSG_RFENCE_ADDR_OFF (SCIPI_MSG_PAYLOAD_OFF + SCIPI_RFENCE_MSG_ADDR_OFF)
#define SCIPI_MSG_RFENCE_SIZE_OFF (SCIPI_MSG_PAYLOAD_OFF + SCIPI_RFENCE_MSG_SIZE_OFF)
#define SCIPI_MSG_RFENCE_ASID_OFF (SCIPI_MSG_PAYLOAD_OFF + SCIPI_RFENCE_MSG_ASID_OFF)
#define SCIPI_MSG_HSM_ADDR_OFF    (SCIPI_MSG_PAYLOAD_OFF + SCIPI_HSM_MSG_ADDR_OFF)
#define SCIPI_MSG_HSM_PRIV_OFF    (SCIPI_MSG_PAYLOAD_OFF + SCIPI_HSM_MSG_PRIV_OFF)

#if PLF_SMP_ICCM_SUPPORT

.macro ipi_wait_cmd cmd
1:
    wfi
    csrr  a1, mip
    andi  a1, a1,  (1 << INT_MM_SOFTWARE)
    beqz  a1, 1b
    // Check buffer is not empty
    li    t0, SCR_ICCM_SEG_A_BUFSTATUS
    lw    t1, 0(t0)
    andi  t1, t1, 1
    beqz  t1, 1b
    // Read buffer and compare
    li    t0, SCR_ICCM_SEG_A_BUFREAD
    lw    a1, 0(t0)
    srli  a3, a1, SCR_IPI_SEND_CSHIFT
    li    a2, \cmd
    bne   a3, a2, 1b
.endm

.macro ipi_get_hls_addr
    li    t0, SCR_IPI_DATA_MASK
    and   a1, a1, t0
    lui   t0, %hi(__C_STACK_TOP__);
    srli  t0, t0, HLSHIFT
    sub   a1, t0, a1
    slli  a1, a1, HLSHIFT               // page to phys addr translation
.endm

.macro ipi_read_cmd
    li    a1, SCR_ICCM_SEG_A_BUFREAD
    lw    a0, 0(a1)
    srli  a1, a0, SCR_IPI_SEND_CSHIFT
.endm

#else // PLF_SMP_ICCM_SUPPORT

.macro ipi_wait_cmd cmd
1:
    wfi
    csrr  a1, mip
    andi  a1, a1,  (1 << INT_MM_SOFTWARE)
    beqz  a1, 1b
    csrrwi a1, SCR_CSR_IPI_MRDATA, 1
    srli  a3, a1, SCR_IPI_RECV_CSHIFT
    li    a2, \cmd
    bne   a3, a2, 1b
.endm

.macro ipi_get_hls_addr
    srli  a1, a1, SCR_IPI_RECV_SRC_BITS // shift IPI src addr field
    slli  a1, a1, HLSHIFT               // page to phys addr translation
.endm

.macro ipi_read_cmd
    csrrw a0, SCR_CSR_IPI_MRDATA, 1
    srli  a1, a0, SCR_IPI_RECV_CSHIFT
.endm

#endif // PLF_SMP_ICCM_SUPPORT

#endif // __ASSEMBLER__

#endif // SCR_INFRA_IPI_MSG_H
