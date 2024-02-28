.section .text
reset:
.globl reset
.code16
  # test A20
a20_1:
  inb     $0x64,%al               # Wait for not busy
  testb   $0x2,%al
  jnz     a20_1
  movb    $0xd1,%al               # 0xd1 -> port 0x64
  outb    %al,$0x64
a20_2:
  inb     $0x64,%al               # Wait for not busy
  testb   $0x2,%al
  jnz     a20_2
  movb    $0xdf,%al               # 0xdf -> port 0x60
  outb    %al,$0x60

  # halt by writing 0x800 to ICH RCR
  movw    $0xcf9, %dx
  movw    $0x800, %ax
  outw    %ax, %dx
spin:
  jmp     spin
