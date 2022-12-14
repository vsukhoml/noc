/* Copyright 2022 Vadim Sukhomlinov
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

/* Minimalistic, statically linked Linux executable. */
ENTRY(_start)

/* Define fake memory regions for testing. */
MEMORY
{
  /* Shift ORIGIN for ROM to avoid alignment with ELF headers. */
	ROM (rxa!w) : ORIGIN = 0x400100, LENGTH = 0x10000000 - 0x100
	RAM (wxa!r) : ORIGIN = 0x20000000, LENGTH = 0x10000000
}

PHDRS
{
	text PT_LOAD;
  /* data PT_LOAD; */
	bss PT_LOAD;
}

SECTIONS
{
  /* .rodata holds read-only data that typically contribute to a non-writable
   * segment in the process image. */
  .rodata : {
    PROVIDE(__tests_start = .);
    KEEP(*(SORT(.rodata.tests.*)))
    PROVIDE(__tests_end = .);
    PROVIDE(__bench_start = .);
    KEEP(*(SORT(.rodata.bench.*)))
    PROVIDE(__bench_end = .);
    *(.note.gnu.build-id)
    *(.rodata .rodata.* .gnu.linkonce.r.*)
    *(.lrodata .lrodata.* .gnu.linkonce.lr.*)
    *(.rodata1)
    /* Place zero at the end. */
    LONG(0)
    . = ALIGN(16);
    _rodata_end = .;
  } > ROM AT>ROM :text
    PROVIDE(_rodata_start = ADDR(.rodata));

  /* .text holds executable instructions. */
  .text ALIGN(16) : {
    *(.text.unlikely .text.*_unlikely .text.unlikely.*)
    *(.text.exit .text.exit.*)
    *(.text.startup .text.startup.*)
    *(.text.hot .text.hot.*)
    *(SORT(.text.sorted.*))
    *(.text .stub .text.* .gnu.linkonce.t.*)

    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);

    PROVIDE_HIDDEN ( __init_array_start = . );
		KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
		KEEP (*(.init_array .ctors))
		PROVIDE_HIDDEN ( __init_array_end = . );

		PROVIDE_HIDDEN ( __fini_array_start = . );
		KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
		KEEP (*(.fini_array .dtors))
		PROVIDE_HIDDEN ( __fini_array_end = . );

    . = ALIGN(16);
		*(.gcc_except_table *.gcc_except_table.*)
    /*	KEEP (*(.eh_frame .eh_frame.*)) */
    *(.eh_frame .eh_frame.*)
		*(.ARM.extab* .gnu.linkonce.armextab.*)
    . = ALIGN(16);
    		__text_end = .;
  } > ROM AT>ROM :text
  PROVIDE (_etext = .);

  /* TODO: There is difference in behavior for ld and lld,
   * causing .data section to not be properly loaded.
   * marking .data as :bss makes it being loaded at proper
   * VMA without need to copy from LMA(!?). This set combo
   * provides consistent behavior for ld and lld. */
  .data : {
    *(.data .data.* .gnu.linkonce.d.* .data1)
    *(.sdata .sdata.* .sdata2.*)
    *(.ldata .ldata.* .gnu.linkonce.l.*)
     /* Place zero word at the end */
    LONG(0)
    . = ALIGN(16);
    PROVIDE(_data_end = .);
  } >RAM AT>ROM :bss /* Should be :data? */
  PROVIDE(_data_start = ADDR(.data));
  PROVIDE(_data_source = LOADADDR(.data));

  .bss (NOLOAD) :
  {
    _bss_start = .;
    *(.dynbss)
    *(.bss .bss.* .gnu.linkonce.b.*)
    *(.dynlbss)
    *(.lbss .lbss.* .gnu.linkonce.lb.*)
    *(COMMON)
    *(LARGE_COMMON)
   . = ALIGN(8);
    _bss_end = .;
  } > RAM AT>RAM :bss
  PROVIDE( _bss_size = _bss_end - _bss_start );
  PROVIDE (_brk_start = .);
  PROVIDE (_brk_end = LENGTH(RAM) + ORIGIN(RAM));

  .shstrtab       : { *(.shstrtab) }

  .strtab          0 : { *(.strtab) }
  .symtab          0 : { *(.symtab) }

  /* Stabs debugging sections.  */
  .stab          0 : { *(.stab) }
  .stabstr       0 : { *(.stabstr) }
  .stab.excl     0 : { *(.stab.excl) }
  .stab.exclstr  0 : { *(.stab.exclstr) }
  .stab.index    0 : { *(.stab.index) }
  .tab.indexstr 0 : { *(.stab.indexstr) }
  /* DWARF 1.  */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions.  */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2.  */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2.  */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line .debug_line.* .debug_line_end) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* SGI/MIPS DWARF 2 extensions.  */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
  /* DWARF 3.  */
  .debug_pubtypes 0 : { *(.debug_pubtypes) }
  .debug_ranges   0 : { *(.debug_ranges) }
  /* DWARF 5.  */
  .debug_addr     0 : { *(.debug_addr) }
  .debug_line_str 0 : { *(.debug_line_str) }
  .debug_loclists 0 : { *(.debug_loclists) }
  .debug_macro    0 : { *(.debug_macro) }
  .debug_names    0 : { *(.debug_names) }
  .debug_rnglists 0 : { *(.debug_rnglists) }
  .debug_str_offsets 0 : { *(.debug_str_offsets) }
  .debug_sup      0 : { *(.debug_sup) }
  /DISCARD/ : {
     *(.note.GNU-stack)
     *(.gnu_debuglink)
     *(.gnu.lto_*)
     *(.gnu.version)
     *(.gnu.version_d)
     *(.gnu.version_r)
     *(.note .note.*)
     *(.eh_frame .eh_frame.*)
     *(.ARM.extab* .gnu.linkonce.armextab.*)
     *(.ARM.exidx*)
     *(.gnu.attributes)
     *(.gnu.build.attributes .gnu.build.attributes.*)
     *(.hash)
     *(.gnu.hash)
     *(.dynsym)
     /* This section holds strings needed for dynamic linking, most
      * commonly the strings that represent the names associated with
      * symbol table entries. */
     *(.dynstr)
     *(.gnu.version)
     *(.gnu.version_d)
     *(.gnu.version_r)
     *(.gnu.warning)
     *(.comment)
     /* We don't use relocations. */
     *(.rela.* .iplt .igot.*)
     *(.got .got.*)
     *(.data.rel.ro.local* .gnu.linkonce.d.rel.ro.local.*)
     *(.data.rel.ro .data.rel.ro.* .gnu.linkonce.d.rel.ro.*)

   }
}
