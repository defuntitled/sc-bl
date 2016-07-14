/*
 * Copyright (C) 2018, Syntacore Ltd.
 * All Rights Reserved.
 */

#include "platform_config.h"

#ifdef PLF_XQSPI_BASE

#include <stdint.h>
#include <stdio.h>
#include "xqspi.h"


uint32_t xqspi_read_reg32(uintptr_t xspi_base, unsigned reg_offs)
{
    return *(volatile uint32_t *) (xspi_base + reg_offs);
}

uint8_t xqspi_read_reg8(uintptr_t xspi_base, unsigned reg_offs)
{
    return *(volatile uint8_t *) (xspi_base + reg_offs);
}

void xqspi_write_reg32(uintptr_t xspi_base, unsigned reg_offs, uint32_t val)
{
    volatile uint32_t *addr = (uint32_t*)(xspi_base + reg_offs);
    *addr = val;
}

void xqspi_write_reg8(uintptr_t xspi_base, unsigned reg_offs, uint8_t val)
{
    *(volatile uint8_t*)(xspi_base + reg_offs) = val;
}

void xqspi_intr_global_disable(uintptr_t xspi_base)
{
    xqspi_write_reg32(xspi_base, XSP_DGIER_OFFSET, 0);
}

uint32_t xqspi_intr_get_status(uintptr_t xspi_base)
{
    return xqspi_read_reg32(xspi_base, XSP_IISR_OFFSET);
}

void xqspi_intr_clear(uintptr_t xspi_base, uint32_t clear_mask)
{
    uint32_t val = xqspi_intr_get_status(xspi_base) | clear_mask;
    xqspi_write_reg32(xspi_base, XSP_IISR_OFFSET, val);
}

uint32_t xqspi_get_status_reg(uintptr_t xspi_base)
{
    return xqspi_read_reg32(xspi_base, XSP_SR_OFFSET);
}

void xqspi_tx32(uintptr_t xspi_base, uint32_t val)
{
    xqspi_write_reg32(xspi_base, XSP_DTR_OFFSET, val);
}

void xqspi_tx8(uintptr_t xspi_base, uint8_t val)
{
    xqspi_write_reg8(xspi_base, XSP_DTR_OFFSET, val);
}

uint32_t xqspi_get_ctrl_reg(uintptr_t xspi_base)
{
    return xqspi_read_reg32(xspi_base, XSP_CR_OFFSET);
}

void xqspi_set_ctrl_reg(uintptr_t xspi_base, uint32_t val)
{
    return xqspi_write_reg32(xspi_base, XSP_CR_OFFSET, val);
}

void xqspi_start(uintptr_t xspi_base)
{
    uint32_t ctrl = xqspi_get_ctrl_reg(xspi_base) & (~XSP_CR_TRANS_INHIBIT_MASK);
    xqspi_write_reg32(xspi_base, XSP_SSR_OFFSET, XSP_SSR_ENA);
    xqspi_set_ctrl_reg(xspi_base, ctrl | XSP_CR_ENABLE_MASK);
}

void xqspi_stop(uintptr_t xspi_base)
{
    uint32_t ctrl = xqspi_get_ctrl_reg(xspi_base) | XSP_CR_TRANS_INHIBIT_MASK;
    xqspi_write_reg32(xspi_base, XSP_SSR_OFFSET, XSP_SSR_DIS);
    xqspi_set_ctrl_reg(xspi_base, ctrl & ~(XSP_CR_ENABLE_MASK));
}

int xqspi_wait_tx_done(uintptr_t xspi_base)
{
    uint32_t status;

    do {
        status = xqspi_get_status_reg(xspi_base/*PLF_XQSPI_BASE*/);
        if (status & XSP_SR_ERR_MASK) {
            return -1;
        }
    } while ((status & XSP_INTR_TX_EMPTY_MASK) == 0);
    return 0;
}

void xqspi_init(uintptr_t xspi_base)
{
    uint32_t ctrl;
    ctrl = xqspi_get_ctrl_reg(xspi_base);
    ctrl |= XSP_CR_TXFIFO_RESET_MASK |
            XSP_CR_RXFIFO_RESET_MASK |
            XSP_CR_MANUAL_SS_MASK |
            XSP_CR_ENABLE_MASK |
            XSP_CR_TRANS_INHIBIT_MASK |
            XSP_CR_MASTER_MODE_MASK;
    xqspi_set_ctrl_reg(xspi_base, ctrl);

    //xqspi_get_status_reg(xspi_base);
    //xqspi_write_reg32(xspi_base, XSP_SSR_OFFSET, XSP_SSR_ENA);
}

void xqspi_reset_tx_rx_fifo(uintptr_t xspi_base)
{
    uint32_t ctrl;
    ctrl = xqspi_get_ctrl_reg(xspi_base);
    ctrl |= XSP_CR_TXFIFO_RESET_MASK | XSP_CR_RXFIFO_RESET_MASK;
    xqspi_set_ctrl_reg(xspi_base, ctrl);
}

void xqspi_reset_rx_fifo(uintptr_t xspi_base)
{
    uint32_t ctrl;
    ctrl = xqspi_get_ctrl_reg(xspi_base);
    ctrl |= XSP_CR_RXFIFO_RESET_MASK;
    xqspi_set_ctrl_reg(xspi_base, ctrl);
}

void xqspi_reset_tx_fifo(uintptr_t xspi_base)
{
    uint32_t ctrl;
    ctrl = xqspi_get_ctrl_reg(xspi_base);
    ctrl |= XSP_CR_TXFIFO_RESET_MASK;
    xqspi_set_ctrl_reg(xspi_base, ctrl);
}

void xqspi_abort(uintptr_t xspi_base)
{

}

void xqspi_reset(uintptr_t xspi_base)
{
    xqspi_abort(xspi_base);
    xqspi_write_reg32(xspi_base, XSP_CR_OFFSET,
        XSP_CR_MASTER_MODE_MASK | XSP_CR_MANUAL_SS_MASK |
        XSP_CR_TXFIFO_RESET_MASK | XSP_CR_RXFIFO_RESET_MASK |
        XSP_CR_TRANS_INHIBIT_MASK);

    /* Reset SPI controller */
    xqspi_write_reg32(xspi_base, XSP_SRR_OFFSET, XSP_SRR_RESET_MASK);

    xqspi_write_reg32(xspi_base, XSP_SSR_OFFSET, XSP_SSR_DIS);

    xqspi_write_reg32(xspi_base, XSP_CR_OFFSET,
        XSP_CR_MASTER_MODE_MASK |
        XSP_CR_MANUAL_SS_MASK |
        XSP_CR_TRANS_INHIBIT_MASK |
        XSP_CR_ENABLE_MASK);
}

void xqspi_flash_cmd(uintptr_t xspi_base, uint8_t cmd)
{
    uint32_t status;

    xqspi_tx32(xspi_base, cmd);
    xqspi_start(xspi_base);
    xqspi_wait_tx_done(xspi_base);
    status = xqspi_get_status_reg(xspi_base);
    while ((status & XSP_SR_RX_EMPTY_MASK) == 0) {
        xqspi_read_reg32(xspi_base, XSP_DRR_OFFSET);
        status = xqspi_get_status_reg(xspi_base);
    }
}

void xqspi_flash_cs_off_on(uintptr_t xspi_base)
{
    xqspi_write_reg32(xspi_base, XSP_SSR_OFFSET, XSP_SSR_DIS);
    xqspi_get_status_reg(xspi_base);
    xqspi_write_reg32(xspi_base, XSP_SSR_OFFSET, XSP_SSR_ENA);
}

static uint8_t xqspi_flash_read_status(uintptr_t xspi_base, int cont)
{
    uint32_t status;
    uint8_t flash_status = 0;

    /* read status command (0x05) */
    xqspi_tx32(xspi_base, 0x05);
    xqspi_tx32(xspi_base, 0x00);
    xqspi_start(xspi_base);
    xqspi_wait_tx_done(xspi_base);

    status = xqspi_get_status_reg(xspi_base);
    while ((status & XSP_SR_RX_EMPTY_MASK) == 0) {
        flash_status = xqspi_read_reg8(xspi_base, XSP_DRR_OFFSET);
        status = xqspi_get_status_reg(xspi_base);
    }
    if (!cont) {
        xqspi_stop(xspi_base);
    }
    return flash_status;
}

/*static*/ uint8_t xqspi_flash_write_status(uintptr_t xspi_base, uint8_t val)
{
    uint32_t status;
    uint8_t flash_status = 0;

    /* write status command (0x01) */
    xqspi_tx8(xspi_base, 0x01);
    xqspi_tx8(xspi_base, val);
    xqspi_start(xspi_base);
    xqspi_wait_tx_done(xspi_base);

    status = xqspi_get_status_reg(xspi_base);
    if (status & XSP_SR_RX_EMPTY_MASK) {
        return 0;
    }

    flash_status = xqspi_read_reg8(xspi_base, XSP_DRR_OFFSET);
    return flash_status;
}

/*static*/ uint8_t xqspi_flash_read_flags_status(uintptr_t xspi_base, int cont)
{
    uint32_t status;
    uint32_t flash_status = 0;

    /* read flags status command (0x70) */
    xqspi_tx32(xspi_base, 0x70);
    xqspi_tx32(xspi_base, 0x00);
    xqspi_start(xspi_base);
    xqspi_wait_tx_done(xspi_base);

    status = xqspi_get_status_reg(xspi_base);
    while ((status & XSP_SR_RX_EMPTY_MASK) == 0) {
        flash_status = xqspi_read_reg8(xspi_base, XSP_DRR_OFFSET);
        status = xqspi_get_status_reg(xspi_base);
    }
    if (!cont) {
        xqspi_stop(xspi_base);
    }
    return flash_status;
}

/*static*/ uint8_t xqspi_flash_clear_flags_status(uintptr_t xspi_base)
{
    uint32_t status;
    uint32_t flash_status = 0;

    /* clear flags status command (0x50) */
    xqspi_tx32(xspi_base, 0x50);
    xqspi_start(xspi_base);
    xqspi_wait_tx_done(xspi_base);

    status = xqspi_get_status_reg(xspi_base);
    if (status & XSP_SR_RX_EMPTY_MASK) {
        return 0;
    }

    flash_status = xqspi_read_reg32(xspi_base, XSP_DRR_OFFSET);
    return flash_status;
}

int xqspi_flash_get_jedec_id(uintptr_t xspi_base, uint8_t *id, size_t len)
{
    uint32_t status, val, op_cnt;

    /* read id command 0x9F/0x9E */
    xqspi_tx32(xspi_base, 0x9F);
    op_cnt = 0;
    while (op_cnt++ < 7) {
        xqspi_tx32(xspi_base, 0x00);
    }

    xqspi_start(xspi_base/*PLF_XQSPI_BASE*/);
    xqspi_wait_tx_done(xspi_base/*PLF_XQSPI_BASE*/);

    status = xqspi_get_status_reg(xspi_base);
    if (status & XSP_SR_RX_EMPTY_MASK) {
        return 0;
    }
    op_cnt = 0;
    while ((status & XSP_SR_RX_EMPTY_MASK) == 0) {
        val = xqspi_read_reg32(xspi_base, XSP_DRR_OFFSET);
        if (op_cnt++ >= 1) {
            if (op_cnt < len) {
                *id++ = val;
            }
        }
        status = xqspi_get_status_reg(xspi_base);
    }
    xqspi_stop(xspi_base/*PLF_XQSPI_BASE*/);
    return (op_cnt - 1);
}

void xqspi_flash_print_jedec_id(uintptr_t xspi_base)
{
    uint8_t id[8];
    int id_len, i;

    id_len = xqspi_flash_get_jedec_id(xspi_base, id, sizeof(id));
    if (id_len) {
        printf("JEDEC Id: ");
        for (i = 0; i < id_len; i++) {
            printf("%02X ", id[i]);
        }
        printf("\n");
    }
}

/*static*/ void xqspi_flash_write_enable(uintptr_t xspi_base)
{
    xqspi_flash_cmd(xspi_base, 0x06);
    xqspi_stop(xspi_base);
}

/*static*/ void xqspi_flash_write_disable(uintptr_t xspi_base)
{
    xqspi_flash_cmd(xspi_base, 0x04);
    xqspi_stop(xspi_base);
}

uint32_t xqspi_flash_fast_read(uintptr_t xspi_base, uintptr_t start_addr,
    uint8_t *buf, uint32_t size, int interactive)
{
    uint32_t status;
    size_t op_cnt, dummy_cycles;
    uint8_t *ptr;
    uint8_t id[8];
    int id_len;

    xqspi_reset(xspi_base/*PLF_XQSPI_BASE*/);
    xqspi_init(xspi_base/*PLF_XQSPI_BASE*/);

    // FIXME: function get_jedec_id() used as additional init
    id_len = xqspi_flash_get_jedec_id(xspi_base, id, sizeof(id));
    (void)id_len;
    /* TODO: check id */

    xqspi_reset_tx_rx_fifo(xspi_base);

    /* fast read 32bit addr command (0x0C) */
    xqspi_tx32(xspi_base, 0x0C);
    // FIXME: 64bit address???
    xqspi_tx32(xspi_base, start_addr >> 24 & 0xFF);
    xqspi_tx32(xspi_base, start_addr >> 16 & 0xFF);
    xqspi_tx32(xspi_base, start_addr >> 8 & 0xFF);
    xqspi_tx32(xspi_base, start_addr & 0xFF);

    xqspi_start(xspi_base/*PLF_XQSPI_BASE*/);

    xqspi_wait_tx_done(xspi_base/*PLF_XQSPI_BASE*/);

    status = xqspi_get_status_reg(xspi_base);
    if (status & XSP_SR_RX_EMPTY_MASK) {
        return 0;
    }

    op_cnt = 0;
    ptr = buf;
    dummy_cycles = 5;

    for (int i = 0; i <= dummy_cycles; ++i) {
        xqspi_read_reg8(xspi_base, XSP_DRR_OFFSET);
        xqspi_write_reg8(xspi_base, XSP_DTR_OFFSET, 0x00);
    }

    for (int i = (8 - (((uintptr_t)ptr) & 0x7)) & 0x7; i > 0 && op_cnt < size; --i, ++op_cnt) {
        *ptr++ = xqspi_read_reg8(xspi_base, XSP_DRR_OFFSET);
        xqspi_write_reg8(xspi_base, XSP_DTR_OFFSET, 0x00);
    }

    int wcnt = 0;
    unsigned long wdata = 0;

    while (op_cnt < size) {
        uint8_t val8 = xqspi_read_reg8(xspi_base, XSP_DRR_OFFSET);
        xqspi_write_reg8(xspi_base, XSP_DTR_OFFSET, 0x00);
        ++op_cnt;
        wdata = wdata | ((unsigned long)val8 << wcnt * 8);
        if (wcnt == (sizeof (unsigned long) - 1)) {
            *(volatile unsigned long*)ptr = wdata;
            ptr += sizeof (wdata);
            wdata = 0;
            wcnt = 0;
        } else {
            ++wcnt;
        }

        if (interactive && !(op_cnt & 0x3FFFF)) {
            printf("\r%luK ", (unsigned long)op_cnt / 1024);
        }
    }

    xqspi_stop(xspi_base/*PLF_XQSPI_BASE*/);
    return size;
}

uint32_t xqspi_flash_quad_read(uintptr_t xspi_base, uintptr_t start_addr,
    uint8_t *buf, uint32_t size, int interactive)
{
    uint32_t status, ctrl;
    size_t op_cnt, dummy_cycles;
    uint8_t *ptr;

    ctrl = xqspi_get_ctrl_reg(xspi_base);
    ctrl |= XSP_CR_TXFIFO_RESET_MASK | XSP_CR_RXFIFO_RESET_MASK |
        XSP_CR_ENABLE_MASK | XSP_CR_MASTER_MODE_MASK;
    xqspi_set_ctrl_reg(xspi_base, ctrl);
    status = xqspi_get_status_reg(xspi_base);

    /* enter quad command (0x35) */
    /* xqspi_flash_cmd(xspi_base, 0x35); */

    /* quad read 4-byte command (0x6C) */
    xqspi_tx32(xspi_base, 0x6C);
    // FIXME: 64bit address???
    xqspi_tx32(xspi_base, start_addr >> 24 & 0xFF);
    xqspi_tx32(xspi_base, start_addr >> 16 & 0xFF);
    xqspi_tx32(xspi_base, start_addr >> 8 & 0xFF);
    xqspi_tx32(xspi_base, start_addr & 0xFF);
    xqspi_tx32(xspi_base, 0);
    xqspi_tx32(xspi_base, 0);
    xqspi_tx32(xspi_base, 0);

    xqspi_start(xspi_base/*PLF_XQSPI_BASE*/);

    xqspi_wait_tx_done(xspi_base/*PLF_XQSPI_BASE*/);

    status = xqspi_get_status_reg(xspi_base);
    if (status & XSP_SR_RX_EMPTY_MASK) {
        return 0;
    }

    op_cnt = 0;
    ptr = buf;
    dummy_cycles = 8;
    while (op_cnt < size + dummy_cycles) {
        uint8_t val = xqspi_read_reg8(xspi_base, XSP_DRR_OFFSET);
        xqspi_write_reg8(xspi_base, XSP_DTR_OFFSET, 0);
        /* skip dummy cycles */
        if (op_cnt++ <= dummy_cycles) {
            continue;
        }

        *ptr++ = val;
        if (interactive && !(op_cnt & 0x0FFFF)) {
            printf("\r%luK ", (unsigned long)op_cnt / 1024);
        }
    }

    /* exit quad command (0xF5) */
    /* xqspi_flash_cmd(xspi_base, 0xF5); */

    xqspi_stop(xspi_base/*PLF_XQSPI_BASE*/);
    return size;
}

void xqspi_flash_4K_subsector_erase(uintptr_t xspi_base, uintptr_t start_addr)
{
    uint8_t flash_status;
    int i;

    /* write enable */
    xqspi_flash_write_enable(xspi_base);

    /* subsector erase 0x20 */
    /* start_addr &= ~(4 * 1024); */
    xqspi_reset_tx_rx_fifo(xspi_base);
    xqspi_init(xspi_base);
    xqspi_tx32(xspi_base, 0x20);
    // FIXME: 64bit address???
    xqspi_tx32(xspi_base, start_addr >> 24 & 0xFF);
    xqspi_tx32(xspi_base, start_addr >> 16 & 0xFF);
    xqspi_tx32(xspi_base, start_addr >> 8 & 0xFF);
    xqspi_tx32(xspi_base, start_addr & 0xFF);
    xqspi_start(xspi_base);
    xqspi_wait_tx_done(xspi_base);

    xqspi_write_reg32(xspi_base, XSP_SSR_OFFSET, XSP_SSR_DIS);
    xqspi_reset_tx_rx_fifo(xspi_base);
    /* wait for start erase */
    i = 0;
    do {
        flash_status = xqspi_flash_read_status(xspi_base, 0);
        if (i++ > 100) {
            break;
        }
    } while ((flash_status & 0x1) == 0);

    /* wait for erase done */
    do {
        flash_status = xqspi_flash_read_status(xspi_base, 0);
    } while (flash_status & 0x1);

    xqspi_stop(xspi_base);
}

/* TODO: start_addr must be page aligned (at 256 bytes) */
void xqspi_flash_write(uintptr_t xspi_base, uintptr_t start_addr,
    uint8_t *buf, uint32_t size, int interactive)
{
    uint8_t *ptr, *end, flash_status;
    uintptr_t cur_addr;
    size_t num_sectors, s_cnt;
    int i;
    int id_len;
    uint8_t id[8];

    xqspi_reset(xspi_base/*PLF_XQSPI_BASE*/);
    xqspi_init(xspi_base/*PLF_XQSPI_BASE*/);

    id_len = xqspi_flash_get_jedec_id(xspi_base, id, sizeof(id));
    (void)id_len;
    /* TODO: check id */

    num_sectors = size / XSP_FLASH_4K_PAGE_SIZE;
    if (size % XSP_FLASH_4K_PAGE_SIZE) {
        num_sectors++;
    }

    ptr = buf;
    cur_addr = start_addr;

    for (s_cnt = 0; s_cnt < num_sectors; s_cnt++) {
        if (interactive) {
            if (ptr != buf && !((ptr - buf) & 0xFFFF)) {
                printf("\r%luK ", (unsigned long)(ptr - buf) / 1024);
            }
        }
        xqspi_flash_4K_subsector_erase(xspi_base, cur_addr);
        end = ptr + (size > XSP_FLASH_4K_PAGE_SIZE ?
            XSP_FLASH_4K_PAGE_SIZE : size);
        while (ptr < end) {
            int wr_size;
            /* write enable */
            xqspi_flash_write_enable(xspi_base);
            xqspi_reset_tx_rx_fifo(xspi_base);
            // FIXME: 64bit address???
            /* fast write 32bit addr command (0x02) */
            xqspi_tx32(xspi_base, 0x02);
            xqspi_tx32(xspi_base, cur_addr >> 24 & 0xFF);
            xqspi_tx32(xspi_base, cur_addr >> 16 & 0xFF);
            xqspi_tx32(xspi_base, cur_addr >> 8 & 0xFF);
            xqspi_tx32(xspi_base, cur_addr & 0xFF);
            xqspi_start(xspi_base);
            xqspi_wait_tx_done(xspi_base);
            cur_addr += XSP_FLASH_PAGE_SIZE;
            wr_size = size > XSP_FLASH_PAGE_SIZE ? XSP_FLASH_PAGE_SIZE : size;

            i = 0;
            while (i < wr_size) {
                int j;
                if (ptr >= end) {
                    break;
                }
                for (j = 0; j < XSP_TX_FIFO_DEPTH; j++, i++) {
                    if (i >= wr_size) {
                        break;
                    }
                    xqspi_tx32(xspi_base, *ptr++);
                }
                xqspi_wait_tx_done(xspi_base);
                xqspi_reset_rx_fifo(xspi_base);
            }

            if (interactive) {
                if (ptr != buf && !((ptr - buf) & 0xFFFF)) {
                    printf("\r%luK ", (unsigned long)(ptr - buf) / 1024);
                }
            }

            if (size > wr_size) {
                size -= wr_size;
            } else {
                size = 0;
            }
            xqspi_write_reg32(xspi_base, XSP_SSR_OFFSET, XSP_SSR_DIS);
            i = 0;
            do {
                flash_status = xqspi_flash_read_status(xspi_base, 0);
                if (i++ > 100) {
                    break;
                }
            } while ((flash_status & 0x1) == 0);
            do {
                flash_status = xqspi_flash_read_status(xspi_base, 0);
            } while (flash_status & 0x1);
        }
    }
    xqspi_stop(xspi_base/*PLF_XQSPI_BASE*/);
}

void xqspi_clean_rx_fifo(uintptr_t xspi_base)
{
    uint32_t status;

    status = xqspi_get_status_reg(xspi_base);
    while ((status & XSP_SR_RX_EMPTY_MASK) == 0) {
        xqspi_read_reg32(xspi_base, XSP_DRR_OFFSET);
        status = xqspi_get_status_reg(xspi_base);
    }
}

uint16_t xqspi_flash_read_nv_cfg_reg(uintptr_t xspi_base)
{
    uint32_t status;
    uint16_t nv_cfg = 0;
    uint8_t val8;

    xqspi_init(xspi_base);
    xqspi_clean_rx_fifo(xspi_base);
    /* read non volatile cfg reg: 0xB5 */
    xqspi_tx32(xspi_base, 0xB5);
    xqspi_tx32(xspi_base, 0);
    xqspi_tx32(xspi_base, 0);
    xqspi_start(xspi_base);

    xqspi_wait_tx_done(xspi_base);

    status = xqspi_get_status_reg(xspi_base);
    if ((status & XSP_SR_RX_EMPTY_MASK) == 0) {
        val8 = (uint8_t)xqspi_read_reg32(xspi_base, XSP_DRR_OFFSET);
        nv_cfg |= val8;
        nv_cfg &= 0x00FF;
    }
    status = xqspi_get_status_reg(xspi_base);
    if ((status & XSP_SR_RX_EMPTY_MASK) == 0) {
        val8 = (uint8_t)xqspi_read_reg32(xspi_base, XSP_DRR_OFFSET);
        nv_cfg |= val8 << 8;
    }

    xqspi_stop(xspi_base);
    return nv_cfg;
}

uint8_t xqspi_flash_read_v_cfg_reg(uintptr_t xspi_base)
{
    uint32_t status;
    uint8_t v_cfg = 0;

    xqspi_init(xspi_base);
    xqspi_clean_rx_fifo(xspi_base);
    /* read non volatile cfg reg: 0x85 */
    xqspi_tx32(xspi_base, 0x85);
    xqspi_tx32(xspi_base, 0);
    xqspi_start(xspi_base);
    xqspi_wait_tx_done(xspi_base);

    status = xqspi_get_status_reg(xspi_base);
    if ((status & XSP_SR_RX_EMPTY_MASK) == 0) {
        v_cfg = (uint8_t)xqspi_read_reg32(xspi_base, XSP_DRR_OFFSET);
    }
    xqspi_stop(xspi_base);
    return v_cfg;
}

void xqspi_flash_write_nv_cfg_reg(uintptr_t xspi_base, uint16_t nv_cfg)
{
    xqspi_init(xspi_base);
    xqspi_clean_rx_fifo(xspi_base);
    xqspi_flash_write_enable(xspi_base);

    /* write non volatile cfg reg: 0xB1 */
    xqspi_tx32(xspi_base, 0xB1);
    xqspi_tx32(xspi_base, nv_cfg >> 8 & 0xFF);
    xqspi_tx32(xspi_base, nv_cfg & 0xFF);
    xqspi_start(xspi_base);
    xqspi_wait_tx_done(xspi_base);
    xqspi_stop(xspi_base);
}

static void pr_data(uintptr_t addr, uint8_t *buf, size_t buf_len)
{
    uint32_t i;

    printf("READ DATA (%p): \n", (void*)addr);
    for (i = 0; i < buf_len; i++) {
        if (i && !(i & 0xF)) {
            printf("\n");
        }
        printf("%02X, ", buf[i]);
    }
    printf("\n");
}

void xqspi_hung_test(uintptr_t xspi_base)
{
    int i;
    uint8_t flash_status;

    i = 0;
    do {
        flash_status = xqspi_flash_read_status(xspi_base, 0);
        if (i++ > 300) {
            break;
        }
    } while ((flash_status & 0x1) == 0);
}

void xqspi_flash_test(void)
{
    uint8_t buf[128];
    //uint16_t nv_cfg;

    xqspi_reset(PLF_XQSPI_BASE);
    xqspi_init(PLF_XQSPI_BASE);

    xqspi_flash_print_jedec_id(PLF_XQSPI_BASE);
    xqspi_reset_tx_rx_fifo(PLF_XQSPI_BASE);
    xqspi_flash_print_jedec_id(PLF_XQSPI_BASE);
#if 0
    printf("flash status=0x%02X\n",
        xqspi_flash_read_status(PLF_XQSPI_BASE, 0));
    xqspi_stop(PLF_XQSPI_BASE);
    printf("flash flags status=0x%02X\n",
        xqspi_flash_read_flags_status(PLF_XQSPI_BASE, 0));
    xqspi_stop(PLF_XQSPI_BASE);

    nv_cfg = xqspi_flash_read_nv_cfg_reg(PLF_XQSPI_BASE);
    printf("nv_cfg_reg: 0x%04X\n", nv_cfg);

    xqspi_flash_write_nv_cfg_reg(PLF_XQSPI_BASE, nv_cfg & ~(3 << 2));
    nv_cfg = xqspi_flash_read_nv_cfg_reg(PLF_XQSPI_BASE);
    printf("nv_cfg_reg: 0x%04X\n", nv_cfg);
    printf("v_cfg_reg: 0x%04X\n", xqspi_flash_read_v_cfg_reg(PLF_XQSPI_BASE));
#endif
#if 0
    nv_cfg = xqspi_flash_read_nv_cfg_reg(PLF_XQSPI_BASE);
    printf("nv_cfg_reg: 0x%04X\n", nv_cfg);
    xqspi_flash_write_nv_cfg_reg(PLF_XQSPI_BASE, nv_cfg | (3 << 2));
    nv_cfg = xqspi_flash_read_nv_cfg_reg(PLF_XQSPI_BASE);
    printf("nv_cfg_reg: 0x%04X\n", nv_cfg);
#endif
    {
        //int i;
        uintptr_t test_addr = 0x4000000;

#if 0
        xqspi_flash_4K_subsector_erase(PLF_XQSPI_BASE, test_addr);

        xqspi_flash_fast_read(PLF_XQSPI_BASE, test_addr, buf, sizeof(buf), 0);
        pr_data(test_addr, buf, sizeof(buf));
#endif
        test_addr = 0x0;
        xqspi_flash_fast_read(PLF_XQSPI_BASE, test_addr, buf, sizeof(buf), 0);
        pr_data(test_addr, buf, sizeof(buf));

        test_addr = 0x4000000 - 128;
        xqspi_flash_fast_read(PLF_XQSPI_BASE, test_addr, buf, sizeof(buf), 0);
        pr_data(test_addr, buf, sizeof(buf));
    }

    printf("flash status=0x%02X\n",
        xqspi_flash_read_status(PLF_XQSPI_BASE, 0));
    printf("flash flags status=0x%02X\n",
        xqspi_flash_read_flags_status(PLF_XQSPI_BASE, 0));

    {
        uint32_t sum = 0;
        uintptr_t test_addr;
        int i, j;
        test_addr = 0;
        printf("start reading....\n");
        for (i = 0; i < 0x1000000 / sizeof(buf); i++) {
            xqspi_flash_fast_read(PLF_XQSPI_BASE, test_addr, buf, sizeof(buf), 0);
            for (j = 0; j < sizeof(buf); j++) {
                sum += buf[j];
            }
            test_addr += sizeof(buf);
            if (test_addr && !(test_addr & 0xFFFF)) {
                printf("\r%08lX", (unsigned long)test_addr);
            }
            if ((test_addr & (0x0400000 - 1)) == 0) {
                printf("\n\n%08lX Sum=%08X\n\n", (unsigned long)test_addr, (unsigned)sum);
            }
        }
        printf("done!\n");
        printf("Sum=%08X\n", (unsigned)sum);
    }

}

#endif // #ifdef PLF_XQSPI_BASE
