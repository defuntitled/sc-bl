/*
 * Copyright (C) 2015, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* framework
///
/// @copyright Copyright (C) 2015-2016, Syntacore Ltd. All rights reserved.
/// @author mn-sc
///
/// @brief SCR PS/2 functions

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "rtc.h"

#if defined(PLF_PS2_PORT0_BASE) || defined(PLF_PS2_PORT1_BASE)

#define PS2_REG_DATA 0
#define PS2_REG_CTRL 4

#define PS2_DATA_MASK (0xff)
#define PS2_DATA_VALID (1 << 15)
#define PS2_DATA_COUNT_MASK (0xffff << 16)

#define PS2_CTRL_RE (1 << 0)
#define PS2_CTRL_RI (1 << 8)
#define PS2_CTRL_CE (1 << 10)

#define PS2_ACK 0xfa

static inline uint32_t plf_ps2_read(unsigned port, unsigned reg)
{
    return *(volatile uint32_t*)(port + reg);
}

static inline void plf_ps2_write(unsigned port, unsigned reg, uint32_t val)
{
    *(volatile uint32_t*)(port + reg) = val;
}

//---------------------------------

int ps2_write_byte(unsigned port, unsigned byte)
{
	plf_ps2_write(port, PS2_REG_DATA, byte);

    // CE bit: error occurs on sending commands
    if (plf_ps2_read(port, PS2_REG_CTRL) & PS2_CTRL_CE)
        return -1;

	return 0;
}

int ps2_read_byte(unsigned port)
{
    uint32_t data_reg = plf_ps2_read(port, PS2_REG_DATA);

    if (data_reg & PS2_DATA_VALID)
        return data_reg & PS2_DATA_MASK;

    return -1;
}

int ps2_read_byte_timeout(unsigned port)
{
	unsigned count = 0;
	do {
        count++;
		uint32_t data_reg = plf_ps2_read(port, PS2_REG_DATA);
		if (data_reg & PS2_DATA_VALID)
            return data_reg & PS2_DATA_MASK;
        rtc_delay_us(20000);
	} while (count < 10);

    return -1;
}

int ps2_read_byte_timeout2(unsigned port)
{
	unsigned count = 0;
	do {
        count++;
		uint32_t data_reg = plf_ps2_read(port, PS2_REG_DATA);
		if (data_reg & PS2_DATA_VALID)
            return data_reg & PS2_DATA_MASK;
        rtc_delay_us(20000);
	} while (count < 50);

    return -1;
}

int ps2_wait_ack(unsigned port)
{
    int res;
	do {
		res = ps2_read_byte_timeout2(port);
        if (res == PS2_ACK)
            return 0;
	} while (res >= 0);

	return -1;
}

int ps2_write_byte_ack(unsigned port, unsigned char byte)
{
	int status = ps2_write_byte(port, byte);

	if (status)
		return status;

	return ps2_wait_ack(port);
}

int ps2_cmd(unsigned port, unsigned cmd, unsigned char *iobuf, unsigned txcnt, unsigned rxcnt)
{
    int status = ps2_write_byte_ack(port, cmd);
    if (!status && iobuf) {
        if (txcnt) {
            for (unsigned i = 0; !status && i < txcnt; ++i)
                status = ps2_write_byte(port, iobuf[i]);
        }
        if (rxcnt) {
            for (unsigned i = 0; i < rxcnt; ++i) {
                int v = ps2_read_byte_timeout(port);
                if (v < 0) {
                    status = v;
                    break;
                }
                iobuf[i] = v;
            }
        }
    }

    return status;
}

//---------------------------------

enum Kbd_shift_state {
    KBD_STATE_SHIFT_OFF = 0,
    KBD_STATE_SHIFT_L = 1,
    KBD_STATE_SHIFT_R = 1,
};

static int shift_state = KBD_STATE_SHIFT_OFF;

void ps2_init(unsigned port)
{
    shift_state = KBD_STATE_SHIFT_OFF;

	// send out the reset command, wait for ACK (0xFA)
	int status = ps2_write_byte_ack(port, 0xff);
	if (status == 0) {
		// received the ACK for reset, now check the BAT result
		status = ps2_read_byte_timeout2(port);
		if (status == 0xAA) {
			// BAT succeed
			//get the 2nd byte
			status = ps2_read_byte_timeout(port);
			if (status == -1) {
				//for keyboard, only 2 bytes are sent(ACK, PASS/FAIL), so timeout
                printf("ps2kbd_init: keyboard detected\n");
                // setup repeat/delay
				if (!ps2_write_byte_ack(port, 0xf3))
                    ps2_write_byte(port, 15 | (1 << 5)); // 250/500
			} else if (status == 0) {
				//for mouse, it will sent out 0x00 after sending out ACK and PASS/FAIL.
				ps2_write_byte(port, 0xf4); // enable data from mouse
                printf("ps2kbd_init: mouse detected\n");
			}
		} else {
			// BAT failed
            /* printf("ps2kbd_init: failed\n"); */
		}
	}
}

#if 0
// led bit masks: 0x1 = SCROLL, 0x2 = NUM, 0x4 = CAPS
void ps2kbd_set_leds(unsigned port, int leds)
{
  	printf("ps2kbd_set_leds: %d %d %d\n",
           leds & 1 ? 1 : 0,
           leds & 2 ? 1 : 0,
           leds & 4 ? 1 : 0
        );
    ps2_write_byte(port, 0xed);
    ps2_write_byte(port, (unsigned char)leds);
}

// reset & disable
void ps2kbd_reset_dis(unsigned port)
{
  	printf("ps2kbd_reset_dis\n");
    ps2_write_byte(port, 0xf5);
}

// enable kbd
void ps2kbd_enable(unsigned port)
{
  	printf("ps2kbd_enable\n");
    ps2_write_byte(port, 0xf4);
}

// read bytes

void ps2kbd_read(unsigned port)
{
    int status;

  	printf("ps2kbd_read:");

    while ((status = ps2_read_byte(port)) >= 0) {
        printf(" %x", status);
    }

  	printf("\n");
}
#endif // 0/1

enum Kbd_scan_code {
    KBD_SHIFT_L = 0x12,
    KBD_SHIFT_R = 0x59,
};

int ps2kbd_getchar(unsigned port)
{
    int status;
    int ch = -1;

    while (1) {
        status = ps2_read_byte(port);
        if (status == -1)
            return -1;
        if (status == 0xf0) {
            // release key scancode
            // read next code and ignore
            status = ps2_read_byte_timeout(port);
            if (status == KBD_SHIFT_L)
                shift_state &= ~KBD_STATE_SHIFT_L;
            else if (status == KBD_SHIFT_R)
                shift_state &= ~KBD_STATE_SHIFT_R;
        } else
            break;
    }

    switch (status) {
    case KBD_SHIFT_R: // R_SHIFT
        shift_state |= KBD_STATE_SHIFT_R;
        break;
    case KBD_SHIFT_L: // L_SHIFT
        shift_state |= KBD_STATE_SHIFT_L;
        break;
    case 0x45:
        ch = '0';
        break;
    case 0x16:
        ch = shift_state ? '!' : '1';
        break;
    case 0x1e:
        ch = '2';
        break;
    case 0x26:
        ch = '3';
        break;
    case 0x25:
        ch = shift_state ? '$' : '4';
        break;
    case 0x2e:
        ch = '5';
        break;
    case 0x36:
        ch = '6';
        break;
    case 0x3d:
        ch = '7';
        break;
    case 0x3e:
        ch = '8';
        break;
    case 0x46:
        ch = '9';
        break;
    case 0x1c:
        ch = 'a';
        break;
    case 0x32:
        ch = 'b';
        break;
    case 0x21:
        ch = 'c';
        break;
    case 0x23:
        ch = 'd';
        break;
    case 0x24:
        ch = 'e';
        break;
    case 0x2b:
        ch = shift_state ? 'F' : 'f';
        break;
    case 0x1b:
        ch = 's';
        break;
    case 0x15:
        ch = 'q';
        break;
    case 0x1d:
        ch = 'w';
        break;
    case 0x3a:
        ch = 'm';
        break;
    case 0x34:
        ch = 'g';
        break;
    case 0x29:
        ch = ' ';
        break;
    case 0x5a:
        ch = '\r';
        break;
    case 0x66:
        ch = '\b';
        break;
    }

    return ch;
}


#endif // PLF_PS2_PORT0_BASE) || PLF_PS2_PORT1_BASE
