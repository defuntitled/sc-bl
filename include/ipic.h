/*
 * Copyright (C) 2015, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* framework
///
/// @copyright Copyright (C) 2015-2016, Syntacore Ltd. All Rights Reserved.
/// @author mn-sc
///
/// @brief IPIC defs and inline funcs

#ifndef SCR_IPIC_H
#define SCR_IPIC_H

#define SCR_IPIC_MBASE   0xbf0
#define SCR_IPIC_SBASE   0x9f0

#define IRQ_PENDING           (1 << 0)
#define IRQ_ENABLE            (1 << 1)
#define IRQ_LEVEL             (0 << 2)
#define IRQ_EDGE              (1 << 2)
#define IRQ_INV               (1 << 3)
#define IRQ_MODE_MASK         (3 << 2)
#define IRQ_CLEAR_PENDING     IRQ_PENDING

#define IRQ_TYPE_EDGE_RISING  IRQ_EDGE
#define IRQ_TYPE_EDGE_FALLING (IRQ_EDGE | IRQ_INV)
#define IRQ_TYPE_LEVEL_HIGH   IRQ_LEVEL
#define IRQ_TYPE_LEVEL_LOW    (IRQ_LEVEL | IRQ_INV)

#define IRQ_IN_SERVICE (1 << 4) // RO
#define IRQ_PRIV_MASK  (3 << 8)
#define IRQ_PRIV_MMODE (3 << 8)
#define IRQ_PRIV_SMODE (1 << 8)
#define IRQ_LN_OFFS    12
#define IRQ_LN_NUM     32
#define IRQ_LN_VOID    IRQ_LN_NUM

#define IRQ_VEC_NUM    32
#define IRQ_VEC_VOID   IRQ_VEC_NUM

// IPIC regs
// M-Mode
#define IPIC_CISV   (SCR_IPIC_MBASE + 0)
#define IPIC_CICSR  (SCR_IPIC_MBASE + 1)
#define IPIC_IPR    (SCR_IPIC_MBASE + 2)
#define IPIC_ISVR   (SCR_IPIC_MBASE + 3)
#define IPIC_EOI    (SCR_IPIC_MBASE + 4)
#define IPIC_SOI    (SCR_IPIC_MBASE + 5)
#define IPIC_IDX    (SCR_IPIC_MBASE + 6)
#define IPIC_ICSR   (SCR_IPIC_MBASE + 7)
#define IPIC_IER    (SCR_IPIC_MBASE + 8)
#define IPIC_IMAP   (SCR_IPIC_MBASE + 9)
// S-Mode
#define IPIC_SCISV  (SCR_IPIC_SBASE + 0)
#define IPIC_SCICSR (SCR_IPIC_SBASE + 1)
#define IPIC_SIPR   (SCR_IPIC_SBASE + 2)
#define IPIC_SISVR  (SCR_IPIC_SBASE + 3)
#define IPIC_SEOI   (SCR_IPIC_SBASE + 4)
#define IPIC_SSOI   (SCR_IPIC_SBASE + 5)
#define IPIC_SIDX   (SCR_IPIC_SBASE + 6)
#define IPIC_SICSR  (SCR_IPIC_SBASE + 7)
#define IPIC_SIER   (SCR_IPIC_SBASE + 8)
#define IPIC_SIMAP  (SCR_IPIC_SBASE + 9)

#define MK_IRQ_CFG(line, mode, flags) ((mode) | (flags) | ((line) << IRQ_LN_OFFS))

#ifndef __ASSEMBLER__

#include "arch.h"
#include "platform_config.h"

static inline void irq_setup(unsigned irq_num, unsigned line, unsigned mode, unsigned flags)
{
    write_csr(IPIC_IDX, irq_num);
    write_csr(IPIC_ICSR, MK_IRQ_CFG(line, mode, flags | IRQ_CLEAR_PENDING));
}

static inline void irq_reset(unsigned irq_num)
{
    irq_setup(irq_num, IRQ_LN_VOID, IRQ_PRIV_MMODE, IRQ_CLEAR_PENDING);
}

static inline unsigned long irq_vect_get(unsigned irq_num)
{
    write_csr(IPIC_IDX, irq_num);
    return read_csr(IPIC_ICSR);
}

static inline void irq_vect_set(unsigned irq_num, unsigned long val)
{
    write_csr(IPIC_IDX, irq_num);
    write_csr(IPIC_ICSR, val);
}

static inline void irq_soi(void)
{
    write_csr(IPIC_SOI, 0);
}

static inline void irq_eoi(void)
{
    write_csr(IPIC_EOI, 0);
}

static inline void irq_enable(unsigned irq_num)
{
    write_csr(IPIC_IDX, irq_num);
    uint32_t state = (read_csr(IPIC_ICSR) & ~IRQ_PENDING) | IRQ_ENABLE;
    write_csr(IPIC_ICSR, state);
}

static inline void irq_disable(unsigned irq_num)
{
    write_csr(IPIC_IDX, irq_num);
    uint32_t state = read_csr(IPIC_ICSR) & ~(IRQ_ENABLE | IRQ_PENDING);
    write_csr(IPIC_ICSR, state);
}

static inline void irq_clear_pending(unsigned irq_num)
{
    write_csr(IPIC_IDX, irq_num);
    uint32_t state = read_csr(IPIC_ICSR) | IRQ_CLEAR_PENDING;
    write_csr(IPIC_ICSR, state);
}

static inline unsigned long irq_pending(void)
{
    return read_csr(IPIC_IPR);
}

static inline unsigned long irq_inservice(void)
{
    return read_csr(IPIC_ISVR);
}

static inline unsigned long irq_current_vector(void)
{
    return read_csr(IPIC_CISV);
}

static inline unsigned long irq_current_get(void)
{
    return read_csr(IPIC_CICSR);
}

static inline void irq_current_set(unsigned long val)
{
    write_csr(IPIC_CICSR, val);
}

static inline unsigned long irq_mmapping(void)
{
    return read_csr(IPIC_IMAP);
}

static inline unsigned long irq_smapping(void)
{
    return read_csr(IPIC_SIMAP);
}

static inline unsigned long irq_mier_get(void)
{
    return read_csr(IPIC_IER);
}

static inline void irq_mier_set(unsigned long val)
{
    write_csr(IPIC_IER, val);
}

static inline unsigned long irq_sier_get(void)
{
    return read_csr(IPIC_SIER);
}

static inline void irq_sier_set(unsigned long val)
{
    write_csr(IPIC_SIER, val);
}

#endif // !__ASSEMBLER__

#endif // SCR_IPIC_H
