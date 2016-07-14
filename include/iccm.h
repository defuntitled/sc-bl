/*
 * Copyright (C) 2023, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* framework
///
/// @copyright Copyright (C) 2023, Syntacore Ltd. All Rights Reserved.
/// @author gdi-sc, dz-sc
///
/// @brief L3 cluster ICCM defs and inline funcs

#ifndef SCR_INFRA_ICCM_H
#define SCR_INFRA_ICCM_H

#include "platform_config.h"

#if PLF_SMP_SUPPORT && PLF_SMP_ICCM_SUPPORT

/* ICCM CSRs */
/* SEGMENT A */
#define SCR_ICCM_SEG_A_BASE                         (PLF_ICCM_BASE)
#define SCR_ICCM_SEG_A_BUFSTATUS                    (SCR_ICCM_SEG_A_BASE + 0)
#define SCR_ICCM_SEG_A_BUFREAD                      (SCR_ICCM_SEG_A_BASE + 4)
#define SCR_ICCM_SEG_A_SNDSTAT_E_LO                 (SCR_ICCM_SEG_A_BASE + 0x3F0)
#define SCR_ICCM_SEG_A_SNDSTAT_E_HI                 (SCR_ICCM_SEG_A_BASE + 0x3F4)
#define SCR_ICCM_SEG_A_SNDSTAT_0_LO                 (SCR_ICCM_SEG_A_BASE + 0x400)
#define SCR_ICCM_SEG_A_SNDSTAT_0_HI                 (SCR_ICCM_SEG_A_BASE + 0x404)
#define SCR_ICCM_SEG_A_SNDSTAT_N_LO(N)              (SCR_ICCM_SEG_A_SNDSTAT_0_LO + (N) * 16)
#define SCR_ICCM_SEG_A_SNDSTAT_N_HI(N)              (SCR_ICCM_SEG_A_SNDSTAT_0_HI + (N) * 16)
#define SCR_ICCM_SEG_A_BUFWRITE_0                   (SCR_ICCM_SEG_A_BASE + 0xC00)
#define SCR_ICCM_SEG_A_BUFWRITE_N(N)                (SCR_ICCM_SEG_A_BUFWRITE_0 + (N) * 4)

/* SEGMENT B */
#define SCR_ICCM_SEG_B_BASE                         (PLF_ICCM_BASE + 0x1000)
#define SCR_ICCM_SEG_B_ICCM_VERSION                 (SCR_ICCM_SEG_B_BASE + 0x0)
#define SCR_ICCM_SEG_B_ICCM_HARTS                   (SCR_ICCM_SEG_B_BASE + 0x4)
#define SCR_ICCM_SEG_B_ICCM_CONTROL                 (SCR_ICCM_SEG_B_BASE + 0x8)
#define SCR_ICCM_SEG_B_ICCM_CLEAR_LO                (SCR_ICCM_SEG_B_BASE + 0x10)
#define SCR_ICCM_SEG_B_ICCM_CLEAR_HI                (SCR_ICCM_SEG_B_BASE + 0x14)

#define SCR_ICCM_BUFSTATUS_FULL_BIT                 (1 << 0)

#define SCR_ICCM_BUFSTATUS_SENDERID_OFFS            (16)
#define SCR_ICCM_BUFSTATUS_SENDERID_MASK            (0xFF << SCR_ICCM_BUFSTATUS_SENDERID_OFFS)

#define SCR_ICCM_BUFSTATUS_OWNID_OFFS               (24)
#define SCR_ICCM_BUFSTATUS_OWNID_MASK               (0xFF << SCR_ICCM_BUFSTATUS_OWNID_OFFS)

#define SCR_ICCM_ICCM_CONTROL_EWE_BIT               (1 << 0)
#define SCR_ICCM_ICCM_CONTROL_ERE_BIT               (1 << 1)
#define SCR_ICCM_ICCM_CONTROL_EEWF_BIT              (1 << 2)
#define SCR_ICCM_ICCM_CONTROL_EIWF_BIT              (1 << 3)

#ifndef __ASSEMBLER__
#include "arch.h"
#include <stdint.h>
#include <stdbool.h>

typedef uint64_t ipi_data_t;

//-----------------------------------------------------------------------------
// Functions specific to new iccm
//-----------------------------------------------------------------------------

/**
 * \brief Returns iccm id of a core (reads OWNID field in BUFSTATUS register)
 *
 * \return iccm id of a core
 */
static inline unsigned iccm_get_own_id(void) {
    uint32_t bufstatus = *((volatile uint32_t *) SCR_ICCM_SEG_A_BUFSTATUS);
    return ((bufstatus & SCR_ICCM_BUFSTATUS_OWNID_MASK) >> SCR_ICCM_BUFSTATUS_OWNID_OFFS);
}

/**
 * \brief Check if receiver is busy for a particular sender (i.e. receiver has unread message from sender in its buffer)
 *
 * \param sender_id iccm id of a sender
 * \param receiver_id iccm id of a receiver
 * \return true: receiver is busy; false: receiver is ready to accept new message from sender
 */
static inline bool iccm_receiver_busy(unsigned sender_id, unsigned receiver_id)
{
    volatile uint64_t *p_sndstat = (volatile uint64_t *) SCR_ICCM_SEG_A_SNDSTAT_N_LO(sender_id);
    return (*p_sndstat >> receiver_id) & 0x1;
}

/**
 * \brief Check if receiver is busy for a external sender (i.e. receiver has unread message from sender in its buffer)
 *
 * \param receiver_id iccm id of a receiver
 * \return true: receiver is busy; false: receiver is ready to accept new message from sender
 */
static inline bool iccm_receiver_external_busy(unsigned receiver_id)
{
    volatile uint64_t *p_sndstat = (volatile uint64_t *) SCR_ICCM_SEG_A_SNDSTAT_E_LO;
    return (*p_sndstat >> receiver_id) & 0x1;
}

/**
 * \brief Write to register control
 *
 * \param set_control register value to set
 */
static inline void iccm_write_b_control(uint32_t set_control)
{
    *(volatile uint32_t *) SCR_ICCM_SEG_B_ICCM_CONTROL = set_control;
}

/**
 * \brief Checking that the register clearing bits are reset to zero
 *
 * \param receiver_id iccm id of a receiver
 * \return 1: buffer not clear; 0: buffer clear
 */
static inline int iccm_read_clear_reg(unsigned receiver_id)
{
    volatile uint64_t *p_sndstat = (volatile uint64_t *) SCR_ICCM_SEG_B_ICCM_CLEAR_LO;
    return (*p_sndstat >> receiver_id) & 0x1;
}

//-----------------------------------------------------------------------------
// Common iccm API implementation
//-----------------------------------------------------------------------------

static inline unsigned ipi_get_num_slots(void)
{
    return *((volatile uint32_t *) SCR_ICCM_SEG_B_ICCM_HARTS);
}

static inline bool ipi_msg_avail(void)
{
    volatile uint32_t *p_bufstatus = (volatile uint32_t *) SCR_ICCM_SEG_A_BUFSTATUS;
    return *p_bufstatus & SCR_ICCM_BUFSTATUS_FULL_BIT;
}

static inline void ipi_send_data(unsigned receiver, ipi_data_t data)
{
    *(volatile uint32_t *) SCR_ICCM_SEG_A_BUFWRITE_N(receiver) = data;
}

static inline bool ipi_receiver_busy(unsigned receiver)
{
    return iccm_receiver_busy(iccm_get_own_id(), receiver);
}

static inline ipi_data_t ipi_read_data(void)
{
    return *(volatile uint32_t *) SCR_ICCM_SEG_A_BUFREAD;
}

static inline unsigned ipi_get_sender_id(void)
{
    volatile uint32_t *p_bufstatus = (volatile uint32_t *) SCR_ICCM_SEG_A_BUFSTATUS;
    return (*p_bufstatus & SCR_ICCM_BUFSTATUS_SENDERID_MASK) >> SCR_ICCM_BUFSTATUS_SENDERID_OFFS;
}

#else // __ASSEMBLER__

.macro ipi_clear_mailbox
    // Get iccm id of a core
    li    t1, SCR_ICCM_SEG_A_BUFSTATUS
    ld    t1, 0(t1)
    li    t0, SCR_ICCM_BUFSTATUS_OWNID_MASK
    and   t1, t1, t0
    srli  t1, t1, SCR_ICCM_BUFSTATUS_OWNID_OFFS
    // (1 << core_id)
    li    t0, 1
    sll   t1, t0, t1
    // write bit to buffer clearing register
    li    t0, SCR_ICCM_SEG_B_ICCM_CLEAR_LO
    sd    t1, 0(t0)
.endm

.macro ipi_send_wait
.Lipi_status_wait:
    // Wait if receiver is busy
    li    t0, SCR_ICCM_SEG_A_SNDSTAT_0_LO // The address of SNDSTAT_0_LO in t0
    // Read our ipi_n from hls
    // t1 = hls:sp.ipi_n
#ifndef __riscv_32e
    load_reg_offs t1, 32, sp
#else // ! __riscv_32e
    load_reg_offs t1, 16, sp
#endif // !__riscv_32e
    slli  t2, t1, 4     // Sender ID * 16 in t2
    add   t0, t0, t2    // address of SNDSTAT_0 + (Sender ID * 16) in t0
    ld    t2, 0(t0)     // load SNDSTAT_N in t2
    li    t3, 1
    sll   t3, t3, a0    // Create mask (1 << hls:a2.ipi_n (Receiver ID)) in t3
    and   t2, t2, t3    // Check REceiver's bit in SNDSTAT_N
    bnez  t2, .Lipi_status_wait  // wait until available

    li    t0, SCR_ICCM_SEG_A_BUFWRITE_0
    slli  t2, a0, 2     // Receiver ID * 4 in t2
    add   t0, t0, t2    // address of BUFWRITE_0 + (Receiver ID * 4) in t0
    sw    a1, 0(t0)     // write msg to BUFWRITE_N
.endm

#endif // __ASSEMBLER__

#endif // PLF_SMP_SUPPORT && PLF_SMP_ICCM_SUPPORT
#endif  /// #ifndef SCR_INFRA_ICCM_H
