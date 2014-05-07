#ifndef __L4_IRQ_H
#define __L4_IRQ_H

/* The i386 irq.h has a struct task_struct in a prototype without including
 * sched.h.  This forward declaration kills the resulting warning.
 */
struct task_struct;

#include "asm/ptrace.h"

#if defined(CONFIG_IGUANA)
#undef NR_IRQS

#define NR_IGUANA_IRQS	(32)     // 32 to 64 = Iguana IRQ (notify bits)
#define HW_IRQ(x)	(x)
#define IGUANA_IRQ(x)	((x) + 32)
#define NR_IRQS		(64)
#endif

/*
 * Use this value to indicate lack of interrupt
 * capability
 */
#ifndef NO_IRQ
#define NO_IRQ  ((unsigned int)(-1))
#endif

#if defined(CONFIG_CELL)
#include INC_SYSTEM2(irq.h)
#endif

#define IGUANA_IRQ_NOTIFY_MASK(x)	(1UL << ((x) - 32))

#ifndef irq_canonicalize
#define irq_canonicalize(irq)	(irq)
#endif

void ack_bad_irq(unsigned int irq);

#endif
