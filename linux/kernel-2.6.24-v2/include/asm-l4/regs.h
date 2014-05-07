#ifndef _L4_REGS_H_
#define _L4_REGS_H_

/* this struct defines the way the registers are stored on the
   stack during a system call. */
struct pt_regs {
	L4_Msg_t msg;
#ifdef CONFIG_ARCH_ARM
//	unsigned long r8;
//	unsigned long r9;
//	unsigned long r10;
	unsigned long ip;
//	unsigned long fp;
#endif
	int strace_flag;

	/* mode: 0 - in user, 1 - in kernel, 2 - isr */
	unsigned int mode;
};

#endif /* _L4_REGS_H_ */
