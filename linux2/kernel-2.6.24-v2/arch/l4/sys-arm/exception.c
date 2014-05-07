/*********************************************************************
 *                
 * Copyright (C) 2006,  National ICT Australia
 *                
 ********************************************************************/

#include "l4.h"
#include "assert.h"


#include <linux/kernel.h>
#include <linux/sched.h>

#include <asm/syscalls.h>
#include <asm/signal.h>
#include <asm/signal_l4.h>

void l4_arch_handle_exception(struct pt_regs *regs)
{
	if ( (ARM_EXCEPT_number(regs) & 0xff0) == 0x100 )
	{
		siginfo_t info;
		switch (ARM_EXCEPT_number(regs) & 0x0f)
		{
		case 1:
		case 3:
			printk(KERN_INFO "alignment fault: ");
			break;
		default:
			printk(KERN_INFO "ARM mmu fault fs=%lx: ", ARM_EXCEPT_number(regs) & 0x0f);
			break;
		}
		printk(" @ %lx, faddr=%08lx in: %s (%d)\n",
				ARM_EXCEPT_pc(regs), ARM_EXCEPT_error_code(regs),
				current->comm, current->pid);

		info.si_signo = SIGSEGV;
		info.si_errno = 0;
		info.si_code = BUS_ADRERR;
		info.si_addr = (void *) ARM_EXCEPT_error_code(regs);
		force_sig_info(SIGSEGV, &info, current);

		return;

	} else {
		switch ( ARM_EXCEPT_number(regs) ) {
		case 1:
			printk(KERN_INFO "undefined instruction @ %lx in: %s (%d)\n",
					ARM_EXCEPT_pc(regs), current->comm, current->pid);
			break;
		case 8:
			printk(KERN_INFO "VFP exception @ %lx in: %s (%d)\n",
					ARM_EXCEPT_pc(regs), current->comm, current->pid);
			break;
		default:
			printk(KERN_INFO "unhandled user exception (0x%lx) @ %lx in %s (%d)\n",
					ARM_EXCEPT_number(regs), ARM_EXCEPT_pc(regs),
					current->comm, current->pid);
		}
	}
//L4_KDB_Enter("exc");
	send_sig(SIGILL, current, 1);
}
