#ifdef __i386__
/*
 * Only i386 has cmpxchg.
 */
#include INC_SYSTEM(cmpxchg.h)

/*
 * These are duplicated because they use the boot_cpu_data as flags
 * but we can't put a define within a define and want to leave the orig
 * code in.  :-(
 */

#undef cmpxchg
#undef cmpxchg_local

#define cmpxchg(ptr,o,n)						\
({									\
	__typeof__(*(ptr)) __ret;					\
	__ret = __cmpxchg((ptr), (unsigned long)(o),			\
			(unsigned long)(n), sizeof(*(ptr)));		\
	__ret;								\
})
#define cmpxchg_local(ptr,o,n)						\
({									\
	__typeof__(*(ptr)) __ret;					\
	__ret = __cmpxchg_local((ptr), (unsigned long)(o),		\
		(unsigned long)(n), sizeof(*(ptr)));			\
	__ret;								\
})

#endif
