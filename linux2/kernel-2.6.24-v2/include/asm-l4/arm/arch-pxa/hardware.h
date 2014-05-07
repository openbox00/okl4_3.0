/*
 *  linux/include/asm-l4/arch/hardware.h
 *
 *  Author:	Nicolas Pitre
 *  Created:	Jun 15, 2001
 *  Copyright:	MontaVista Software Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

/*
 * We requires absolute addresses.
 */
#define PCIO_BASE		0


#ifndef __ASSEMBLY__

#ifdef CONFIG_PXA25x
#define __cpu_is_pxa21x(id)				\
	({						\
		/*unsigned int _id = (id) >> 4 & 0xf3f;*/	\
		/*_id == 0x212;*/				\
		0 == 1; /* - hack for now -cvs */	\
	})

#define __cpu_is_pxa25x(id)				\
	({						\
		/*unsigned int _id = (id) >> 4 & 0xfff;*/	\
		/*_id == 0x2d0 || _id == 0x290;*/		\
		0 == 0; /* - hack for now -cvs */	\
	})
#else
#define __cpu_is_pxa21x(id)	(0)
#define __cpu_is_pxa25x(id)	(0)
#endif

#ifdef CONFIG_PXA27x
#define __cpu_is_pxa27x(id)				\
	({						\
		unsigned int _id = (id) >> 4 & 0xfff;	\
		_id == 0x411;				\
	})
#else
#define __cpu_is_pxa27x(id)	(0)
#endif

#ifdef CONFIG_CPU_PXA300
#define __cpu_is_pxa300(id)				\
	({						\
		unsigned int _id = (id) >> 4 & 0xfff;	\
		_id == 0x688;				\
	 })
#else
#define __cpu_is_pxa300(id)	(0)
#endif

#ifdef CONFIG_CPU_PXA310
#define __cpu_is_pxa310(id)				\
	({						\
		unsigned int _id = (id) >> 4 & 0xfff;	\
		_id == 0x689;				\
	 })
#else
#define __cpu_is_pxa310(id)	(0)
#endif

#ifdef CONFIG_CPU_PXA320
#define __cpu_is_pxa320(id)				\
	({						\
		unsigned int _id = (id) >> 4 & 0xfff;	\
		_id == 0x603 || _id == 0x682;		\
	 })
#else
#define __cpu_is_pxa320(id)	(0)
#endif

#define cpu_is_pxa21x()					\
	({						\
		__cpu_is_pxa21x(read_cpuid_id());	\
	})

#define cpu_is_pxa25x()					\
	({						\
		__cpu_is_pxa25x(read_cpuid_id());	\
	})

#define cpu_is_pxa27x()					\
	({						\
		__cpu_is_pxa27x(read_cpuid_id());	\
	})

#define cpu_is_pxa300()					\
	({						\
		__cpu_is_pxa300(read_cpuid_id());	\
	 })

#define cpu_is_pxa310()					\
	({						\
		__cpu_is_pxa310(read_cpuid_id());	\
	 })

#define cpu_is_pxa320()					\
	({						\
		__cpu_is_pxa320(read_cpuid_id());	\
	 })

/*
 * CPUID Core Generation Bit
 * <= 0x2 for pxa21x/pxa25x/pxa26x/pxa27x
 * == 0x3 for pxa300/pxa310/pxa320
 */
#define __cpu_is_pxa2xx(id)				\
	({						\
		unsigned int _id = (id) >> 13 & 0x7;	\
		_id <= 0x2;				\
	 })

#define __cpu_is_pxa3xx(id)				\
	({						\
		unsigned int _id = (id) >> 13 & 0x7;	\
		_id == 0x3;				\
	 })

#define cpu_is_pxa2xx()					\
	({						\
		__cpu_is_pxa2xx(read_cpuid_id());	\
	 })

#define cpu_is_pxa3xx()					\
	({						\
		__cpu_is_pxa3xx(read_cpuid_id());	\
	 })

/*
 * Handy routine to set GPIO alternate functions
 */
extern int pxa_gpio_mode( int gpio_mode );

/*
 * Return GPIO level, nonzero means high, zero is low
 */
extern int pxa_gpio_get_value(unsigned gpio);

/*
 * Set output GPIO level
 */
extern void pxa_gpio_set_value(unsigned gpio, int value);

#endif

#ifndef __ASSEMBLY__

# define __PREG(x)	(PHYS_##x)

// XXX OKL4 physaddr - where should this go?
extern unsigned long PHYS_FFUART;
extern unsigned long PHYS_BTUART;
extern unsigned long PHYS_STUART;
extern unsigned long PHYS_HWUART;

#else

#endif

#endif  /* _ASM_ARCH_HARDWARE_H */
