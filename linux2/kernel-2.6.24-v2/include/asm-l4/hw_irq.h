#ifndef _ASM_UM_HW_IRQ_H
#define _ASM_UM_HW_IRQ_H

#include "asm/irq.h"

static inline void hw_resend_irq(struct hw_interrupt_type *h, unsigned int i)
{}

#define ACTUAL_NR_IRQS	NR_IRQS

#endif
