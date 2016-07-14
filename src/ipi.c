/*
 * Copyright (C) 2015, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* framework
///
/// @copyright Copyright (C) 2015-2023, Syntacore Ltd. All rights reserved.
/// @author mn-sc, dz-sc
///
/// @brief IPI funcs

#include "smp.h"

#if PLF_SMP_SUPPORT

#include "hls.h"
#include "vm.h" // slot_bits_shift, num_harts
#include "rtc.h"

#define NOINLINE_ATTR  __attribute__((noinline))
ipi_msg ipi_msg_pool[SCIPI_MSG_POOL_SIZE];

static void NOINLINE_ATTR do_local_ipi_cmd(unsigned long msg);
static void ipi_check_local_cmd(void);

static void ipi_send_cmd(unsigned long hart_mask, unsigned long packed_msg, sc_atomic *msg_cnt)
{
    // process local msg
    unsigned long cur_hart_mask = 1UL << logical_hart();
    if (hart_mask & cur_hart_mask) {
        hart_mask &= ~cur_hart_mask;
        if (msg_cnt)
            atomic_add(1, msg_cnt);
        do_local_ipi_cmd(packed_msg);
    }

    for (unsigned i = 0; i < num_harts; ++i, hart_mask >>= 1) {
        if (!(hart_mask & 1))
            continue;
        unsigned slotn = hart_hls(i)->ipi_n;

        // process incoming messages to prevent deadlocks
        while (ipi_receiver_busy(slotn))
            ipi_check_local_cmd();

        if (!ipi_receiver_busy(slotn)) {
            if (msg_cnt)
                atomic_add(1, msg_cnt);
            ipi_send_data(slotn, packed_msg);
        }
    }
}

static void ipi_check_local_cmd(void)
{
    if (ipi_msg_avail())
        do_local_ipi_cmd(ipi_read_data());
}

/* returns true if receiver became available, false otherwise */
bool ipi_wait_receiver_avail(unsigned receiver, unsigned timeout_us)
{
    if (!timeout_us)
        return !ipi_receiver_busy(receiver);
    for (unsigned i = 0; i < timeout_us; ++i) {
        if (!ipi_receiver_busy(receiver))
            return true;
        rtc_delay_us(1);
    }
    return false;
}

void NOINLINE_ATTR ipi_free_msg(ipi_msg *msg)
{
    // decrement usage counter
    atomic_add(-1, &msg->shared_cnt);
}

void NOINLINE_ATTR ipi_share_msg(ipi_msg *msg)
{
    // decrement usage counter
    atomic_add(1, &msg->shared_cnt);
}

static void NOINLINE_ATTR do_local_ipi_cmd(unsigned long msg)
{
#if !PLF_SMP_ICCM_SUPPORT
    msg <<= SCR_IPI_RECV_SRC_BITS;
#endif
    /* unsigned long from_slot = msg & SCR_IPI_RECV_SRC_MASK; */
    unsigned long cmd = EXTRACT_SCIPI_MSG_CMD(msg);

    if (cmd == SCR_IPI_CMD_SIGNAL) {
        set_csr(mip, (1 << INT_SM_SOFTWARE)); // set MIP_SSIP
    } else if (cmd == SCR_IPI_CMD_FENCE_I) {
        ifence();
    } else if (cmd >= SCR_IPI_CMD_MSG_BASE && cmd < SCR_IPI_CMD_MSG_BASE + SCIPI_MSG_POOL_SIZE) {
        // ipi messages
        ipi_msg *im = &ipi_msg_pool[cmd - SCR_IPI_CMD_MSG_BASE];
        ipi_msg_handler handler = (ipi_msg_handler)(im->handler_msg);
        handler(im);
        ipi_free_msg(im);
    }
}

ipi_msg * NOINLINE_ATTR ipi_alloc_msg(void)
{
    unsigned long i = logical_hart() % SCIPI_MSG_POOL_SIZE;
    for (;;) {
        sc_atomic *cnt = &ipi_msg_pool[i].shared_cnt;
        if (atomic_read(cnt) == 0) {
            if (atomic_add(1, cnt) == 0) {
                break;
            }
            atomic_add(-1, cnt); // allocation conflict
        }
        i = (i + 1) % SCIPI_MSG_POOL_SIZE;
        if (i == 0) {
            // process incoming IPI
            ipi_check_local_cmd();
        }
    }

    return &ipi_msg_pool[i];
}

void ipi_send_signal(unsigned long hart_mask, unsigned long sig)
{
    ipi_send_cmd(hart_mask, PACK_SCIPI_MSG(sig, 0), 0);
}

void ipi_send_msg(unsigned long hart_mask, ipi_msg *msg)
{
    unsigned long n = msg - ipi_msg_pool; // msg offset

    ipi_send_cmd(hart_mask, PACK_SCIPI_MSG(SCR_IPI_CMD_MSG_BASE + n, 0), &msg->shared_cnt);
}

#endif // PLF_SMP_SUPPORT
