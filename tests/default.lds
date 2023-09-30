OUTPUT_ARCH( "i386:x86-64" )

ENTRY( reset )

PHDRS
{
  init PT_LOAD;
  text PT_LOAD;
  data PT_LOAD;
}

SECTIONS
{
  .init : ALIGN(16) {
    PROVIDE(_init_start = .);
    *(.init .init.*)
    PROVIDE(_init_end = .);
  } :init

  .text : ALIGN(16) {
    PROVIDE(_text_start = .);
    *(.text .text.*)
    PROVIDE(_text_end = .);
  } :text

  .rodata : ALIGN(16) {
    PROVIDE(_rodata_start = .);
    *(.rodata .rodata.*)
    PROVIDE(_rodata_end = .);
  } :text

  .data : ALIGN(16) {
    PROVIDE(_data_start = .);
    *(.data .data.*)
    PROVIDE(_data_end = .);
  } :data

  .bss : ALIGN(16) {
    PROVIDE(_bss_start = .);
    *(.sbss .sbss.*) *(.bss .bss.*)
    PROVIDE(_bss_end = .);
  } :data

  PROVIDE(_bss_length = SIZEOF(.bss));
}
