#include "x86_msr.h"
#include "x86_lapic.h"
#include "x86_cpu.h"
#include "x86_flags.h"

.globl apstart16
.globl apstart32
.globl apstart64
.globl apstart_c

.globl ap_count
.globl ap_ready
.globl ap_valid
.globl aplist
.globl aplist_end
.globl apstack
.globl apstack_end

.section .data
.p2align 3, 0
ap_count:
.long 0
ap_ready:
.long 0
ap_valid:
.long 0
aplist:
.rept MAXCPUS
.long 0
.endr
aplist_end:
apstack:
.rept MAXCPUS
.quad 0
.endr
apstack_end:

.section .init
.p2align 12, 0
.code16
apstart16:
  # disable interrupts
  cli

  # load GDT
  lgdt gdtdesc

  # enable protected mode (CR0.PE=1)
  movl %cr0, %eax
  bts $0, %eax
  movl %eax, %cr0

  # long jmp to 32-bit protected mode reloading %cs and %eip
  ljmp    $(SEG_BCODE<<3), $apstart32

apstart32:
.code32

  # enable PAE (CR4.PAE=1)
  mov %cr4, %eax
  bts $5, %eax
  mov %eax, %cr4

  # load page table root (CR3=$pml4)
  mov $pml4, %eax
  mov %eax, %cr3

  # enable long mode (EFER.LME=1)
  mov $0xc0000080, %ecx
  rdmsr
  bts $8, %eax
  wrmsr

  # enable paging/caching (CR0.{PG=1,CD=0,NW=0,WP=1})
  mov %cr0, %eax
  bts $31, %eax
  btc $30, %eax
  btc $29, %eax
  bts $16, %eax
  mov %eax, %cr0

  # long jmp to 64-bit long mode reloading %cs and %rip
  ljmp    $(SEG_KCODE<<3), $apstart64

.p2align 12, 0
.section .text
.code64
apstart64:
  movw    $(SEG_KDATA<<3), %ax    # Our data segment
  movw    %ax, %ds                # -> DS: Data Segment
  movw    %ax, %es                # -> ES: Extra Segment
  movw    %ax, %ss                # -> SS: Stack Segment
  movw    %ax, %fs                # -> FS: User Segment
  movw    %ax, %gs                # -> GS: Kernel Segment

  # set CR0.MP, clear CR0.EM
  mov %cr0, %rax
  or $X86_CR0_MP, %rax
  and $~X86_CR0_EM, %rax
  mov %rax, %cr0

  # set CR4.OSFXSR, CR4.OSXMMEXCPT, CR4.OSXSAVE
  mov %cr4, %rax
  or $(X86_CR4_OSFXSR|X86_CR4_OSXMMEXCPT|X86_CR4_OSXSAVE), %rax
  mov %rax, %cr4

  # set XCR.X87, XCR.SSE, XCR.AVX
  xor %rcx, %rcx
  xgetbv
  or $(X86_XCR_X87|X86_XCR_SSE|X86_XCR_AVX), %rax
  xsetbv

  # read local APIC base address
  mov     $MSR_IA32_APIC_BASE, %ecx
  rdmsr
  shlq    $32, %rdx
  orq     %rax, %rdx
  movq    $APIC_BASE_MASK, %rax
  andq    %rax, %rdx

  # read local APIC id
  mov     (LAPIC_ID * 0x10)(%rdx), %ebx

  # write local APIC id to a free slot in aplist
  movq    $aplist, %rdi
  movq    $aplist_end, %rsi
aploop1:
  mov     $-1, %eax
  lock    cmpxchg %ebx, (%rdi)
  jz      apready
  add     $4, %rdi
  cmp     %rsi, %rdi
  je      apspin
  jmp     aploop1

apready:
  # increment ap_ready
  movq    $ap_ready, %r8
  mov     $1, %eax
  lock    xadd %eax, (%r8)

  # wait for BSP to sort list
  movq    $ap_valid, %r8
apwait:
  mov     (%r8), %r9
  cmp     $1, %r9
  jne     apwait

  # find our APIC id in aplist
  movq    $aplist, %rdi
  movq    $aplist_end, %rsi
aploop2:
  mov     (%rdi), %eax
  cmp     %eax, %ebx
  jz      apfound
  add     $4, %rdi
  cmp     %rsi, %rdi
  je      apspin
  jmp     aploop2

apfound:
  # turn address into index
  movq    $aplist, %rsi
  sub     %rsi, %rdi
  shr     $2, %edi

  # apic id is in %ebx
  # cpu idx is in %edi

  # load our stack pointer and call apstart_c
  movq    $apstack, %rsi
  movq    (%rsi,%rdi,8), %rsp
  xorq    %rbp, %rbp
  call    apstart_c

  # spin
apspin:
  jmp     apspin
