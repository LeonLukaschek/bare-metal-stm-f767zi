ENTRY(_reset)

MEMORY
{
  flash (rx)  : ORIGIN = 0x08000000, LENGTH = 2048K    /* 2 MB Flash */

  /* itcm  (rx)  : ORIGIN = 0x00000000, LENGTH = 16K      ITCM RAM (Instruction Tightly Coupled Memory) */
  /* dtcm  (rwx) : ORIGIN = 0x20000000, LENGTH = 128K     DTCM RAM (Data Tightly Coupled Memory) */
  sram1 (rwx) : ORIGIN = 0x20020000, LENGTH = 368K     /* SRAM1 */
  /* sram2 (rwx) : ORIGIN = 0x2007C000, LENGTH = 16K      SRAM2 */
}

_estack = ORIGIN(sram1) + LENGTH(sram1); /* Stack pointer start at end of SRAM1 */

SECTIONS
{
  /* Vector table and interrupt handlers must be at beginning of flash */
  .vectors  : { KEEP(*(.vectors)) }   > flash

  /* Code and read-only data */
  .text     : { *(.text*) }           > flash
  .rodata   : { *(.rodata*) }         > flash

  /* Initialized data placed in SRAM1 */
  .data :
  {
    _sdata = .; /* .data section start */
    *(.data)
    _edata = .; /* .data section end */
  } > sram1 AT > flash
  _sidata = LOADADDR(.data);

  /* Uninitialized data */
  .bss :
  {
    _sbss = .; /* .bss section start */
    *(.bss*)
    *(COMMON)
    _ebss = .; /* .bss section end */
  } > sram1


  /* Align and end */
  . = ALIGN(8);
  _end = .;
}
