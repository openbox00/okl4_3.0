#ifndef __L4_MODULE_H
#define __L4_MODULE_H

#include <asm/macros.h>

#ifdef CONFIG_ARCH_I386	/* Need to define cpu type for I386 */
#define CONFIG_M386
#endif

#include INC_SYSTEM(module.h)

#endif /* __L4_MODULE_H */
