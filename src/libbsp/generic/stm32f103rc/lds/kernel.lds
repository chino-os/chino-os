/*
 * linker script for STM32F10x with GNU ld
 * bernard.xiong 2009-10-14
 */

/* Program Entry, set to mark it as "used" and avoid gc */
MEMORY
{
    CODE (rx) : ORIGIN = 0x08000000, LENGTH = 256k /* 256KB flash */
    DATA (rw) : ORIGIN = 0x20000000, LENGTH =  48k /* 48K sram */
}
ENTRY(Reset_Handler)
_system_stack_size = 0x400;
_ram_end = ORIGIN(DATA) + LENGTH(DATA);

SECTIONS
{
	.isr :
	{
        KEEP(*(.isr_vector))            /* Startup code */
	} > CODE = 0

	.init :
	{
		KEEP (*(SORT_NONE(.init)))
	} > CODE = 0

    .text :
    {
        . = ALIGN(4);
        _stext = .;
        . = ALIGN(4);
        *(.text)                        /* remaining code */
        *(.text.*)                      /* remaining code */
        *(.glue_7)
        *(.glue_7t)
        *(.gnu.linkonce.t*)

        . = ALIGN(4);
        _etext = .;
    } > CODE = 0

	.fini :
	{
	  KEEP (*(SORT_NONE(.fini)))
	} > CODE = 0
	
    /* Read-only data segment */
    .rodata :
    {
      *(.rdata)
      *(.rodata .rodata.*)
      *(.gnu.linkonce.r.*)
    } > CODE = 0

	.ARM.extab : /* exception unwinding information */
    {
        *(.ARM.extab*)
	} > CODE = 0

    /* .ARM.exidx is sorted, so has to go in its own output section.  */
    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > CODE = 0
    __exidx_end = .;

    /* .data section which is used for initialized data */
	   /*
     * Newlib and Eglibc (at least) need these for C++ support.
     *
     * (Copied from Sourcery CodeBench Lite: arm-none-eabi-gcc -V)
     */
    .preinit_array :
    {
        PROVIDE_HIDDEN(__preinit_array_start = .);
        KEEP(*(.preinit_array*))
        PROVIDE_HIDDEN(__preinit_array_end = .);
    } > CODE = 0
    .init_array :
    {
        PROVIDE_HIDDEN(__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array*))
        PROVIDE_HIDDEN(__init_array_end = .);
    } > CODE = 0
    .fini_array :
    {
        PROVIDE_HIDDEN(__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array*))
        PROVIDE_HIDDEN(__fini_array_end = .);
	} > CODE = 0
	
    /* This is used by the startup in order to initialize the .data secion */
    _sidata = .;
    .data : AT (_sidata)
    {
        . = ALIGN(4);
        /* This is used by the startup in order to initialize the .data secion */
        _sdata = . ;

        *(.data)
        *(.data.*)
        *(.gnu.linkonce.d*)
		
        *(.got)
        *(.got.plt)

        . = ALIGN(4);
        /* This is used by the startup in order to initialize the .data secion */
        _edata = . ;
    } >DATA

    __bss_start = .;
    .bss :
    {
        . = ALIGN(4);
        /* This is used by the startup in order to initialize the .bss secion */
        _sbss = .;

        *(.bss)
        *(.bss.*)
        *(COMMON)

        . = ALIGN(4);
        /* This is used by the startup in order to initialize the .bss secion */
        _ebss = . ;
        
        *(.bss.init)
    } > DATA
    __bss_end = .;

    .stack : 
    {
        . = . + _system_stack_size;
        . = ALIGN(4);
        _estack = .;
    } >DATA

    _end = .;

	PROVIDE( _heap_start = ABSOLUTE(.) );
	PROVIDE( _heap_end = _ram_end );

    /* Stabs debugging sections.  */
    .stab          0 : { *(.stab) }
    .stabstr       0 : { *(.stabstr) }
    .stab.excl     0 : { *(.stab.excl) }
    .stab.exclstr  0 : { *(.stab.exclstr) }
    .stab.index    0 : { *(.stab.index) }
    .stab.indexstr 0 : { *(.stab.indexstr) }
    .comment       0 : { *(.comment) }
    /* DWARF debug sections.
     * Symbols in the DWARF debugging sections are relative to the beginning
     * of the section so we begin them at 0.  */
    /* DWARF 1 */
    .debug          0 : { *(.debug) }
    .line           0 : { *(.line) }
    /* GNU DWARF 1 extensions */
    .debug_srcinfo  0 : { *(.debug_srcinfo) }
    .debug_sfnames  0 : { *(.debug_sfnames) }
    /* DWARF 1.1 and DWARF 2 */
    .debug_aranges  0 : { *(.debug_aranges) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    /* DWARF 2 */
    .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_line     0 : { *(.debug_line) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_str      0 : { *(.debug_str) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_macinfo  0 : { *(.debug_macinfo) }
    /* SGI/MIPS DWARF 2 extensions */
    .debug_weaknames 0 : { *(.debug_weaknames) }
    .debug_funcnames 0 : { *(.debug_funcnames) }
    .debug_typenames 0 : { *(.debug_typenames) }
    .debug_varnames  0 : { *(.debug_varnames) }
}
