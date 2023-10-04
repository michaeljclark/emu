.section .init
reset:
.globl reset
.code16
  # output string to UART
  movw    $0x3f8, %dx
  mov $str, %di
  movb    (%di), %al
  testb    %al,%al
  jz done
loop:
  outb    %al, %dx
  inc %di
  movb    (%di), %al
  testb    %al,%al
  jnz loop
done:
  # halt by writing 0x800 to ICH RCR
  movw    $0xcf9, %dx
  movw    $0x800, %ax
  outw    %ax, %dx
spin:
  jmp     spin

.data
str:
  .asciz "hello world\n"
