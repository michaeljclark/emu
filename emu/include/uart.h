#pragma once

enum {
    REG_RBR      = 0,
    REG_THR      = 0,
    REG_DLL      = 0,
    REG_IER      = 1,
    REG_DLM      = 1,
    REG_IIR      = 2,
    REG_FCR      = 2,
    REG_LCR      = 3,
    REG_MCR      = 4,
    REG_LSR      = 5,
    REG_MSR      = 6,
    REG_SCR      = 7,

    IER_ERBDA    = 0x01,  /* Enable Recieved Buffer Data Available Interrupt */
    IER_ETHRE    = 0x02,  /* Enable Transmitter Holding Register Empty Interrupt */
    IER_ERLS     = 0x04,  /* Enable Reciever Line Status Interrupt */
    IER_EMSC     = 0x08,  /* Enable Modem Status Interrupt */
    IER_MASK     = 0x0f,  /* Interrupt Enable Mask */

    IIR_NOPEND   = 0x01,  /* No Interrupt Pending */
    IIR_RD_MSR   = 0x00,  /* Modem Status Change (Reset by MSR read) */
    IIR_TX_RDY   = 0x02,  /* Transmit Ready      (Reset by IIR read or THR write) */
    IIR_RX_RDY   = 0x04,  /* Receive Ready       (Reset by RBR read) */
    IIR_RD_LSR   = 0x06,  /* Read Line Status    (Reset by LSR read) */
    IIR_TIMEOUT  = 0x0c,  /* Read Timeout        (Reset by LSR read) */
    IIR_MASK     = 0x0f,  /* Interrupt Identification Mask */
    IIR_FIFO     = 0xc0,  /* FIFO Enabled */

    FCR_ENABLE   = 0x01,  /* FIFO Enable */
    FCR_RX_CLR   = 0x02,  /* FIFO Receive Clear */
    FCR_TX_CLR   = 0x04,  /* FIFO Transmit Clear */
    FCR_DMA      = 0x08,  /* FIFO DMA */
    FCR_RX_MASK  = 0xc0,  /* FIFO Trigger Mask (1,4,8,14) */

    LCR_5BIT     = 0x00,  /* 5-bit */
    LCR_6BIT     = 0x01,  /* 6-bit */
    LCR_7BIT     = 0x02,  /* 7-bit */
    LCR_8BIT     = 0x03,  /* 8-bit */
    LCR_BMASK    = 0x07,  /* Bit mask */
    LCR_STOPB    = 0x04,  /* Stop bit */
    LCR_PNONE    = 0x00,  /* Parity None */
    LCR_PODD     = 0x08,  /* Parity Odd */
    LCR_PEVEN    = 0x18,  /* Parity Even */
    LCR_PHIGH    = 0x28,  /* Parity High */
    LCR_PLOW     = 0x38,  /* Parity Low */
    LCR_PMASK    = 0x38,  /* Parity Mask */
    LCR_BREAK    = 0x40,  /* Break Signal Enabled */
    LCR_DLAB     = 0x80,  /* Divisor Latch Bit */

    MCR_DTR      = 0x01,  /* Data Terminal Ready */
    MCR_RTS      = 0x02,  /* Request To Send */
    MCR_OUT1     = 0x04,  /* Output 1 */
    MCR_OUT2     = 0x08,  /* Output 2 */
    MCR_LOOP     = 0x10,  /* Loopback Mode */

    LSR_DA       = 0x01,  /* Data Available */
    LSR_OE       = 0x02,  /* Overrun Error */
    LSR_PE       = 0x04,  /* Parity Error */
    LSR_FE       = 0x08,  /* Framing Error */
    LSR_BS       = 0x10,  /* Break Signal */
    LSR_RE       = 0x20,  /* THR is empty */
    LSR_RI       = 0x40,  /* THR is empty and line is idle */
    LSR_EF       = 0x80,  /* Erroneous data in FIFO */

    MSR_DCTS     = 0x01,  /* Delta Clear To Send */
    MSR_DDSR     = 0x02,  /* Delta Data Set Ready */
    MSR_TERI     = 0x04,  /* Trailing Edge Ring Indicator */
    MSR_DDCD     = 0x08,  /* Delta Carrier Detect */
    MSR_CTS      = 0x10,  /* Clear to Send */
    MSR_DSR      = 0x20,  /* Data Set Ready */
    MSR_RI       = 0x40,  /* Ring Indicator */
    MSR_DCD      = 0x80,  /* Data Carrier Detect */

    FIFOSZ       = 16,    /* FIFO size costant */
};

typedef struct {
    uchar rbr;              /* (R  [0]) Recieve Buffer Register        */
    uchar thr;              /* (W  [0]) Transmit Holding Register      */
    uchar ier;              /* (RW [1]) Interrupt Enable Register      */
    uchar iir;              /* (R  [2]) Interrupt Identity Register    */
    uchar fcr;              /* (W  [2]) FIFO Control Register          */
    uchar lcr;              /* (RW [3]) Line Control Register          */
    uchar mcr;              /* (RW [4]) MODEM Control Register         */
    uchar lsr;              /* (RW [5]) Line Status Register           */
    uchar msr;              /* (RW [6]) MODEM Status Register          */
    uchar scr;              /* (RW [7]) Scratch Register               */
    uchar dll;              /* (RW [0]) Divisor Latch LSB (LCR.DLAB=1) */
    uchar dlm;              /* (RW [1]) Divisor Latch MSB (LCR.DLAB=1) */
} uart_regs;
