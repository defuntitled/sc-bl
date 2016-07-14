/*
 * Copyright (C) 2015, Syntacore Ltd.
 * All Rights Reserved.
 */

/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2001-2022 Georges Menie (www.menie.org)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/// Syntacore SCR* infra
///
/// @copyright Copyright (C) 2015-2016, Syntacore Ltd. All rights reserved.
/// @author mn-sc
///
/// @brief tiny xmodem loader

#include "xmodem.h"
#include "uart.h"
#include "rtc.h"
#include "crc.h"
#include <hal/drivers/leds.h>

#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18
#define CTRLZ 0x1A

#define DLY_1S 1000
#define MAXRETRANS 25

static inline int _inbyte(unsigned timeout_msec)
{
    int c;
    unsigned delay = timeout_msec << 7;
    do {
        c = uart_getc_nowait();
        if (c < 0) {
            rtc_delay_us(10);
            if (timeout_msec && !delay) {
                c = -2;
                break;
            }
            --delay;
        }
    } while (c < 0);

    return c;
}

static inline void _outbyte(int c)
{
    plf_con_put(c);
}

static void flushinput(void)
{
    while (_inbyte(DLY_1S) >= 0);
}

static void xmodem_send_can(void)
{
    _outbyte(CAN);
    _outbyte(CAN);
    _outbyte(CAN);
    flushinput();
}

int xmodem_receive(uint8_t *dest, unsigned destsz)
{
    int bufsz/*, crc = 0*/;
    unsigned char trychar = 'C';
    int c;

    uint8_t *buf = dest;
    uint16_t partial_crc;
    uint8_t seq[2];
    uint8_t seqnum = 1;
    uint16_t buf_crc = 0;
    int pktsize;
    int rxsize;

    int totalbytes = 0;

    int retry = MAXRETRANS;

    for (;;) {
        // wait for control byte
        while (1) {
            if (trychar) _outbyte(trychar);
            if ((c = _inbyte((DLY_1S)*3/2)) >= 0) {
                switch (c) {
                case SOH:
                    pktsize = bufsz = 128;
                    partial_crc = 0;
                    rxsize = -2;
                    goto start_recv;
                case STX:
                    pktsize = bufsz = 1024;
                    partial_crc = 0;
                    rxsize = -2;
                    goto start_recv;
                /* case 0x3: */
                /*     return -2; // ^C pressed */
                case EOT:
                    if (!totalbytes) // ^D pressed
                        return -2;
                    // transmition done
                    // send ACK and finish
                    _outbyte(ACK);
                    flushinput();
                    return totalbytes;
                case CAN:
                    // transmition cancelled by remote host
                    if ((c = _inbyte(DLY_1S)) == CAN) {
                        _outbyte(ACK);
                        flushinput();
                        return -1;
                    }
                    break;
                default:
                    break;
                }
            }
        }

    start_recv:
        /* if (trychar == 'C') crc = 1; */
        trychar = 0;
        // receive packet
        while (1) {
            c = _inbyte(DLY_1S);
            if (c < 0) {
                _outbyte(NAK);
                flushinput();
                break;
            }
            if (rxsize < 0) {
                seq[2 + rxsize] = c;
            } else if (rxsize < pktsize) {
                if (totalbytes + pktsize <= destsz)
                    buf[rxsize] = c;
                partial_crc = crc16_ccitt_update(partial_crc, c);
            } else if (rxsize == pktsize) {
                buf_crc = (c & 0xff) << 8;
            } else {
                // last crc byte
                buf_crc |= c & 0xff;

                if (seq[0] != (uint8_t)~seq[1]) {
                    // integrity error
                    _outbyte(NAK);
                    flushinput();
                    break;
                } else if (buf_crc != partial_crc) {
                    // CRC error
                    _outbyte(NAK);
                    flushinput();
                    break;
                } else if (seqnum == seq[0] + 1) {
                    // retransmission of the last packet
                    // ignore it
                    if (--retry <= 0) {
                        // too many retry error
                        xmodem_send_can();
                        return -3;
                    }
                    _outbyte(ACK);
                    break;
                } else if (seqnum == seq[0]) {
                    ++seqnum;
                    buf += pktsize;
                    totalbytes += pktsize;
                    retry = MAXRETRANS; // reset retries
                    _outbyte(ACK);
                    break;
                } else {
                    // out of sync
                    xmodem_send_can();
                    return -2;
                }
            }
            ++rxsize;
        }
    }
}
