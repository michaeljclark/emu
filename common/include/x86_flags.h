#pragma once

#define X86_CR0_PE         (1 << 0) // Protected Mode Enable
#define X86_CR0_MP         (1 << 1) // Monitor co-processor
#define X86_CR0_EM         (1 << 2) // x87 FPU Emulation
#define X86_CR0_TS         (1 << 3) // Task switched
#define X86_CR0_ET         (1 << 4) // Extension type
#define X86_CR0_NE         (1 << 5) // Numeric error
#define X86_CR0_WP         (1 << 16) // Write protect
#define X86_CR0_AM         (1 << 18) // Alignment mask
#define X86_CR0_NW         (1 << 29) // Not-write through
#define X86_CR0_CD         (1 << 30) // Cache disable
#define X86_CR0_PG         (1 << 31) // Paging

#define X86_CR4_VME        (1 << 0) // Virtual 8086 Mode Extensions
#define X86_CR4_PVI        (1 << 1) // Protected-mode Virtual Interrupts
#define X86_CR4_TSD        (1 << 2) // Time Stamp Disable
#define X86_CR4_DE         (1 << 3) // Debugging Extensions
#define X86_CR4_PSE        (1 << 4) // Page Size Extension
#define X86_CR4_PAE        (1 << 5) // Physical Address Extension
#define X86_CR4_MCE        (1 << 6) // Machine Check Exception
#define X86_CR4_PGE        (1 << 7) // Page Global Enabled
#define X86_CR4_PCE        (1 << 8) // Performance-Monitoring Counter enable
#define X86_CR4_OSFXSR     (1 << 9) // Support for FXSAVE/FXRSTOR instructions
#define X86_CR4_OSXMMEXCPT (1 << 10) // Support for Unmasked SIMD Floating-Point Exceptions
#define X86_CR4_UMIP       (1 << 11) // User-Mode Instruction Prevention (CPL > 0)
#define X86_CR4_VMXE       (1 << 13) // Virtual Machine Extensions Enable
#define X86_CR4_SMXE       (1 << 14) // Safer Mode Extensions Enable
#define X86_CR4_FSGBASE    (1 << 16) // Support for RDFSBASE, RDGSBASE, WRFSBASE, and WRGSBASE
#define X86_CR4_PCIDE      (1 << 17) // PCID Enable
#define X86_CR4_OSXSAVE    (1 << 18) // Processor Extended States Enable
#define X86_CR4_SMEP       (1 << 20) // Supervisor Mode Execution Protection Enable
#define X86_CR4_SMAP       (1 << 21) // Supervisor Mode Access Prevention Enable
#define X86_CR4_PKE        (1 << 22) // Protection Key Enable
#define X86_CR4_CET        (1 << 23) // Control-flow Enforcement Technology
#define X86_CR4_PKS        (1 << 24) // Enable Protection Keys for Supervisor-Mode Pages

#define X86_XCR_X87        (1 << 0) // x87 FPU execution environment
#define X86_XCR_SSE        (1 << 1) // streaming SIMD extensions
#define X86_XCR_AVX        (1 << 2) // AVX state
#define X86_XCR_BNDREGS    (1 << 3) // MPX bound registers
#define X86_XCR_BNDCSR     (1 << 4) // MPS control and status registers
#define X86_XCR_AVX5_MASK  (1 << 5) // AVX-512 opmask state
#define X86_XCR_AVX5_HI256 (1 << 6) // AVX-512 upper 256 bits of the registers ZMM0–ZMM15
#define X86_XCR_AVX5_HI16  (1 << 7) // AVX-512 16 512-bit registers ZMM16–ZMM31
#define X86_XCR_PT         (1 << 8) // Intel Processor Trace
#define X86_XCR_PKRU       (1 << 9) // Intel Protection Keys
#define X86_XCR_PASID      (1 << 10) // Intel Process Address Space MSR state
#define X86_XCR_CET_U      (1 << 11) // Intel CET user-mode functionality
#define X86_XCR_CET_S      (1 << 12) // Intel CET shadow-stack pointers for privilege levels 0–2
#define X86_XCR_HDC        (1 << 13) // Intel Hardware Duty Cycling MSR state
#define X86_XCR_UINTR      (1 << 14) // Intel User Interrupts state
#define X86_XCR_LBR        (1 << 15) // Intel Last Branch Record state
#define X86_XCR_HWP        (1 << 16) // Intel Hardwrare P-states MSR state
#define X86_XCR_AMX_CFG    (1 << 17) // Intel AMX 64-byte TILECFG register state
#define X86_XCR_AMX_DATA   (1 << 18) // Intel AMX 8192 bytes of tile data state
