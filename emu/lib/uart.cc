#include <cstdio>
#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <cstdarg>

#include "sys.h"
#include "uart.h"

// 16550a UART emulation

typedef struct {
    ullong irq;
    uart_regs com;
} uart_state;

int emu_uart_create(emu_system *sys, emu_device_class *devclass, emu_device *dev, const char* devattrs)
{
    emu_io *io;
    uart_state *state = new uart_state{};
    dev->state = state;
    CHECK_ERROR(emu_map_io(&io, dev, emu_io_type_portio, 0, 0x3f8, 8));
    return 0;

error:
    return -1;
}

int emu_uart_destroy(emu_system *sys, emu_device_class *devclass, emu_device *dev)
{
    uart_state *state = (uart_state*)dev->state;
    delete state;
    return 0;
}

int emu_uart_io(emu_io *io, emu_io_op op, emu_io_addr addr, ullong size, ullong *val)
{
    uart_state *state = (uart_state*)io->dev->state;
    emu_tracef("emu_uart_io: op:%d addr:%llx size:%llx\n", op, addr, size);
    ullong va = addr - io->base;
    switch (op) {
    case emu_io_op_read:
        if (state->com.lcr & LCR_DLAB) {
            switch (va) {
            case REG_DLL: /* Divisor Latch LSB */
                *val = state->com.dll;
                break;
            case REG_DLM: /* Divisor Latch MSB */
                *val = state->com.dlm;
                break;
            case REG_LCR: /* Line Control Register */
                *val = state->com.lcr;
                break;
            case REG_MCR: /* MODEM Control Register */
                *val = state->com.mcr;
                break;
            default:
                break;
            }
        } else {
            switch (va) {
            case REG_RBR: /* Recieve Buffer Register */
                if (emu_console_has_char()) state->com.rbr = (uchar)emu_console_read_char();
                *val = state->com.rbr;
                break;
            case REG_IER: /* Interrupt Enable Register */
                *val = state->com.ier;
            case REG_IIR: /* Interrupt Identity Register */
                *val = emu_console_has_char() ? IIR_RD_LSR : IIR_TX_RDY;
                break;
            case REG_LCR: /* Line Control Register */
                *val = state->com.lcr;
                break;
            case REG_MCR: /* MODEM Control Register */
                *val = state->com.mcr;
                break;
            case REG_LSR: /* Line Status Register */
                *val = LSR_RI | (emu_console_has_char() ? LSR_DA : 0);
                break;
            case REG_MSR: /* MODEM Status Register */
                *val = MSR_DCD | MSR_DSR;
                break;
            case REG_SCR: /* Scratch Register */
                *val = state->com.scr;
                break;
            default:
                *val = 0;
                break;
            }
        }
        break;
    case emu_io_op_write:
        if (state->com.lcr & LCR_DLAB) {
            switch (va) {
            case REG_DLL: /* Divisor Latch LSB */
                state->com.dll = (uchar)*val;
                break;
            case REG_DLM: /* Divisor Latch MSB */
                state->com.dlm = (uchar)*val;
                break;
            case REG_LCR: /* Line Control Register */
                state->com.lcr = (uchar)*val;
                break;
            case REG_MCR: /* MODEM Control Register */
                state->com.mcr = (uchar)*val;
                break;
            default:
                break;
            }
        } else {
            switch (va) {
            case REG_THR: /* Transmist Holding Register */
                state->com.thr = (uchar)*val;
                emu_console_write_char((uchar)*val);
                break;
            case REG_IER: /* Interrupt Enable Register */
                state->com.ier = (uchar)*val & IER_MASK;
                break;
            case REG_FCR: /* FIFO Control Register */
                /* ignore writes */
                break;
            case REG_LCR: /* Line Control Register */
                state->com.lcr = (uchar)*val;
                break;
            case REG_MCR: /* MODEM Control Register */
                state->com.mcr = (uchar)*val;
                break;
            case REG_LSR: /* Line Status Register */
                /* ignore writes */
                break;
            case REG_MSR: /* MODEM Status Register */
                /* ignore writes */
                break;
            case REG_SCR: /* Scratch Register */
                state->com.scr = (uchar)*val;
                break;
            default:
                break;
            }
        }
        break;
    }
    return 0;
}

emu_device_class emu_uart = {
    "uart", "16550a compatible uart",
    emu_uart_create, emu_uart_destroy, emu_uart_io
};

// UART init

int emu_uart_init()
{
    emu_register_device_class(&emu_uart);
    return 0;
}
