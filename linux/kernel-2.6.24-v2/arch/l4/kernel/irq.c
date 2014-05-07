/*
 *	linux/arch/l4/kernel/irq.c
 *
 *	Copyright (C) 1995 Linus Torvalds
 *
 * This file contains the code used by various IRQ handling routines:
 * asking for different IRQ's should be done through these routines
 * instead of just grabbing them. Thus setups with different IRQ numbers
 * shouldn't result in any weird surprises, and installing new handlers
 * should be easier.
 */


#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/kernel_stat.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/ptrace.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>

#if defined(CONFIG_IGUANA)
#include <iguana/hardware.h>
#endif

extern L4_ThreadId_t timer_thread;

volatile unsigned long irq_err_count;

void
ack_bad_irq(unsigned int irq)
{
	irq_err_count++;
	printk(KERN_CRIT "Unexpected IRQ trap at vector %u\n", irq);
}

int
show_interrupts(struct seq_file *p, void *v)
{
#ifdef CONFIG_SMP
	int j;
#endif
	int i = *(loff_t *) v;
	struct irqaction * action;
	unsigned long flags;

#ifdef CONFIG_SMP
	if (i == 0) {
		seq_puts(p, "           ");
		for (i = 0; i < NR_CPUS; i++)
			if (cpu_online(i))
				seq_printf(p, "CPU%d       ", i);
		seq_putc(p, '\n');
	}
#endif

	if (i < NR_IRQS) {
		spin_lock_irqsave(&irq_desc[i].lock, flags);
		action = irq_desc[i].action;
		if (!action) 
			goto unlock;
		seq_printf(p, "%3d: ",i);
#ifndef CONFIG_SMP
		seq_printf(p, "%10u ", kstat_irqs(i));
#else
		for (j = 0; j < NR_CPUS; j++)
			if (cpu_online(j))
				seq_printf(p, "%10u ", kstat_cpu(j).irqs[i]);
#endif
		seq_printf(p, " %14s", irq_desc[i].chip->name);
		seq_printf(p, "  %s",
				action->name);

		for (action=action->next; action; action = action->next) {
			seq_printf(p, ", %s",
				   action->name);
		}

		seq_putc(p, '\n');
unlock:
		spin_unlock_irqrestore(&irq_desc[i].lock, flags);
	} else if (i == NR_IRQS) {
#ifdef CONFIG_SMP
		seq_puts(p, "IPI: ");
		for (i = 0; i < NR_CPUS; i++)
			if (cpu_online(i))
				seq_printf(p, "%10lu ", cpu_data[i].ipi_count);
		seq_putc(p, '\n');
#endif
		seq_printf(p, "ERR: %10lu\n", irq_err_count);
	}
	return 0;
}


/*
 * handle_irq handles all normal device IRQ's (the special
 * SMP cross-CPU interrupts have their own specific
 * handlers).
 */

#define MAX_ILLEGAL_IRQS 16

void
handle_irq(int irq, struct pt_regs * regs)
{	
	/* 
	 * We ack quickly, we don't want the irq controller
	 * thinking we're snobs just because some other CPU has
	 * disabled global interrupts (we have already done the
	 * INT_ACK cycles, it's too late to try to pretend to the
	 * controller that we aren't taking the interrupt).
	 *
	 * 0 return value means that this irq is already being
	 * handled by some other CPU. (or is disabled)
	 */
	static unsigned int illegal_count = 0;

	
	if ((unsigned) irq > NR_IRQS && illegal_count < MAX_ILLEGAL_IRQS ) {
		irq_err_count++;
		illegal_count++;
		printk(KERN_CRIT "device_interrupt: invalid interrupt %d\n",
		       irq);
		return;
	}

	irq_enter();
	__do_IRQ(irq);
	irq_exit();
}

#ifdef CONFIG_SMP
void synchronize_irq(unsigned int irq)
{
        /* is there anything to synchronize with? */
	if (!irq_desc[irq].action)
		return;

	while (irq_desc[irq].status & IRQ_INPROGRESS)
		barrier();
}
#endif
