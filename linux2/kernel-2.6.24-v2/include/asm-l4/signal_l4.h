#ifndef _ASM_L4_SIGNAL_L4_H_
#define _ASM_L4_SIGNAL_L4_H_


extern int l4_do_signal(sigset_t *oldset, struct pt_regs *regs, int syscall);

static inline int test_my_flag(struct thread_info * my_info, int flag)
{
	return test_bit(flag, &my_info->flags);
}

static inline int need_syscall_trace(struct thread_info * my_info)
{
	return unlikely(test_my_flag(my_info, TIF_SYSCALL_TRACE));
}

static inline int need_to_resched(struct thread_info * my_info)
{
	return unlikely(test_my_flag(my_info, TIF_NEED_RESCHED));
}


extern void do_syscall_trace(int, void *);

extern inline void
syscall_entry(struct thread_info * my_info)
{
	if (unlikely(need_syscall_trace(my_info)))
	{
//printk("%s (%d): syscall entry trace\n", current->comm, current->pid);
#ifdef CONFIG_ARCH_ARM
		do_syscall_trace(0, &my_info->regs);
#endif
	}
}

extern inline void
syscall_exit(struct thread_info * my_info, int syscall)
{
	if (unlikely( syscall && need_syscall_trace(my_info) ))
	{
//printk("%s (%d): syscall exit trace\n", current->comm, current->pid);
#ifdef CONFIG_ARCH_ARM
		do_syscall_trace(1, &my_info->regs);
#endif
	}
}

extern inline int
l4_work_pending(struct thread_info * my_info, int syscall, struct pt_regs *regs)
{
	if (unlikely( need_to_resched(my_info) )) {
		schedule();
	}

	if (unlikely( signal_pending(my_info->task) )) {
		return l4_do_signal(NULL, regs, syscall);
	}
	return 0;
}

extern inline int
l4_work_pending_preempt(struct pt_regs *regs)
{
	int restart = 0;
	if (need_resched()) {
		/* Prevent current from running */
		//int r = L4_Set_Priority(current_thread_info()->user_tid, 0);
		//assert(r != 0);
		L4_Stop_Thread (current_thread_info()->user_tid);
		restart = 1;

		schedule();

		//r = L4_Set_Priority(current_thread_info()->user_tid, 98);
		//assert(r != 0);
	}

	if (signal_pending(current)) {
		return l4_do_signal(NULL, regs, 0);
	}
	if (restart) {
		L4_Start(current_thread_info()->user_tid);
	}
	return 0;
}

#endif /* _ASM_L4_SIGNAL_L4_H_ */
