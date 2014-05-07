/* signal.c
 * (C) 2004, National ICT Australia
 */

#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/freezer.h>
#include <asm/signal.h>
#include <asm/syscalls.h>

int l4_do_signal(sigset_t *oldset, struct pt_regs *regs, int syscall);

void l4_arch_setup_frame(int signr, struct k_sigaction * ka, siginfo_t *info,
		sigset_t *set, struct exregs_regs *regs);

/*
 * OK, we're invoking a handler
 */

//			current_thread_info()->restart_block.fn =
//						do_no_restart_syscall;
#define SIG_DEBUG
#undef SIG_DEBUG

#ifdef SIG_DEBUG
#define printk_dbg  printk
#else
#define printk_dbg(...)
#endif

/*
 * Called from l4_do_signal below.
 */
static void
handle_signal(int sig, struct k_sigaction *ka,
			siginfo_t *info, sigset_t *oldset,
		        int syscall)
{
	struct exregs_regs regs;

	/*
	 * Depending on L4 thread state, get either saved state
	 * or get it from L4
	 */
	if (user_need_restart(current_thread_info()))
	{
		regs.sp = current_thread_info()->restart.user_sp;
		regs.ip = current_thread_info()->restart.user_ip;
		regs.flags = current_thread_info()->restart.user_flags;
	} else {
		L4_Stop_SpIpFlags(current_thread_info()->user_tid,
				&regs.sp, &regs.ip, &regs.flags);
	}

	regs.syscall_action = 0;

	/* Remember that Linux user thread's IPC was cancelled */
	set_user_ipc_cancelled(current_thread_info());

	printk_dbg("%s old ip=%lx, sp=%lx, flags =%lx\n", __func__,
			regs.ip, regs.sp, regs.flags);

	/* Did we come from a syscall? */
	if (syscall) {
		/* Restart the system call after the signal? */
		long ret = l4_arch_get_error(current_regs());

		switch (ret) {
		case ERESTART_RESTARTBLOCK:
		case ERESTARTNOHAND:
			regs.syscall_action= 4;	/* syscall will be interrupted */
			break;
		case ERESTARTSYS:
			if (!(ka->sa.sa_flags & SA_RESTART)) {
				regs.syscall_action = 4;	/* syscall will be interrupted */
				break;
			}
			 /* fallthrough */
		case ERESTARTNOINTR:
			printk_dbg("%s setup restart syscall after signal\n", __func__);
			regs.syscall_action = 2;	/* syscall will be restarted */
			break;
		default:
			/*
			 * If we are in the trampoline right after
			 * the sigreturn(), do nothing and keep going.
			 *
			 * The reason is in OKLinux the sigreturn()
			 * changes the instruction pointer to point
			 * to another bit of user-space trampoline 
			 * which restores the GPRs and the instruction
			 * pointer to restart the instruction.  Normally
			 * we want to skip over the instruction which
			 * did the system call, but in this case we don't
			 * because doing so would skip over one instruction
			 * in the trampoline or in the case of x86, 
			 * probably junk in the instruction stream.
			 *
			 * In fact any system call which changes the 
			 * instruction pointer like this should have
			 * a special case here, but sigreturn() seems to 
			 * be the only one.
			 *
			 * Otherwise, we can simply indicate that we came
			 * from a system call.
			 *
		 	 * -gl
			 */
			/* Indicate we are from a syscall */
			if (syscall == __L4_sys_rt_sigreturn ||
			    syscall == __L4_sys_sigreturn)
				regs.syscall_action = 0;
			else
				regs.syscall_action = 1;
		}
	}

	/* Create signal frame on user stack */
	l4_arch_setup_frame(sig, ka, info, oldset, &regs);

	if (!(ka->sa.sa_flags & SA_NODEFER)) {
		spin_lock_irq(&current->sighand->siglock);
		
		sigorsets(&current->blocked,&current->blocked,&ka->sa.sa_mask);
		sigaddset(&current->blocked,sig);
		recalc_sigpending();

		spin_unlock_irq(&current->sighand->siglock);
	}
}

/*
 * Called from sys(_rt)_sigsuspend and from syscall loop when checking
 * if any work is pending.
 */
int l4_do_signal(sigset_t *oldset, struct pt_regs *regs, int syscall)
{
	struct k_sigaction ka;
	siginfo_t info;
	int signr;

	/*
	 * We want the common case to go fast, which
	 * is why we may in certain cases get here from
	 * kernel mode. Just return without doing anything
	 * if so.
	 */
	if (!user_mode(regs))
		return 0;

	printk_dbg("l4_do_signal(%d)\n", syscall);

	if (try_to_freeze()) {
		goto no_signal;
	}

	if (!oldset)
		oldset = &current->blocked;

	signr = get_signal_to_deliver(&info, &ka, NULL, NULL);
	if (signr > 0) {
		handle_signal(signr, &ka, &info, oldset, syscall);
		return 1;
	}

no_signal:
	/* Did we come from a system call? */
	if (syscall) {
		/* Restart the system call? */
		long ret = l4_arch_get_error(current_regs());

		if (ret == ERESTARTNOHAND ||
		    ret == ERESTARTSYS ||
		    ret == ERESTARTNOINTR) {
			printk_dbg("%s setup restart syscall\n", __func__);
			l4_arch_setup_restart(current_regs());

		} else
		if (ret == ERESTART_RESTARTBLOCK) {
			printk_dbg("%s setup sys_restart syscall\n", __func__);
			l4_arch_setup_sys_restart(current_regs());
		}
	}
	return 0;
}
