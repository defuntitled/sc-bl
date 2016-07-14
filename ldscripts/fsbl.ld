/*
 * Copyright (C) 2015, Syntacore Ltd.
 * All Rights Reserved.
 */

/**
*  Syntacore SCR* infra
*
*  @copyright Copyright (C) 2015-2019, Syntacore Ltd. All rights reserved.
*  @author mn-sc
*
*  @brief Bare metal linker script
**/

OUTPUT_ARCH( "riscv" )
ENTRY(_start)

MEMORY {
  RAM (rwx) : ORIGIN = 0-64K, LENGTH = 64K
}

SEC_ALIGN = 32;

STACK_SIZE = 16K;

SECTIONS {

  __start_of_ram : {
    _end = .;
  } > RAM

  .stack : {
    __STACK_START__ = .;
      LONG(0);
    . += STACK_SIZE - 4;
    __C_STACK_TOP__ = .;
    __STACK_END__ = .;
  } >RAM
/*
  .dtb ALIGN(4096) : {
    __DTB_PAGES = .;
    *(.dtb.rodata);
    FILL(0xffffffff);
   . = ALIGN(4096);
  } >RAM
*/
  .dtb ALIGN(16) : {
    __DTB_PAGES = .;
    *(.dtb.rodata);
    FILL(0xffffffff);
   . = ALIGN(16);
  } >RAM


  .bss ALIGN(SEC_ALIGN) : {
    __BSS_START__ = .;
    *(.dynbss) *(.bss .bss.* .gnu.linkonce.b.*) *(COMMON)
  } >RAM

  .sbss ALIGN(SEC_ALIGN) : {
    __global_pointer$ = . + 0x800;
    *(.sbss .sbss.* .gnu.linkonce.sb.*)
    *(.scommon)
    . = ALIGN(0x10);
    __BSS_END__ = .;
  } >RAM

  .data ALIGN(SEC_ALIGN) : {
    *(.data .data.*)
  } >RAM

  .sdata ALIGN(SEC_ALIGN) : {
    *(.sdata .sdata2.* .sdata.* .gnu.linkonce.s.*)
  } >RAM

  .srodata ALIGN(SEC_ALIGN) : {
    *(.srodata.cst16) *(.srodata.cst8) *(.srodata.cst4) *(.srodata.cst2) *(.srodata.*)
  } >RAM

  .rodata ALIGN(SEC_ALIGN) : {
    *(.rodata) *(.rodata.*) *(.gnu.linkonce.r.*)
  } >RAM

  /* thread-local data segments */
  .tdata ALIGN(SEC_ALIGN) : {
    _tls_data = .;
    _tdata_begin = .;
    KEEP(*(.tdata .tdata.* .gnu.linkonce.td.*));
    _tdata_end = .;
    . = ALIGN(0x10);
  } >RAM

  .tbss ALIGN(SEC_ALIGN) : {
    _tbss_begin = .;
    KEEP(*(.tbss .tbss.* .gnu.linkonce.tb.* .tcommon));
    . = ALIGN(SEC_ALIGN);
    _tbss_end = .;
  } >RAM

  __TLS_SIZE__ = _tbss_end - _tdata_begin;
  __TLS0_BASE__ = __C_STACK_TOP__ - __TLS_SIZE__;

  .text ALIGN(SEC_ALIGN) : {
    __TEXT_START__ = .;
    *(.text .text*)
    *(.reloc_l3_fixup)
    KEEP(*(.init.text));
    __TEXT_END__ = .;
  } >RAM

  .vectors ORIGIN(RAM) + LENGTH(RAM) - 0x1000 : SUBALIGN(0x1000) {
    KEEP(*(.vectors));
  } >RAM

  .reset_vector ORIGIN(RAM) + LENGTH(RAM) - 0x100 : {
    KEEP(*(.reset_vector));
    FILL(0);
    . = ALIGN(0x100) - 4;
    LONG(0);
  } >RAM

  /* scbl_sign_world : { */
  /*   LONG(0x78524353); */
  /* } >RAM */

  /DISCARD/ : {
    *(.eh_frame .eh_frame.*)
  }
}
