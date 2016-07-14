/*
 * Copyright (C) 2023, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* framework
///
/// @copyright Copyright (C) 2023, Syntacore Ltd. All Rights Reserved.
/// @author gdi-sc, dz-sc
///
/// @brief SMP common includes

#ifndef SCR_COMMON_SMP_H
#define SCR_COMMON_SMP_H

#include "platform_config.h"

#if PLF_SMP_SUPPORT

#if PLF_SMP_ICCM_SUPPORT
#include "iccm.h"
#else // PLF_SMP_ICCM_SUPPORT
#include "ipi.h"
#endif // PLF_SMP_ICCM_SUPPORT
#include "ipi_msg.h"

#ifndef __ASSEMBLER__

#include <stdbool.h>

/* common API for old and new ICCM */
unsigned ipi_get_num_slots(void);
bool ipi_msg_avail(void);
void ipi_send_data(unsigned receiver, ipi_data_t data);
bool ipi_receiver_busy(unsigned receiver);
ipi_data_t ipi_read_data(void);
unsigned ipi_get_sender_id(void);
bool ipi_wait_receiver_avail(unsigned receiver, unsigned timeout_us);

#endif // __ASSEMBLER__
#endif // PLF_SMP_SUPPORT
#endif // SCR_COMMON_SMP_H
