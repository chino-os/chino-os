/* Same as elf_x86_64_fbsd_efi.lds, except for OUTPUT_FORMAT below - KEEP IN SYNC */
OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64", "elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)
SECTIONS
{
  . = 0;
  /* .hash and/or .gnu.hash MUST come first! */
  .hash : { *(.hash) }
  .gnu.hash : { *(.gnu.hash) }
  .eh_frame : 
  { 
    *(.eh_frame)
  }

  . = ALIGN(4096);
  /* Program code segment, also known as a text segment */
  .text :
  {
    PROVIDE( _text = ABSOLUTE(.) );
    /* Initialization code segment */
    KEEP( *(.text.start) )
    *(.text.unlikely .text.unlikely.*)
    *(.text.startup .text.startup.*)
    /* Normal code segment */
    *(.text .text.*)
    *(.gnu.linkonce.t.*)

    . = ALIGN(8);
    PROVIDE( _etext = ABSOLUTE(.) );
   }
  .note.gnu.build-id : { *(.note.gnu.build-id) }
  
  /* Read-only data segment */
  .rodata :
  {
    *(.rdata)
    *(.rodata .rodata.*)
    *(.gnu.linkonce.r.*)
  }
  
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
    KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
    PROVIDE_HIDDEN (__init_array_end = .);
  }

  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
    KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
    PROVIDE_HIDDEN (__fini_array_end = .);
  }

  .ctors :
  {
    /* gcc uses crtbegin.o to find the start of
       the constructors, so we make sure it is
       first.  Because this is a wildcard, it
       doesn't matter if the user does not
       actually link against crtbegin.o; the
       linker won't look for a file to match a
       wildcard.  The wildcard also means that it
       doesn't matter which directory crtbegin.o
       is in.  */
    KEEP (*crtbegin.o(.ctors))
    KEEP (*crtbegin?.o(.ctors))
    /* We don't want to include the .ctor section from
       the crtend.o file until after the sorted ctors.
       The .ctor section from the crtend file contains the
       end of ctors marker and it must be last */
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
  }

  .dtors :
  {
    KEEP (*crtbegin.o(.dtors))
    KEEP (*crtbegin?.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
  }
  .reloc :
  {
   *(.reloc)
  }
  .interp :
  {
   *(.interp)
  }
  .gcc_except_table :
  {
   *(.gcc_except_table)
  }

  /* .data, .sdata and .srodata segment */
  .data :
  {
    /* Writable data segment (.data segment) */
    *(.data .data.*)
    *(.gnu.linkonce.d.*)
    /* Have _gp point to middle of sdata/sbss to maximize displacement range */
    . = ALIGN(8);
    PROVIDE( _gp = ABSOLUTE(.) + 0x800);
    /* Writable small data segment (.sdata segment) */
    *(.sdata .sdata.*)
    *(.gnu.linkonce.s.*)
    /* Read-only small data segment (.srodata segment) */
    . = ALIGN(8);
    *(.srodata.cst16)
    *(.srodata.cst8)
    *(.srodata.cst4)
    *(.srodata.cst2)
    *(.srodata .srodata.*)
    /* Align _edata to cache line size */
    . = ALIGN(64);
    PROVIDE( _edata = ABSOLUTE(.) );
  }

  /* .bss and .sbss segment */
  .bss :
  {
    PROVIDE( _bss = ABSOLUTE(.) );
    /* Writable uninitialized small data segment (.sbss segment)*/
    *(.sbss .sbss.*)
    *(.gnu.linkonce.sb.*)
    *(.scommon)
    /* Uninitialized writeable data section (.bss segment)*/
    *(.bss .bss.*)
    *(.gnu.linkonce.b.*)
    *(COMMON)

    . = ALIGN(8);
    PROVIDE( _ebss = ABSOLUTE(.) );
  }
  
  .dynamic  : { *(.dynamic) }
  . = ALIGN(4096);
  .rela :
  {
    *(.rela.data*)
    *(.rela.got)
    *(.rela.stab)
    *(.rela.dyn)
  }
  . = ALIGN(4096);
  .dynsym   : { *(.dynsym) }
  . = ALIGN(4096);
  .dynstr   : { *(.dynstr) }
  . = ALIGN(4096);
  .ignored.reloc :
  {
    *(.rela.reloc)
    *(.eh_frame)
    *(.note.GNU-stack)
  }

  .got : { *(.got.plt) *(.igot.plt) *(.got) *(.igot) }

  /* Stabs debugging sections.  */
  .stab          0 : { *(.stab) }
  .stabstr       0 : { *(.stabstr) }
  .stab.excl     0 : { *(.stab.excl) }
  .stab.exclstr  0 : { *(.stab.exclstr) }
  .stab.index    0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment 0 : { *(.comment) }
}
