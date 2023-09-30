#define SEG_BCODE 1  // boot code
#define SEG_KCODE 2  // kern code
#define SEG_KDATA 3  // kern data

.section .init
reset:
.globl reset
.code16
  ljmpl $0x0, $start16

.globl start16
.globl start32
.globl start64

# Load GDT and switch to 32-bit protected mode

.section .text
.p2align 12
start16:
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
  ljmp    $(SEG_BCODE<<3), $start32

start32:
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

  # enable paging (CR0.PG=1)
  mov %cr0, %eax
  bts $31, %eax
  mov %eax, %cr0

  # long jmp to 64-bit long mode reloading %cs and %rip
  ljmp    $(SEG_KCODE<<3), $start64

.code64
start64:
  movw    $(SEG_KDATA<<3), %ax    # Our data segment
  movw    %ax, %ds                # -> DS: Data Segment
  movw    %ax, %es                # -> ES: Extra Segment
  movw    %ax, %ss                # -> SS: Stack Segment
  movw    %ax, %fs                # -> FS: User Segment
  movw    %ax, %gs                # -> GS: Kernel Segment

  # zero bss
  xor %eax, %eax
  mov $_bss_start, %rdi
  mov $_bss_length, %rcx
  rep stosb

  # test mmio
  movl $0x40000000, %ecx
  movl $0, (%ecx)

  # halt by writing 0x800 to ICH RCR
  movw    $0xcf9, %dx
  movw    $0x800, %ax
  outw    %ax, %dx
spin:
  jmp     spin

# GDT
.data
.p2align 12
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
.p2align 12
pml4:
.quad pdpt + 3
.skip 4088

# page-directory-pointer
.globl pdpt
.p2align 12
pdpt:
.quad 0x00000000 + 0x83 # 1GiB page
.quad 0x40000000 + 0x83 # 1GiB page
.quad 0x80000000 + 0x83 # 1GiB page
.quad 0xc0000000 + 0x83 # 1GiB page
.skip 4064
