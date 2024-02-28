.section .text
reset:
.globl reset
.code16
  # halt by writing 0x800 to ICH RCR
  movw    $0xcf9, %dx
  movw    $0x800, %ax
  outw    %ax, %dx
spin:
  jmp     spin
