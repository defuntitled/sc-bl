/*
 * Copyright (C) 2015, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* framework
///
/// @copyright Copyright (C) 2015-2016, Syntacore Ltd. All Rights Reserved.
/// @author mn-sc
///
/// @brief SCR PS/2 functions

#ifndef SCR_KBD_H
#define SCR_KBD_H

#include "platform_config.h"

#ifdef PLF_PS2_PORT_KBD

void ps2_init(unsigned port);
int ps2kbd_getchar(unsigned port);

#else // PLF_PS2_PORT_KBD

#define ps2_init(port) do {} while (0)
#define ps2kbd_getchar(port) (-1)

#endif // PLF_PS2_PORT_KBD

#endif // SCR_KBD_H
