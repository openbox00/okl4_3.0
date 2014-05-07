#ifndef _L4_MACROS_H_
#define _L4_MACROS_H_



#ifdef CONFIG_MIPS64
#define _MIPS_SIM _MIPS_SIM_ABI64
#define _MIPS_SZLONG _L4_WORD
#endif

#define INC_SYSTEM(file) <asm-__SYSINC__/file>
#define INC_SYSTEM2(file) <asm/__SYSTEM__/file>
#define INC_MACHINE(file) <asm-__SYSINC__/__MACHINE_INC__/file>

#endif /* _L4_MACROS_H_ */
