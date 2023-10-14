#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "atomic.h"
#include "x86_mmu.h"
#include "x86_msr.h"
#include "x86_intrin.h"
#include "page.h"
#include "stack.h"
#include "vmm.h"
#include "x86_cpu.h"
#include "x86_lapic.h"

static inline uint io_read(volatile void *addr)
{
    return *(volatile uint*)addr;
}

static inline void io_write(volatile void *addr, uint val)
{
    *(volatile uint*)addr = val;
}

static inline ullong io_readl(volatile void *addr)
{
    return *(volatile ullong*)addr;
}

static inline void io_writel(volatile void *addr, ullong val)
{
    *(volatile ullong*)addr = val;
}

uint x86_lapic_read(ullong addr, uint reg)
{
    return io_read((volatile void *)(uintp)(addr + reg * 0x10));
}

void x86_lapic_write(ullong addr, uint reg, ullong val)
{
    io_write((volatile void *)(uintp)(addr + reg * 0x10), val);
}

void x86_lapic_write_icr(ullong addr, uint id, uint value)
{
    x86_lapic_write(addr, LAPIC_ICRHI, id);
    x86_lapic_write(addr, LAPIC_ICRLO, value);
}

ullong x86_lapic_read_icr(ullong addr)
{
    ullong hi = x86_lapic_read(addr, LAPIC_ICRHI);
    ullong lo = x86_lapic_read(addr, LAPIC_ICRLO);
    return (hi << 32) | lo;
}

void x86_lapic_eoi(ullong addr)
{
    x86_lapic_write(addr, LAPIC_EOI, 0);
}

void x86_lapic_timer_div(ullong addr, int divider)
{
    int tcr = 0;
    switch (divider) {
    case 1: tcr = LAPIC_X1; break;
    case 2: tcr = LAPIC_X2; break;
    case 4: tcr = LAPIC_X4; break;
    case 8: tcr = LAPIC_X8; break;
    case 16: tcr = LAPIC_X16; break;
    case 32: tcr = LAPIC_X32; break;
    case 64: tcr = LAPIC_X64; break;
    case 128: tcr = LAPIC_X128; break;
    default: break;
    }
    x86_lapic_write(addr, LAPIC_TDCR, tcr);
}

void x86_lapic_timer_icr(ullong addr, int counter)
{
    x86_lapic_write(addr, LAPIC_TICR, counter);
}

static int x86_lapic_compare(const void *a, const void *b)
{
    return *(const int*)a - *(const int*)b;
}

ullong x86_lapic_base()
{
    ullong base = x86_rdmsr(MSR_IA32_APIC_BASE);
    return base & APIC_BASE_MASK;;
}

int x86_lapic_init(int cpu)
{
    ullong base = x86_rdmsr(MSR_IA32_APIC_BASE);
    ullong addr = base & APIC_BASE_MASK;
    uint bsp = !!(base & APIC_BASE_BSP);
    uint enable = !!(base & APIC_BASE_ENABLE);
    uint lapic_id = x86_lapic_read(addr, LAPIC_ID);
    uint lapic_ver = x86_lapic_read(addr, LAPIC_VER);
    uint apicver = lapic_ver & 0xff;
    uint maxlvt = (lapic_ver >> 16) & 0xff;
    uint eoisup = (lapic_ver >> 24) & 1;

    printf("# lapic sp:%llx cpu:%d addr:%llx id:%x bsp:%d enable:%d version:%d maxlvt:%d eoisup:%d\n",
        (ullong)get_sp(), cpu, addr, lapic_id, bsp, enable, apicver, maxlvt, eoisup);

    // begin derived from xv64/kernel/lapic.c

    // Enable local APIC; set spurious interrupt vector.
    x86_lapic_write(addr, LAPIC_SVR, LAPIC_SVR_ENABLE | (IRQ_DELTA + IRQ_SPURIOUS));

    // Disable logical interrupt lines.
    x86_lapic_write(addr, LAPIC_LINT0, LAPIC_MASKED);
    x86_lapic_write(addr, LAPIC_LINT1, LAPIC_MASKED);

    // Disable performance counter and thermal interrupts
    // on machines that provide those LVT entries.
    if (maxlvt >= 5) {
        x86_lapic_write(addr, LAPIC_PCINT, LAPIC_MASKED);
        x86_lapic_write(addr, LAPIC_THERMAL, LAPIC_MASKED);
    }

    // Map error interrupt to IRQ_ERROR.
    x86_lapic_write(addr, LAPIC_ERROR, IRQ_DELTA + IRQ_ERROR);

    // Clear error status register (requires back-to-back writes).
    x86_lapic_write(addr, LAPIC_ESR, 0);
    x86_lapic_write(addr, LAPIC_ESR, 0);

    // Acknowledge outstanding interrupts
    x86_lapic_eoi(addr);

    // Send an Init Level De-Assert to synchronise arbitration ID's.
    x86_lapic_write_icr(addr, 0, LAPIC_BCAST_INC | LAPIC_INIT | LAPIC_LEVEL);
    while (x86_lapic_read_icr(addr) & LAPIC_DELIVS);

    // Enable interrupts on the APIC (but not on the processor).
    x86_lapic_write(addr, LAPIC_TPR, 0);

    // end derived from xv64/kernel/lapic.c

    cpulist[cpu].cpu = cpu;
    cpulist[cpu].lapic_id = lapic_id;

    x86_lapic_write(addr, LAPIC_TIMER, LAPIC_PERIODIC | (IRQ_DELTA + IRQ_TIMER));
    x86_lapic_timer_div(addr, 128);
    x86_lapic_timer_icr(addr, 1000000);

    /* application processor done initializing */
    if (!bsp) {
        atomic_fetch_add(&ap_count, 1);
        return 0;
    }

    /* boot processor startup intialization */

    atomic_store(&ap_ready, 1);
    atomic_store(&ap_valid, 0);
    atomic_store(&ap_count, 1);
    atomic_store(&aplist[0], lapic_id);
    atomic_store(&apstack[0], (llong)cpulist[0].kstack);
    memset(&aplist[1], -1, (uintp)&aplist_end[0] - (uintp)&aplist[1]);
    memset(&apstack[1], 0, (uintp)&apstack_end[0] - (uintp)&apstack[1]);

    if (ncpus == 1) return 0;

    /* send startup IPI */
    uint boot = (uint)((uintp)(&apstart16) >> 12);
    x86_lapic_write_icr(addr, 0, LAPIC_BCAST_EXC | LAPIC_STARTUP | boot);

    /* wait for CPUs to come up */
    while (atomic_load(&ap_ready) < ncpus);

    /* sort list of lapic IDs */
    qsort(&aplist[0], ncpus, sizeof(atomic_int), x86_lapic_compare);

    /* record cpu index to lapic ID and send stack */
    for (int i = 0; i < ncpus; i++) {
        cpulist[i].cpu = i;
        cpulist[i].lapic_id = atomic_load(&aplist[i]);
        atomic_store(&apstack[i], (llong)cpulist[i].kstack);
    }

    /* code depends on BSP having lowest integer lapic ID
     * because bpstart already pivoted to cpulist[0].kstack */
    if (cpulist[0].lapic_id != aplist[0]) abort();

    /* signal CPUs to start up */
    atomic_store(&ap_valid, 1);

    /* wait for CPUs to come up */
    while (atomic_load(&ap_count) < ncpus);

    /* print status */
    printf("# lapic init complete, %d cpus up\n", ncpus);

    return 0;
}
