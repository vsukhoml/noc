/* Copyright 2022 Vadim Sukhomlinov
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

/* Single-segment variant of x86_64-pc-linux-gnu.ld.
 *
 * Selected via the Makefile switch:  make test LINUX_LAYOUT=single
 * (which also adds -Wl,--no-warn-rwx-segments and -DNOC_SINGLE_SEGMENT=1).
 * The default LINUX_LAYOUT=split uses the two-segment ROM/RAM script.
 *
 * The default script models an embedded ROM/RAM split with two PT_LOAD
 * segments; the second (RAM, VMA 0x20000000) is file-backed, which forces a
 * page of padding so its file offset stays page-congruent with its VMA (the
 * kernel mmaps file-backed segments page-aligned). That ~1 KB of padding is
 * dead weight in the file.
 *
 * Here we instead use ONE PT_LOAD covering .rodata/.text/.data and a NOLOAD
 * .bss/.heap, all with contiguous VMAs. Because .data sits right after .text in
 * the same segment there is no inter-segment gap to begin with -- so the saving
 * does NOT need -Wl,--nmagic (which would only shave the ~190 B of header
 * padding and is fragile). The one rule we must still honour is the kernel's
 * file-offset == VMA congruence (mod page size) for the single file-backed
 * segment: the headers occupy file [0, 0x100) and ORIGIN starts at 0x400100, so
 * the first section is at file offset 0x100 and VMA 0x400100 -- both ==0x100
 * (mod 0x1000). Tradeoff: the segment is RWX and the ROM/RAM separation (with
 * its LMA!=VMA .data relocation) is gone, so _start never needs a copy loop.
 */
ENTRY(_start)

MEMORY
{
	MEM (rwxa) : ORIGIN = 0x400100, LENGTH = 0x10000000 - 0x100
}

PHDRS
{
	load PT_LOAD;
	tls  PT_TLS;
}

SECTIONS
{
  .rodata : {
    PROVIDE(__tests_start = .);
    KEEP(*(SORT(.rodata.tests.*)))
    PROVIDE(__tests_end = .);
    PROVIDE(__bench_start = .);
    KEEP(*(SORT(.rodata.bench.*)))
    PROVIDE(__bench_end = .);
    *(.rodata .rodata.* .gnu.linkonce.r.*)
    *(.lrodata .lrodata.* .gnu.linkonce.lr.*)
    *(.rodata1)
    LONG(0)
    . = ALIGN(16);
    _rodata_end = .;
  } > MEM :load
  PROVIDE(_rodata_start = ADDR(.rodata));

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
    __text_end = .;
  } > MEM :load
  PROVIDE (_etext = .);

  /* Single segment: .data keeps VMA == LMA and is loaded in place by the
   * kernel, so _start still needs no copy loop. */
  .data : {
    *(.data .data.* .gnu.linkonce.d.* .data1)
    *(.sdata .sdata.* .sdata2.*)
    *(.ldata .ldata.* .gnu.linkonce.l.*)
    LONG(0)
    . = ALIGN(16);
    PROVIDE(_data_end = .);
  } > MEM :load
  PROVIDE(_data_start = ADDR(.data));
  PROVIDE(_data_source = LOADADDR(.data));

  .tdata : {
    PROVIDE(__tdata_start = .);
    *(.tdata .tdata.*)
    PROVIDE(__tdata_end = .);
    . = ALIGN(8);
  } > MEM AT>MEM :load :tls

  .tbss (NOLOAD) : {
    PROVIDE(__tbss_start = .);
    *(.tbss .tbss.*)
    PROVIDE(__tbss_end = .);
    . = ALIGN(8);
  } > MEM :tls

  .bss (NOLOAD) : {
    _bss_start = .;
    *(.dynbss)
    *(.bss .bss.* .gnu.linkonce.b.*)
    *(.dynlbss)
    *(.lbss .lbss.* .gnu.linkonce.lb.*)
    *(COMMON)
    *(LARGE_COMMON)
    . = ALIGN(8);
    _bss_end = .;
  } > MEM :load

  PROVIDE( _bss_size = _bss_end - _bss_start );
  PROVIDE (_brk_start = .);

  /* Reserve the brk heap *inside* this segment. The segment's memsz must cover
   * it so the kernel maps the whole range zero-fill and sbrk() hands out mapped
   * memory; otherwise the first malloc writes past the mapped region and faults.
   * (The two-segment script got this for free because its RAM segment spanned
   * the entire region.) memsz costs nothing in the file -- only filesz does. */
  .heap (NOLOAD) : {
    . = ORIGIN(MEM) + LENGTH(MEM);
  } > MEM :load
  PROVIDE (_brk_end = .);

  .shstrtab : { *(.shstrtab) }
  .strtab 0 : { *(.strtab) }
  .symtab 0 : { *(.symtab) }

  /* Non-allocated sections, parked at address 0 (kept for objdump/gdb on the
   * unstripped binary; removed from $@.clean). Listed explicitly so they are
   * not flagged by --orphan-handling=warn. */
  .stab          0 : { *(.stab) }
  .stabstr       0 : { *(.stabstr) }
  .debug_aranges 0 : { *(.debug_aranges) }
  .debug_info    0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev  0 : { *(.debug_abbrev) }
  .debug_line    0 : { *(.debug_line .debug_line.* .debug_line_end) }
  .debug_frame   0 : { *(.debug_frame) }
  .debug_str     0 : { *(.debug_str) }
  .debug_loc     0 : { *(.debug_loc) }
  .debug_ranges  0 : { *(.debug_ranges) }
  .debug_line_str 0 : { *(.debug_line_str) }
  .debug_loclists 0 : { *(.debug_loclists) }
  .debug_rnglists 0 : { *(.debug_rnglists) }
  .debug_str_offsets 0 : { *(.debug_str_offsets) }

  /DISCARD/ : {
     *(.note.GNU-stack)
     *(.note .note.*)
     *(.gnu_debuglink)
     *(.gnu.lto_*)
     *(.gnu.version)
     *(.gnu.version_d)
     *(.gnu.version_r)
     *(.eh_frame .eh_frame.*)
     *(.ARM.extab* .gnu.linkonce.armextab.*)
     *(.ARM.exidx*)
     *(.gnu.attributes)
     *(.gnu.build.attributes .gnu.build.attributes.*)
     *(.hash)
     *(.gnu.hash)
     *(.dynsym)
     *(.dynstr)
     *(.gnu.warning)
     *(.comment)
     *(.rela.* .iplt .igot.*)
     *(.got .got.*)
     *(.data.rel.ro.local* .gnu.linkonce.d.rel.ro.local.*)
     *(.data.rel.ro .data.rel.ro.* .gnu.linkonce.d.rel.ro.*)
  }
}
