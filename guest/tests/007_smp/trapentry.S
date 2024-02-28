#include "x86_flags.h"

.globl trap
.globl trapentry

trapentry:
    push %r15
    push %r14
    push %r13
    push %r12
    push %r11
    push %r10
    push %r9
    push %r8
    push %rdi
    push %rsi
    push %rbp
    push %rdx
    push %rcx
    push %rbx
    push %rax

    xor %edx, %edx
    movl $(X86_XCR_X87 | X86_XCR_SSE | X86_XCR_AVX), %eax
    movq %gs:(16), %rcx
    xsave64 0(%rcx)

    # %rdi (argo) points to trapframe
    mov %rsp, %rdi
    call trap

    xor %edx, %edx
    movl $(X86_XCR_X87 | X86_XCR_SSE | X86_XCR_AVX), %eax
    movq %gs:(16), %rcx
    xrstor64 0(%rcx)

    pop %rax
    pop %rbx
    pop %rcx
    pop %rdx
    pop %rbp
    pop %rsi
    pop %rdi
    pop %r8
    pop %r9
    pop %r10
    pop %r11
    pop %r12
    pop %r13
    pop %r14
    pop %r15

    # discard trapno and errorcode
    add $16, %rsp
    iretq
