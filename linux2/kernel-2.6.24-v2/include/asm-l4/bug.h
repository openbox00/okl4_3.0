#ifndef __UM_BUG_H
#define __UM_BUG_H

#ifndef __ASSEMBLY__

#define HAVE_ARCH_BUG
#define BUG() do { \
	panic("kernel BUG at %s:%d!\n", __FILE__, __LINE__); \
} while (0)

#define HAVE_ARCH_BUG_ON
#define BUG_ON(condition) do { \
	if (unlikely((condition)!=0)) \
		BUG(); \
} while(0)

#define HAVE_ARCH_PAGE_BUG
#define PAGE_BUG(page) do { \
	BUG(); \
} while (0)

extern int foo;

#include <asm-generic/bug.h>

#endif

#endif
