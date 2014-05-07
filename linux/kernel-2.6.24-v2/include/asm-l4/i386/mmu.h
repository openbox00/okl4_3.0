/*
 * include/asm-i386/arm/mmu.h
 */

#ifndef __L4_I386_MMU_H_
#define __L4_I386_MMU_H_

/*
 * LDT information
 */
#ifndef CONFIG_IA32_VDSO_ENABLE          

# define	L4_ARCH_MMU_CONTEXT	\
	int size;			\
	struct semaphore sem;		\
	void *ldt;			\

#else /* ifdef CONFIG_IA32_VDSO_ENABLE */

# define	L4_ARCH_MMU_CONTEXT	\
	int size;			\
	struct semaphore sem;		\
	void *ldt;			\
        void *vdso;                     \

#endif /* ifndef CONFIG_IA32_VDSO_ENABLE */

#endif
