#pragma once

#define IRQ_DELTA       32
#define IRQ_TIMER        0
#define IRQ_COM1         4
#define IRQ_ERROR       19
#define IRQ_SPURIOUS    31
#define MAX_IRQS        32

/* local APIC registers */

#define LAPIC_ID               0x02   // ID
#define LAPIC_VER              0x03   // Version
#define LAPIC_TPR              0x08   // Task Priority
#define LAPIC_APR              0x09   // Arbitration Priority
#define LAPIC_PPR              0x0A   // Processor Priority
#define LAPIC_EOI              0x0B   // EOI
#define LAPIC_LDR              0x0D   // Logical destination
#define LAPIC_DFR              0x0E   // Destination format
#define LAPIC_SVR              0x0F   // Spurious Interrupt Vector
#define LAPIC_SVR_ENABLE 0x00000100   // Unit Enable
#define LAPIC_SVR_EOISUP 0x00001000   // EOI Broadcast Supression
#define LAPIC_ISR0             0x10   // In-service bits 0:31
#define LAPIC_ISR7             0x17   // In-service bits 224:255
#define LAPIC_TMR0             0x18   // Trigger mode bits 0:31
#define LAPIC_TMR7             0x1F   // Trigger mode bits 224:255
#define LAPIC_IRR0             0x20   // Interrupt request bits 0:31
#define LAPIC_IRR7             0x27   // Interrupt request bits 224:255
#define LAPIC_ESR              0x28   // Error Status
#define LAPIC_ICRLO            0x30   // Interrupt Command [31:0]
#define LAPIC_ICRHI            0x31   // Interrupt Command [63:32]
#define LAPIC_INIT       0x00000500   // INIT/RESET
#define LAPIC_STARTUP    0x00000600   // Startup IPI
#define LAPIC_DELIVS     0x00001000   // Delivery status
#define LAPIC_ASSERT     0x00004000   // Assert interrupt (vs deassert)
#define LAPIC_DEASSERT   0x00000000
#define LAPIC_LEVEL      0x00008000   // Level triggered
#define LAPIC_BCAST_INC  0x00080000   // Send to all APICs, including self.
#define LAPIC_BCAST_EXC  0x000C0000   // Send to all APICs, excluding self.
#define LAPIC_BUSY       0x00001000
#define LAPIC_FIXED      0x00000000
#define LAPIC_TIMER            0x32   // Local Vector Table 0 (TIMER)
#define LAPIC_X1         0x0000000B   // divide counts by 1
#define LAPIC_X2         0x00000000   // divide counts by 2
#define LAPIC_X4         0x00000001   // divide counts by 4
#define LAPIC_X8         0x00000002   // divide counts by 8
#define LAPIC_X16        0x00000003   // divide counts by 16
#define LAPIC_X32        0x00000008   // divide counts by 32
#define LAPIC_X64        0x00000009   // divide counts by 64
#define LAPIC_X128       0x0000000A   // divide counts by 128
#define LAPIC_PERIODIC   0x00020000   // Periodic
#define LAPIC_DEADLINE   0x00040000   // Deadline
#define LAPIC_THERMAL          0x33   // Thermal Monitoring LVT
#define LAPIC_PCINT            0x34   // Performance Counter LVT
#define LAPIC_LINT0            0x35   // Local Vector Table 1 (LINT0)
#define LAPIC_LINT1            0x36   // Local Vector Table 2 (LINT1)
#define LAPIC_ERROR            0x37   // Local Vector Table 3 (ERROR)
#define LAPIC_MASKED     0x00010000   // Interrupt masked
#define LAPIC_TICR             0x38   // Timer Initial Count
#define LAPIC_TCCR             0x39   // Timer Current Count
#define LAPIC_TDCR             0x3E   // Timer Divide Configuration

/* APIC base register bits */

#define APIC_BASE_BSP (1 << 8)
#define APIC_BASE_ENABLE (1 << 11)
#define APIC_BASE_MASK 0xffffff000LL

#ifndef __ASSEMBLER__

#include "atomic.h"

extern char apstart16;
extern char stack_top;
extern atomic_int ap_ready;
extern atomic_int ap_valid;
extern atomic_int ap_count;
extern atomic_int aplist[];
extern atomic_int aplist_end[];
extern atomic_llong apstack[];
extern atomic_llong apstack_end[];

ullong x86_lapic_base();
int x86_lapic_init(int cpu);
uint x86_lapic_read(ullong addr, uint reg);
void x86_lapic_write(ullong addr, uint reg, ullong val);
void x86_lapic_write_icr(ullong addr, uint id, uint value);
ullong x86_lapic_read_icr(ullong addr);
void x86_lapic_eoi(ullong addr);
void x86_lapic_timer_div(ullong addr, int divider);
void x86_lapic_timer_icr(ullong addr, int counter);

#endif
