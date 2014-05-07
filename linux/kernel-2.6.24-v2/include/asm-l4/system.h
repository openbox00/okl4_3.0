#ifndef _L4_SYSTEM_H
#define _L4_SYSTEM_H

#include <asm/macros.h>
#include <asm/percpu.h>
#include <linux/smp.h>
#include <linux/linkage.h>
#include <linux/irqflags.h>

struct task_struct;
extern asmlinkage void *__switch_to(struct task_struct *prev, struct task_struct *next);

#include <l4/kdebug.h>
#include <l4/thread.h>
/**
 *  libcompat also defines __FUNCTION__ so undef it to allow OKLinux to def it.
 */
#undef __FUNCTION__

typedef struct {
    volatile unsigned short irq_state;
    volatile unsigned short pending_irq;
} irq_info_t;

extern void l4_handle_pending(void);

extern DEFINE_PER_CPU(irq_info_t, _l4_irq_state);
#define IRQ_state(cpu) (per_cpu(_l4_irq_state, cpu).irq_state)
#define IRQ_pending(cpu) (per_cpu(_l4_irq_state, cpu).pending_irq)

#define switch_to(prev,next,last)		\
do {						\
	(last) = __switch_to(prev, next);	\
} while(0)


static inline void isr_irq_enable(void)
{
	IRQ_state(smp_processor_id()) = 0;
}

static inline void isr_irq_disable(void)
{
	IRQ_state(smp_processor_id()) = 1;
}

extern void local_irq_enable(void);

static inline void local_irq_disable(void)
{
	IRQ_state(smp_processor_id()) = 1;
}

#define local_save_flags(x)			\
do {						\
	x = IRQ_state(smp_processor_id());	\
} while (0)

#define local_irq_save(x)			\
do {						\
	x = IRQ_state(smp_processor_id());	\
	local_irq_disable();			\
} while (0)

#define local_irq_restore(flags)		\
do {						\
	if ( likely(flags == 0) )		\
		local_irq_enable();		\
	else					\
		IRQ_state(smp_processor_id()) = flags;	\
} while (0)

#define irqs_disabled()		IRQ_state(smp_processor_id())

/*
 * L4 Notify messages for L_timer and L_syscall communication
 */
#define L4_PREEMPT		0x1

#include INC_SYSTEM2(system.h)

#endif /* _L4_SYSTEM_H */
