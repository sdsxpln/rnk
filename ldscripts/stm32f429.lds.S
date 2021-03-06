#include <rnk.lds.h>

MEMORY
{
    rom (rx)  : ORIGIN = 0x08000000, LENGTH = 0x00100000
    ram (rwx) : ORIGIN = 0x20000000, LENGTH = 0x0002FFFF
    ccm (rwx) : ORIGIN = 0x10000000, LENGTH = 0x00010000
}
 
_eram = 0x20000000 + 0x0002FFFF;

/* Section Definitions */
SECTIONS
{
    /* collect all uninitialized .ccm sections */
	/* A directed static allocation would look like this :
		volatile uint8_t buff0[16384] __attribute__((section(".ccm")));   */
	.ccm (NOLOAD) :
    {
        . = ALIGN(4);
        *(.ccm)
        *(.ccm.*)
        . = ALIGN(4);
    } > ccm
 
    .text :
    {
        _text = .;
        KEEP(*(.isr_vector .isr_vector.*))
        KEEP(*(.cm_isr_vector))
	KEEP(*(.isr_vector_cmsis))
        *(.text .text.* .gnu.linkonce.t.*)
        *(.glue_7t) *(.glue_7)
        *(.rodata .rodata* .gnu.linkonce.r.*)

	. = ALIGN(4);
	_etext = .;
	PROVIDE(etext = .);
    } > rom
 
#ifdef CONFIG_UNWIND
    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > rom
 
    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > rom
    __exidx_end = .;
#endif

   __rnk_initcalls_start = .;
   .rnk_initcalls : {
	INITCALLS
   } > rom
   __rnk_initcalls_end = .;


   __rnk_exitcalls_start = .;
   .rnk_exitcalls : {
	EXITCALLS
   } > rom
   __rnk_exitcalls_end = .;

   __rnk_ksym_start = .;
   .rnk_ksym : {
	KSYM
   } > rom
   __rnk_ksym_end = .;
 
    _sidata = .;
    .data : AT (_sidata)
    {
        _sdata = .;
        *(.data .data.*)
        . = ALIGN(4);
        _edata = . ;
    } > ram
 
    /* .bss section which is used for uninitialized data */
    .bss (NOLOAD) :
    {
        _sbss = . ;
	__bss_start__ = . ;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = . ;
	__bss_end__ = . ;
    } > ram
 
    /* stack section */
    .co_stack (NOLOAD):
    {
        . = ALIGN(8);
        *(.co_stack .co_stack.*)
    } > ram
 
    . = ALIGN(4);
    _end = . ;
    PROVIDE( _estack = _end + CONFIG_STACK_SIZE );
}
