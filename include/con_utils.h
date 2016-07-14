/*
 * Copyright (C) 2019, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* framework
///
/// @copyright Copyright (C) 2019, Syntacore Ltd. All Rights Reserved.
/// @author mn-sc
///
/// @brief console utilites defs

#ifndef CON_UTILS_H
#define CON_UTILS_H

/* Print colorized Syntacore header */
void  con_print_scr_header(void);

/* Linux kernel log colorizer */
#if PLF_MMODE_ONLY == 0
void con_colorize_putchar(int ch);
#else
#define con_colorize_putchar(ch) do {} while (0)
#endif

#endif /* CON_UTILS_H */
