/*
 * The MEMORY command describes the location and size of blocks of memory
 * in the target. You can use it to describe which memory regions may be
 * used by the linker, and which memory regions it must avoid.
 */
MEMORY
{
  /*
   * Boot ROM under AXI. All readonly data save to here.
   */
  rom (rxai!w) : ORIGIN = 0x08000000, LENGTH = (256 * 1024)
  /*
   * Memory with CPU cache.
   */
  ram (wxa!ri) : ORIGIN = 0x20000000, LENGTH = (48 * 1024)
}

PROVIDE( _rom_start  = ORIGIN(rom) );
PROVIDE( _rom_end    = ORIGIN(rom) + LENGTH(rom) );
PROVIDE( _ram_start  = ORIGIN(ram) );
PROVIDE( _ram_end    = ORIGIN(ram) + LENGTH(ram) );
PROVIDE( _stack_size = (1 * 1024) );

/*
 * The OUTPUT_ARCH command specifies the machine architecture where the
 * argument is one of the names used in the MAIX library.
 */
OUTPUT_ARCH( "arm" )

/*
 * The linker only pays attention to the PHDRS command when generating
 * an ELF output file. In other cases, the linker will simply ignore PHDRS.
 */
PHDRS
{
  rom      PT_LOAD;
  ram_init PT_LOAD;
  ram      PT_NULL;
}

/*
 * This is where we specify how the input sections map to output
 * sections.
 */
SECTIONS
{
  .ARM.vectortable :
  {
    *(.ARM.vectortable)
  } >rom AT>rom :rom
  
  . = ALIGN(4);

  /* Program code segment, also known as a text segment */
  .text :
  {
    PROVIDE( _text = ABSOLUTE(.) );
    /* Initialization code segment */
    KEEP( *(.init.header) )
    KEEP( *(.init.configstring) )
    KEEP( *(.init.static_data) )
    *(.text.unlikely .text.unlikely.*)
    *(.text.startup .text.startup.*)
    /* Normal code segment */
    *(.text .text.*)
    *(.gnu.linkonce.t.*)
    *(.ARM.extab.text.*)
    . = ALIGN(4);
    PROVIDE( _etext = ABSOLUTE(.) );

  } >rom AT>rom :rom

  __exidx_start = .;
  __exidx_end = .;
       
  .ARM.exidx : {
      __exidx_start = .;
      *(.ARM.exidx* .gnu.linkonce.armexidx.*)
      __exidx_end = .;
  } >rom AT>rom :rom

  /* Read-only data segment */
  .rodata :
  {
    *(.rdata)
    *(.rodata .rodata.*)
    *(.gnu.linkonce.r.*)
  } >rom AT>rom :rom

  . = ALIGN(4);

  .lalign :
  {
    . = ALIGN(8);
    PROVIDE( _data_lma = . );
  } >rom AT>rom :rom

  .dalign :
  {
    . = ALIGN(4);
    PROVIDE( _data = . );
  } >ram AT>rom :ram_init

  . = ALIGN(4);

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
  } >ram AT>rom :ram_init

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

    . = ALIGN(4);
    PROVIDE( _ebss = ABSOLUTE(.) );
  } >ram AT>ram :ram

  PROVIDE( _tls_data = ABSOLUTE(.) );
  /*
   * Thread Local Storage (TLS) are per-thread global variables.
   * Compilers such as GCC provide a __thread keyword to mark global
   * variables as per-thread. Support is required in the program loader
   * and thread creator.
   */

  /* Thread-local data segment, .tdata (initialized tls). */
  .tdata :
  {
    KEEP( *(.tdata.begin) )
    *(.tdata .tdata.*)
    *(.gnu.linkonce.td.*)
    KEEP( *(.tdata.end) )
  } >ram AT>ram :ram

  /* Thread-local bss segment, .tbss (zero-initialized tls). */
  .tbss :
  {
    *(.tbss .tbss.*)
    *(.gnu.linkonce.tb.*)
    KEEP( *(.tbss.end) )
  } >ram AT>ram :ram

  /*
   * End of uninitalized data segement
   *
   * Actually the stack needs 16B alignment, and it won't hurt to also slightly
   * increase the alignment to 32 or even 64 (cache line size).
   *
   * Align _heap_start to cache line size
   */
  . = ALIGN(64);
  PROVIDE( _end = ABSOLUTE(.) );
  PROVIDE( _stack_top = ABSOLUTE(.) + _stack_size );
  PROVIDE( _heap_start = _stack_top );
  /* Heap end is at the end of memory, the memory size can set in kconfig */
  PROVIDE( _heap_size = _ram_end - _heap_start );
}
