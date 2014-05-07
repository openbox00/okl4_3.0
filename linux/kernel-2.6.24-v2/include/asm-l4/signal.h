#ifndef _L4_SIGNAL_H_
#define _L4_SIGNAL_H_

#include <asm/macros.h>
#include <linux/compiler.h>

#include INC_SYSTEM(signal.h)

struct exregs_regs {
	unsigned long ip;
	unsigned long sp;
	unsigned long flags;

	/* For syscall restarting/interrupting only */
	unsigned long syscall_action;
};

#undef ptrace_signal_deliver
#define ptrace_signal_deliver(regs, cookie) do {} while(0/*CONSTCOND*/)

#endif /* _L4_SIGNAL_H_ */
