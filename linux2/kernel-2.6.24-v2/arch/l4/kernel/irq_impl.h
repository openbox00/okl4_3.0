/*
 *	linux/arch/l4/kernel/irq_impl.h
 *
 *	Copyright (C) 1995 Linus Torvalds
 *	Copyright (C) 1998, 2000 Richard Henderson
 *
 * This file contains declarations and inline functions for interfacing
 * with the IRQ handling routines in irq.c.
 */

#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/profile.h>

extern void handle_irq(int irq, struct pt_regs * regs);

extern unsigned long prof_cpu_mask;

