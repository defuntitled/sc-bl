/*
 * Copyright (C) 2015, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* framework
///
/// @copyright Copyright (C) 2015-2023, Syntacore Ltd. All Rights Reserved.
/// @author mn-sc, dz-sc
///
/// @brief IPI defs and inline funcs

#ifndef SCR_INFRA_IPI_H
#define SCR_INFRA_IPI_H

#include "platform_config.h"

#define SCR_CSR_IPI_MBASE   0xbd8

// M mode CSRs
#define SCR_CSR_IPI_MADDR   (SCR_CSR_IPI_MBASE + 0)
#define SCR_CSR_IPI_MSTATUS (SCR_CSR_IPI_MBASE + 1)
#define SCR_CSR_IPI_MRDATA  (SCR_CSR_IPI_MBASE + 2)
#define SCR_CSR_IPI_MWDATA  (SCR_CSR_IPI_MBASE + 3)
#define SCR_CSR_IPI_MRDATA_EMPTY_MASK (1ul << (__riscv_xlen - 1))

#ifndef __ASSEMBLER__

#include "csr.h"
#include "ipi_msg.h" // SCR_IPI_RECV_SRC_BITS

typedef unsigned long ipi_data_t;

static inline unsigned ipi_get_num_slots(void)
{
    uint64_t rddata;
    while (!((rddata = swap_csr(SCR_CSR_IPI_MRDATA, 1)) & SCR_CSR_IPI_MRDATA_EMPTY_MASK))
        ;
    return (rddata & ~SCR_CSR_IPI_MRDATA_EMPTY_MASK) + 1;
}

static inline bool ipi_msg_avail(void)
{
    return !(read_csr(SCR_CSR_IPI_MRDATA) & SCR_CSR_IPI_MRDATA_EMPTY_MASK);
}

static inline void ipi_send_data(unsigned receiver, ipi_data_t data)
{
    extern volatile unsigned slot_bits_shift;
    write_csr(SCR_CSR_IPI_MADDR, receiver);
    write_csr(SCR_CSR_IPI_MWDATA, data << slot_bits_shift);
}

static inline ipi_data_t ipi_read_data(void)
{
    return swap_csr(SCR_CSR_IPI_MRDATA, 1) >> SCR_IPI_RECV_SRC_BITS;
}

static inline bool ipi_receiver_busy(unsigned receiver)
{
    write_csr(SCR_CSR_IPI_MADDR, receiver);
    return read_csr(SCR_CSR_IPI_MSTATUS);
}

#else

.macro ipi_clear_mailbox
1:  csrrw t0, SCR_CSR_IPI_MRDATA, 1
    bgez  t0, 1b
.endm

.macro ipi_send_wait
    csrw  SCR_CSR_IPI_MADDR, a0  // dst slotn <- hls:a2.ipi_n

.Lipi_status_wait:
    csrr  a0, SCR_CSR_IPI_MSTATUS
    bnez  a0, .Lipi_status_wait  // wait until available
    csrw  SCR_CSR_IPI_MWDATA, a1 // write msg
.endm

#endif // __ASSEMBLER__

#endif // SCR_INFRA_IPI_H
