#ifndef _L4_MIPS_EXCEPTION_H_
#define _L4_MIPS_EXCEPTION_H_

extern void do_ade(struct pt_regs *regs);

inline void l4_arch_handle_exception(struct pt_regs *regs)
{
	switch ( MIPS_EXCEPT_number(regs) ) {
	case 4:
	case 5:
		do_ade(regs);
		break;
	default:
		printk(KERN_INFO "unhandled user exception (%ld) in %s (%d)\n",
				MIPS_EXCEPT_number(regs), current->comm, current->pid);
		send_sig(SIGILL, current, 1);
	}
}

#endif /* _L4_MIPS_EXCEPTION_H_ */
