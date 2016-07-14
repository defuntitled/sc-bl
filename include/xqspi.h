/*
 * Copyright (C) 2018, Syntacore Ltd.
 * All Rights Reserved.
 */

#ifndef XQSPI_H
#define XQSPI_H

#define XSP_TX_FIFO_DEPTH           256
#define XSP_FLASH_PAGE_SIZE         256
#define XSP_FLASH_4K_PAGE_SIZE      (4 * 1024)

#define XSP_DGIER_OFFSET            0x1C	/*  Global Intr Enable Reg */
#define XSP_IISR_OFFSET             0x20	/*  Interrupt status Reg */
#define XSP_IIER_OFFSET             0x28	/*  Interrupt Enable Reg */
#define XSP_SRR_OFFSET              0x40	/*  Software Reset register */
#define XSP_CR_OFFSET               0x60	/*  Control register */
#define XSP_SR_OFFSET               0x64	/*  Status Register */
#define XSP_DTR_OFFSET              0x68	/*  Data transmit */
#define XSP_DRR_OFFSET              0x6C	/*  Data receive */
#define XSP_SSR_OFFSET              0x70	/*  32-bit slave select */
#define XSP_TFO_OFFSET              0x74	/*  Tx FIFO occupancy */
#define XSP_RFO_OFFSET              0x78	/*  Rx FIFO occupancy */

#define XSP_CR_LOOPBACK_MASK        0x00000001 /*  Local loopback mode */
#define XSP_CR_ENABLE_MASK          0x00000002 /*  System enable */
#define XSP_CR_MASTER_MODE_MASK	    0x00000004 /*  Enable master mode */
#define XSP_CR_CLK_POLARITY_MASK    0x00000008 /*  Clock polarity high or low */
#define XSP_CR_CLK_PHASE_MASK       0x00000010 /*  Clock phase 0 or 1 */
#define XSP_CR_TXFIFO_RESET_MASK    0x00000020 /*  Reset transmit FIFO */
#define XSP_CR_RXFIFO_RESET_MASK    0x00000040 /*  Reset receive FIFO */
#define XSP_CR_MANUAL_SS_MASK       0x00000080 /*  Manual slave select assert */
#define XSP_CR_TRANS_INHIBIT_MASK   0x00000100 /*  Master transaction inhibit */

#define XSP_CR_LSB_MSB_FIRST_MASK       0x00000200

#define XSP_CR_XIP_CLK_PHASE_MASK       0x00000001
#define XSP_CR_XIP_CLK_POLARITY_MASK    0x00000002

#define XSP_SR_RX_EMPTY_MASK            0x00000001 /*  Receive Reg/FIFO is empty */
#define XSP_SR_RX_FULL_MASK	            0x00000002 /*  Receive Reg/FIFO is full */
#define XSP_SR_TX_EMPTY_MASK            0x00000004 /*  Transmit Reg/FIFO is empty */
#define XSP_SR_TX_FULL_MASK	            0x00000008 /*  Transmit Reg/FIFO is full */
#define XSP_SR_MODE_FAULT_MASK          0x00000010 /*  Mode fault error */
#define XSP_SR_SLAVE_MODE_MASK          0x00000020 /*  Slave mode select */

#define XSP_SR_CPOL_CPHA_ERR_MASK       0x00000040 /*  CPOL/CPHA error */
#define XSP_SR_SLAVE_MODE_ERR_MASK      0x00000080 /*  Slave mode error */
#define XSP_SR_MSB_ERR_MASK	            0x00000100 /*  MSB Error */
#define XSP_SR_LOOP_BACK_ERR_MASK       0x00000200 /*  Loop back error */
#define XSP_SR_CMD_ERR_MASK	            0x00000400 /*  'Invalid cmd' error */
#define XSP_SR_ERR_MASK                 ( \
                                            XSP_SR_CPOL_CPHA_ERR_MASK | \
                                            XSP_SR_SLAVE_MODE_ERR_MASK | \
                                            XSP_SR_MSB_ERR_MASK | \
                                            XSP_SR_LOOP_BACK_ERR_MASK | \
                                            XSP_SR_CMD_ERR_MASK \
                                        )


#define XSP_SR_XIP_RX_EMPTY_MASK        0x00000001 /*  Receive Reg/FIFO	is empty */
#define XSP_SR_XIP_RX_FULL_MASK	        0x00000002 /*  Receive Reg/FIFO is full */
#define XSP_SR_XIP_MASTER_MODF_MASK     0x00000004 /*  Receive Reg/FIFO is full */
#define XSP_SR_XIP_CPHPL_ERROR_MASK	    0x00000008 /*  Clock Phase,Clock Polarity Error */
#define XSP_SR_XIP_AXI_ERROR_MASK       0x00000010 /*  AXI Transaction 	Error */

#define XSP_INTR_MODE_FAULT_MASK        0x00000001 /* Mode fault error */
#define XSP_INTR_SLAVE_MODE_FAULT_MASK  0x00000002 /* Selected as slave while disabled */
#define XSP_INTR_TX_EMPTY_MASK          0x00000004 /* DTR/TxFIFO is empty */
#define XSP_INTR_TX_UNDERRUN_MASK       0x00000008 /* DTR/TxFIFO underrun */
#define XSP_INTR_RX_FULL_MASK           0x00000010 /* DRR/RxFIFO is full */
#define XSP_INTR_RX_OVERRUN_MASK        0x00000020 /* DRR/RxFIFO overrun */
#define XSP_INTR_TX_HALF_EMPTY_MASK     0x00000040 /* TxFIFO is half empty */
#define XSP_INTR_SLAVE_MODE_MASK        0x00000080 /* Slave select mode */
#define XSP_INTR_RX_NOT_EMPTY_MASK      0x00000100 /* RxFIFO not empty */

#define XSP_SRR_RESET_MASK              0x0000000A

#define XSP_SSR_ENA	~(PLF_NVRAM_CHIPSELECT)
#define XSP_SSR_DIS	~0

void xqspi_flash_test(void);

uint32_t xqspi_flash_fast_read(uintptr_t xspi_base, uintptr_t start_addr,
    uint8_t *buf, uint32_t size, int interactive);

void xqspi_flash_write(uintptr_t xspi_base, uintptr_t start_addr,
    uint8_t *buf, uint32_t size, int interactive);

void xqspi_reset(uintptr_t xspi_base);

void xqspi_init(uintptr_t xspi_base);

#endif // #ifndef XQSPI_H
