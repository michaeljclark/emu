#define SEG_BCODE 1  // boot code
#define SEG_BDATA 2  // boot data

.section .text
reset:
.globl reset
.code16
  ljmpl $0x0, $start16

.globl start16
.globl start32

# Load GDT and switch to 32-bit protected mode

.p2align 12, 0
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
  movw    $(SEG_BDATA<<3), %ax    # Our data segment
  movw    %ax, %ds                # -> DS: Data Segment
  movw    %ax, %es                # -> ES: Extra Segment
  movw    %ax, %ss                # -> SS: Stack Segment

  # test cpuid
  mov $0, %eax
  mov $0, %ecx
  cpuid

  # halt by writing 0x800 to ICH RCR
  movw    $0xcf9, %dx
  movw    $0x800, %ax
  outw    %ax, %dx
spin:
  jmp     spin

# GDT
.data
.p2align 12, 0
gdt:
  .long 0x00000000, 0x00000000    # null segment
  .long 0x0000ffff, 0x00cf9e00    # boot code (32-bit)
  .long 0x0000ffff, 0x00cf9200    # boot data (32-bit)

gdtdesc:
  .word   (gdtdesc - gdt - 1)     # sizeof(gdt) - 1
  .long   gdt                     # address gdt
