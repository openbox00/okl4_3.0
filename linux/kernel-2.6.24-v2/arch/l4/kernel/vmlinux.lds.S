#include <asm-generic/vmlinux.lds.h>


OUTPUT_ARCH(ELF_ARCH)
ENTRY(_start)
#if !defined(CONFIG_BIG_ENDIAN) || defined(CONFIG_ARCH_MIPS64)
jiffies = jiffies_64;
#elif defined(CONFIG_BIG_ENDIAN)
jiffies = jiffies_64 + 4;
#endif
PHDRS
{
  text PT_LOAD FLAGS(5);  /* R_E */
  data PT_LOAD FLAGS(7);  /* RWE */
  note PT_NOTE FLAGS(4);  /* ___ */
}
SECTIONS
{
  . = 0x1000			;
#ifdef CONFIG_ARCH_MIPS64
#define PAGE_SIZE	4096
#endif
#ifdef CONFIG_ARCH_ARM
#define PAGE_SIZE	4096
#endif
#ifdef CONFIG_ARCH_I386
#define PAGE_SIZE	4096
#endif
  /* read-only */
  .text : {
    _text = .;			/* Text and read-only data */
    *(.text)
    SCHED_TEXT
    LOCK_TEXT
    *(.fixup)
    *(.gnu.warning)
    *(.note*)
    . = ALIGN(PAGE_SIZE);
    __user_exregs_page = .;
    *(.exregspage)
    . = ALIGN(PAGE_SIZE);
  }: text =0

  _etext = .;

  . = ALIGN(16);		/* Exception table */
  __start___ex_table = .;
  __ex_table : { *(__ex_table) }
  __stop___ex_table = .;

  RODATA

  . = ALIGN(PAGE_SIZE); 
	
  .data : {			/* Data */
   /* Align the initial ramdisk image (INITRD) on page boundaries. */
/*
   __rd_start = .;
   *(.initrd)
   . = ALIGN(PAGE_SIZE);
   __rd_end = .;
*/

    *(.data)

    CONSTRUCTORS
   . = ALIGN(PAGE_SIZE);
  } :data

  . = ALIGN(32);
  .data.cacheline_aligned : { *(.data.cacheline_aligned) }

  .unprotected : { *(.unprotected) }
  . = ALIGN(PAGE_SIZE);
  PROVIDE (_unprotected_end = .);
  __start___param = .;
  __param : { *(__param) }
  __stop___param = .;

  . = ALIGN(32);
  __per_cpu_start = . ; 
  .data.percpu : { *(.data.percpu) }
  __per_cpu_end = . ;

  _edata = .;			/* End of data section */

  /* will be freed after init */
  . = ALIGN(PAGE_SIZE);		/* Init code and data */
  __init_begin = .;
  /* /DISCARD/ doesn't work for .reginfo */
  .reginfo : { *(.reginfo) }
  .init.text : {
	_sinittext = .;
	*(.init.text)
	_einittext = .;
  }
  .init.data : { *(.init.data) }
  . = ALIGN(16);
  __setup_start = .;
  .init.setup : { *(.init.setup) }
  __setup_end = .;

  . = ALIGN(16);
  __initcall_start = .;
  .initcall.init : {
        INITCALLS
  }
  __initcall_end = .;
  __con_initcall_start = .;
  .con_initcall.init : { *(.con_initcall.init) }
  __con_initcall_end = .;
  SECURITY_INIT
  . = ALIGN(4);
  /*
   * The alt instructions and para instructions is only used by the i386
   * but it should be OK to leave it generically here.
   */
  .altinstructions : AT(ADDR(.altinstructions) - LOAD_OFFSET) {
        __alt_instructions = .;
        *(.altinstructions)
        __alt_instructions_end = .;
  }
  .altinstr_replacement : AT(ADDR(.altinstr_replacement) - LOAD_OFFSET) {
        *(.altinstr_replacement)
  }
  . = ALIGN(4);
  .parainstructions : AT(ADDR(.parainstructions) - LOAD_OFFSET) {
        __parainstructions = .;
        *(.parainstructions)
        __parainstructions_end = .;
  }
  . = ALIGN(PAGE_SIZE);
  __initramfs_start = .;
  .init.ramfs : { *(.init.ramfs) }
  __initramfs_end = .;
  . = ALIGN(PAGE_SIZE);
  __init_end = .;
  /* freed after init ends here */

  __exitcall_begin = .;
  .exitcall : { *(.exitcall.exit) }
  __exitcall_end = .;

  .bss : {
	__bss_start = .;	/* BSS				*/
	*(.bss)
	*(COMMON)
	_end = .;
  }
}
