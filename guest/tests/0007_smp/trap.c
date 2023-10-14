#include "stdio.h"
#include "trap.h"
#include "x86_msr.h"
#include "x86_intrin.h"
#include "x86_lapic.h"
#include "x86_exception.h"
#include "x86_cpu.h"
#include "poweroff.h"
#include "spinlock.h"

static spinlock trap_lock;

#define array_size(arr) (sizeof(arr)/sizeof(arr[0]))

static const char *exception_names[22] = {
    [x86_exception_divide]         = "divide",
    [x86_exception_debug]          = "debug",
    [x86_exception_nmi]            = "nmi",
    [x86_exception_breakpoint]     = "breakpoint",
    [x86_exception_overflow]       = "overflow",
    [x86_exception_bound_range]    = "bound_range",
    [x86_exception_invalid_opcode] = "invalid_opcode",
    [x86_exception_fpu_unavail]    = "fpu_unavail",
    [x86_exception_double_fault]   = "double_fault",
    [x86_exception_fpu_reserved]   = "fpu_reserved",
    [x86_exception_invalid_tss]    = "invalid_tss",
    [x86_exception_segment_fault]  = "segment_fault",
    [x86_exception_stack_fault]    = "stack_fault",
    [x86_exception_general_prot]   = "general_prot",
    [x86_exception_page_fault]     = "page_fault",
    [x86_exception_reserved]       = "reserved",
    [x86_exception_math_fault]     = "math_fault",
    [x86_exception_align_check]    = "align_check",
    [x86_exception_machine_check]  = "machine_check",
    [x86_exception_simd_fault]     = "simd_fault",
    [x86_exception_ept_exception]  = "ept_exception",
    [x86_exception_cet_exception]  = "cet_exception"
};

void trap(trapframe *tf)
{
    ullong tsc_delta = x86_rdtsc() - cpulist[0].init_tsc;
    if (tf->trapno == IRQ_DELTA + IRQ_TIMER) {
        x86_lapic_eoi(x86_lapic_base());
        spinlock_lock(&trap_lock);
        printf("trap: timer tscdelta=%llx trapno=%lld tf=%p tf->rip=%llx\n",
            tsc_delta, tf->trapno, tf, tf->rip);
        spinlock_unlock(&trap_lock);
    } else {
        const char* trap_name = tf->trapno < array_size(exception_names) ?
            exception_names[tf->trapno] : "unhandled";
        spinlock_lock(&trap_lock);
        printf("trap: %s tscdelta=%llx trapno=%lld tf=%p tf->rip=%llx\n",
            trap_name, tsc_delta, tf->trapno, tf, tf->rip);
        spinlock_unlock(&trap_lock);
        poweroff_halt(1);
    }
}
