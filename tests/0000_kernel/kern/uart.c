#include "types.h"
#include "x86_intrin.h"
#include "console.h"
#include "uart.h"

#define UART0_CLOCK_FREQ 1843200
#define UART0_BAUD_RATE 115200

#define UART0_IO_BASE 0x3f8

void ns16550a_init()
{
    uint divisor = UART0_CLOCK_FREQ / (16 * UART0_BAUD_RATE);
    x86_outb(UART0_IO_BASE + REG_LCR, LCR_DLAB);
    x86_outb(UART0_IO_BASE + REG_DLL, divisor & 0xff);
    x86_outb(UART0_IO_BASE + REG_DLM, (divisor >> 8) & 0xff);
    x86_outb(UART0_IO_BASE + REG_LCR, LCR_PODD | LCR_8BIT);
}

int ns16550a_getchar()
{
    if (x86_inb(UART0_IO_BASE + REG_LSR) & LSR_DA) {
        return x86_inb(UART0_IO_BASE + REG_RBR);
    } else {
        return -1;
    }
}

int ns16550a_putchar(int ch)
{
    while ((x86_inb(UART0_IO_BASE + REG_LSR) & LSR_RI) == 0);
    x86_outb(UART0_IO_BASE + REG_THR, ch & 0xff);
    return ch;
}

console_device_t console_ns16550a = {
    ns16550a_init,
    ns16550a_getchar,
    ns16550a_putchar
};