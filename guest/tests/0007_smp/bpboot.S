#include "x86_cpu.h"

.section .init
reset:
.globl reset
.code16
  ljmpl $0x0, $bpstart16

.globl bpstart16
.globl bpstart32
.globl bpstart64
.globl bpstart_c

.globl gdtdesc
.globl stack_bot
.globl stack_top

# Load GDT and switch to 32-bit protected mode

.p2align 12, 0
bpstart16:
.code16
  # disable interrupts
  cli

  # load GDT
  lgdt gdtdesc

  # enable protected mode (CR0.PE=1)
  movl %cr0, %eax
  bts $0, %eax
  movl %eax, %cr0

  # long jmp to 32-bit protected mode reloading %cs and %eip
  ljmp    $(SEG_BCODE<<3), $bpstart32

bpstart32:
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
  ljmp    $(SEG_KCODE<<3), $bpstart64

.p2align 12, 0
.section .text
.code64
bpstart64:
  movw    $(SEG_KDATA<<3), %ax    # Our data segment
  movw    %ax, %ds                # -> DS: Data Segment
  movw    %ax, %es                # -> ES: Extra Segment
  movw    %ax, %ss                # -> SS: Stack Segment
  movw    %ax, %fs                # -> FS: User Segment
  movw    %ax, %gs                # -> GS: Kernel Segment

  # zero bss
  xor %eax, %eax
  mov $_bss_start, %rdi
  mov $_bss_end, %rcx
  sub %rdi, %rcx
  rep stosb

  # load boot stack pointer and call bpstart_c.
  xorq    %rbp, %rbp
  movq    $stack_top, %rsp
  call    bpstart_c

spin:
  jmp     spin

# GDT
.data
.p2align 12, 0
gdt:
  .long 0x00000000, 0x00000000    # null segment
  .long 0x0000ffff, 0x00cf9e00    # boot code (32-bit)
  .long 0x00000000, 0x00a09e00    # kern code (64-bit)
  .long 0x00000000, 0x00809200    # kern data (64-bit)

gdtdesc:
  .word   (gdtdesc - gdt - 1)     # sizeof(gdt) - 1
  .long   gdt                     # address gdt

# pml4-table
.globl pml4
.p2align 12, 0
pml4:
.quad pdpt + 3

# page-directory-pointer
.globl pdpt
.p2align 12, 0
pdpt:
.quad 0x00000000 + 0x83 # 1GiB page
.quad 0x40000000 + 0x83 # 1GiB page
.quad 0x80000000 + 0x83 # 1GiB page
.quad 0xc0000000 + 0x83 # 1GiB page

# stack
.p2align 4, 0
stack_bot:
.skip 1024
stack_top:
