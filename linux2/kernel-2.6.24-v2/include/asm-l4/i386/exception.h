#ifndef _L4_I386_EXCEPTION_H_
#define _L4_I386_EXCEPTION_H_

inline void l4_arch_handle_exception(struct pt_regs *regs)
{
	printk(KERN_INFO "unhandled user exception in %s (%d)\n",
			current->comm, current->pid);
	send_sig(SIGILL, current, 1);
}

#endif /* _L4_I386_EXCEPTION_H_ */
