/*
 * Copyright (C) 2015, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* infra
///
/// @copyright Copyright (C) 2015-2017, Syntacore Ltd. All Rights Reserved.
/// @author mn-sc
///
/// @brief UART defs and inline funcs

#ifndef SC1_FPGA_UART_H
#define SC1_FPGA_UART_H

#include "platform_config.h"

#ifdef PLF_UART0_BASE

#include <hal/drivers/console.h>

#if defined(PLF_UART0_16550)
#include <hal/drivers/uart_16550.h>
#elif defined(PLF_UART0_16550_DW)
#include <hal/drivers/uart_dw.h>
#else
#error "Unknown uart implementation"
#endif

#include "clk.h"

static inline void uart_init(void) { con_init_clk(clk_uart_get()); }
static inline int uart_putc(int c) { return con_putc(c); }
static inline int uart_getc(void) { return con_getc(); }
static inline int uart_getc_nowait(void) { return con_getc_nowait(); }
static inline void uart_puthex(unsigned long val) { con_puthex(val); }
static inline void uart_puthex4(unsigned val) { con_puthex4(val); }
static inline void uart_puthex8(uint8_t val) { con_puthex8(val); }
static inline void uart_puthex16(uint16_t val) { con_puthex16(val); }
static inline void uart_puthex32(uint32_t val) { con_puthex32(val); }
static inline void uart_puthex64(uint64_t val) { con_puthex64(val); }
static inline void uart_putdec(unsigned long v) { con_putdec(v); }
static inline void uart_puts(const char *s) { con_puts(s); }
static inline unsigned long uart_read_hex(void) { return con_read_hex(); }
static inline unsigned long uart_read_dec(void) { return con_read_dec(0); }

#endif // PLF_UART0_BASE

#endif // SC1_FPGA_UART_H
