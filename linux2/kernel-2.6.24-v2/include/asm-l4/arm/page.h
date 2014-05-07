#ifndef _L4_ARM_PAGE_H
#define _L4_ARM_PAGE_H

#define PAGE_SHIFT	12
#define TASK_SIZE_26	(0x04000000UL)

/*
 * With EABI on ARMv5 and above we must have 64-bit aligned slab pointers.
 */
#if defined(CONFIG_AEABI) && (__LINUX_ARM_ARCH__ >= 5)
#define ARCH_SLAB_MINALIGN 8
#endif

#endif /* _L4_ARM_PAGE_H */
