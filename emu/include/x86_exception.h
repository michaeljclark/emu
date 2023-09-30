#pragma once

enum {
    x86_exception_divide = 0,         /* #DE - DIV and IDIV instructions */
    x86_exception_debug = 1,          /* #DB - Any code or data reference */
    x86_exception_nmi = 2,            /* NMI - Non-maskable interrupt */
    x86_exception_breakpoint = 3,     /* #BP - INT3 instruction */
    x86_exception_overflow = 4,       /* #OF - INTO instruction */
    x86_exception_bound_range = 5,    /* #BR - BOUND Range Exceeded. */
    x86_exception_invalid_opcode = 6, /* #UD - UD instruction or reserved opcode */
    x86_exception_fpu_unavail = 7,    /* #NM - Floating-point or WAIT/FWAIT instruction */
    x86_exception_double_fault = 8,   /* #DF - Double Faulting instruction or interrupt */
    x86_exception_fpu_reserved = 9,   /* #MF - Floating-point instruction */
    x86_exception_invalid_tss = 10,   /* #TS - TSS Task switch or TSS access */
    x86_exception_segment_fault = 11, /* #NP - Segment not present */
    x86_exception_stack_fault = 12,   /* #SS - Stack operations and SS register */
    x86_exception_general_prot = 13,  /* #GP - Memory reference and protection checks */
    x86_exception_page_fault = 14,    /* #PF - Memory reference due to page fault */
    x86_exception_reserved = 15,      /* Reserved */
    x86_exception_math_fault = 16,    /* #MF - Floating-point instruction */
    x86_exception_align_check = 17,   /* #AC - Any data reference in memory */
    x86_exception_machine_check = 18, /* #MC - Machine check errors */
    x86_exception_simd_fault = 19,    /* #XM - SIMD Floating-Point Instruction */
    x86_exception_ept_exception = 20, /* #VE - EPT violations */
    x86_exception_cet_exception = 21  /* #CP - CET faulting instructions */
};
