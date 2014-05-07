#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/personality.h>
#include <linux/freezer.h>
#include <asm/signal.h>
#include <asm/syscalls.h>
#include <asm/ucontext.h>
#include <asm/uaccess.h>
#include INC_SYSTEM(unistd.h)
#include INC_SYSTEM(vm86.h)
#include "sigframe.h"

#define SIG_DEBUG
#undef SIG_DEBUG

#ifdef SIG_DEBUG
#define printk_dbg  printk
#else
#define printk_dbg(...)
#endif

#define _BLOCKABLE (~(sigmask(SIGKILL) | sigmask(SIGSTOP)))

asmlinkage int l4_do_signal(sigset_t *oldset, struct pt_regs *regs, int syscall);

/*
 * Atomically swap in the new signal mask, and wait for a signal.
 */
asmlinkage int
sys_sigsuspend(int history0, int history1, old_sigset_t mask, struct pt_regs * regs)
{
	sigset_t saveset;
	printk_dbg("%s %d\n", __func__, __LINE__);

	mask &= _BLOCKABLE;
	spin_lock_irq(&current->sighand->siglock);
	saveset = current->blocked;
	siginitset(&current->blocked, mask);
	recalc_sigpending();
	spin_unlock_irq(&current->sighand->siglock);

	i386_put_eax(regs, -EINTR);
	while (1) {
		current->state = TASK_INTERRUPTIBLE;
		schedule();
		if (l4_do_signal(&saveset, regs, 1))
			return -EINTR;
	}
}

asmlinkage int
sys_rt_sigsuspend(struct pt_regs * regs)
{
	sigset_t saveset, newset;
	printk_dbg("%s %d\n", __func__, __LINE__);

	/* XXX: Don't preclude handling different sized sigset_t's.  */
	if (i386_ecx(regs) != sizeof(sigset_t))
		return -EINVAL;

	if (copy_from_user(&newset, (sigset_t __user *)i386_ebx(regs), sizeof(newset)))
		return -EFAULT;
	sigdelsetmask(&newset, ~_BLOCKABLE);

	spin_lock_irq(&current->sighand->siglock);
	saveset = current->blocked;
	current->blocked = newset;
	recalc_sigpending();
	spin_unlock_irq(&current->sighand->siglock);

	i386_put_eax(regs, -EINTR);
	while (1) {
		current->state = TASK_INTERRUPTIBLE;
		schedule();
		if (l4_do_signal(&saveset, regs, 1))
			return -EINTR;
	}
}

asmlinkage int 
sys_sigaction(int sig, const struct old_sigaction __user *act,
	      struct old_sigaction __user *oact)
{
	struct k_sigaction new_ka, old_ka;
	int ret;

	if (act) {
		old_sigset_t mask;
		if (verify_area(VERIFY_READ, act, sizeof(*act)) ||
		    __get_user(new_ka.sa.sa_handler, &act->sa_handler) ||
		    __get_user(new_ka.sa.sa_restorer, &act->sa_restorer))
			return -EFAULT;
		__get_user(new_ka.sa.sa_flags, &act->sa_flags);
		__get_user(mask, &act->sa_mask);
		siginitset(&new_ka.sa.sa_mask, mask);
	}

	ret = do_sigaction(sig, act ? &new_ka : NULL, oact ? &old_ka : NULL);

	if (!ret && oact) {
		if (verify_area(VERIFY_WRITE, oact, sizeof(*oact)) ||
		    __put_user(old_ka.sa.sa_handler, &oact->sa_handler) ||
		    __put_user(old_ka.sa.sa_restorer, &oact->sa_restorer))
			return -EFAULT;
		__put_user(old_ka.sa.sa_flags, &oact->sa_flags);
		__put_user(old_ka.sa.sa_mask.sig[0], &oact->sa_mask);
	}

	return ret;
}

asmlinkage int
sys_sigaltstack(struct pt_regs *regs)
{
	const stack_t __user *uss = (const stack_t __user *)i386_ebx(regs);
	stack_t __user *uoss = (stack_t __user *)i386_ecx(regs);
	return do_sigaltstack(uss, uoss, i386_esp(regs));
}

extern void __user __wombat_user_sigentry;
extern void __user __wombat_user_sigentry_restart;
extern void __user __wombat_user_sigentry_int;
extern void __user __wombat_user_sigreturn;
extern void __user __wombat_user_rt_sigentry;
extern void __user __wombat_user_rt_sigentry_restart;
extern void __user __wombat_user_rt_sigentry_int;
extern void __user __wombat_user_rt_sigreturn;
extern void __user __wombat_user_sigrestore;

/*
 * Do a signal return; undo the signal stack.
 */

static int
restore_sigcontext(struct pt_regs *ptregs, struct sigcontext __user *sc)
{
	unsigned int err = 0;
	struct exregs_regs regs;
	struct restore_sigframe __user *restore;
	printk_dbg("%s frame = %p\n", __func__, sc);

	/* Always make any pending restarted system calls return -EINTR */
	current_thread_info()->restart_block.fn = do_no_restart_syscall;

//	COPY(edi);
//	COPY(esi);
//	COPY(ebp);
//	COPY(esp);
//	COPY(ebx);
//	COPY(edx);
//	COPY(ecx);
//	COPY(eip);
	
	err |= __get_user(regs.sp, &sc->esp);
	i386_put_esp(ptregs, regs.sp);	// for sig_rt_sigreturn
	err |= __get_user(regs.ip, &sc->eip);

	{
		unsigned int tmpflags;
		err |= __get_user(tmpflags, &sc->eflags);
		regs.flags = ((i386_eflags(ptregs) & ~0x40DD5) | (tmpflags & 0x40DD5));
	}
#if 0	/*oklinux: done in userland stub -gl */

	{
		struct _fpstate __user * buf;
		err |= __get_user(buf, &sc->fpstate);
		if (buf) {
			if (verify_area(VERIFY_READ, buf, sizeof(*buf)))
				goto badframe;
			err |= restore_i387(buf);
			BUG();
		}
	}
#endif

	restore = (void*)(regs.sp - sizeof(struct restore_sigframe));

	err |= __put_user(regs.ip, &restore->ret_ip);

	if (!err) {
		set_need_restart(current_thread_info(),
			(unsigned long)TASK_SIG_BASE +
			(((unsigned long)&__wombat_user_sigrestore) & ~PAGE_MASK),
			(unsigned long)sc, regs.flags);

		set_user_ipc_cancelled(current_thread_info());
	}

	return err;

badframe:
	return 1;
}

asmlinkage int sys_sigreturn(struct pt_regs *regs)
{
	struct sigframe __user *frame = (struct sigframe __user *)(i386_esp(regs) - 8);
	sigset_t set;
	printk_dbg("%s frame = %p\n", __func__, frame);

	if (verify_area(VERIFY_READ, frame, sizeof(*frame)))
		goto badframe;
	if (__get_user(set.sig[0], &frame->sc.oldmask)
	    || (_NSIG_WORDS > 1
		&& __copy_from_user(&set.sig[1], &frame->extramask,
				    sizeof(frame->extramask))))
		goto badframe;

	sigdelsetmask(&set, ~_BLOCKABLE);
	spin_lock_irq(&current->sighand->siglock);
	current->blocked = set;
	recalc_sigpending();
	spin_unlock_irq(&current->sighand->siglock);
	
	/* Disable restart check */
	i386_put_eflags(regs, i386_eflags(regs) & ~0x80000000 );

	if (restore_sigcontext(regs, &frame->sc))
		goto badframe;
	return 0;

badframe:
	force_sig(SIGSEGV, current);
	return 0;
}	

asmlinkage int sys_rt_sigreturn(struct pt_regs *regs)
{
	struct rt_sigframe __user *frame = (struct rt_sigframe __user *)(i386_esp(regs) - 4);
	sigset_t set;
	printk_dbg("%s frame = %p\n", __func__, frame);

	if (verify_area(VERIFY_READ, frame, sizeof(*frame)))
		goto badframe;
	if (__copy_from_user(&set, &frame->uc.uc_sigmask, sizeof(set)))
		goto badframe;

	sigdelsetmask(&set, ~_BLOCKABLE);
	spin_lock_irq(&current->sighand->siglock);
	current->blocked = set;
	recalc_sigpending();
	spin_unlock_irq(&current->sighand->siglock);
	
	/* Disable restart check */
	i386_put_eflags(regs, i386_eflags(regs) & ~0x80000000 );

	if (restore_sigcontext(regs, &frame->uc.uc_mcontext))
		goto badframe;

	if (do_sigaltstack(&frame->uc.uc_stack, NULL, i386_esp(regs)) == -EFAULT)
		goto badframe;

	return 0;

badframe:
	force_sig(SIGSEGV, current);
	return 0;
}	

/*
 * Set up a signal frame.
 */

static int
setup_sigcontext(struct sigcontext __user *sc, struct _fpstate __user *fpstate,
		 struct exregs_regs *regs, unsigned long mask)
{
	int tmp, err = 0;
	printk_dbg("%s frame = %p\n", __func__, sc);

	tmp = 0;
// XXX
//	__asm__("movl %%gs,%0" : "=r"(tmp): "0"(tmp));
//	err |= __put_user(tmp, (unsigned int *)&sc->gs);
//	__asm__("movl %%fs,%0" : "=r"(tmp): "0"(tmp));
//	err |= __put_user(tmp, (unsigned int *)&sc->fs);

//	err |= __put_user(regs->xes, (unsigned int *)&sc->es);
//	err |= __put_user(regs->xds, (unsigned int *)&sc->ds);
//	err |= __put_user(i386_edi(regs), &sc->edi);
//	err |= __put_user(i386_esi(regs), &sc->esi);
//	err |= __put_user(i386_ebp(regs), &sc->ebp);
	err |= __put_user(regs->sp, &sc->esp);
//	err |= __put_user(i386_ebx(regs), &sc->ebx);
//	err |= __put_user(i386_edx(regs), &sc->edx);
//	err |= __put_user(i386_ecx(regs), &sc->ecx);
	if (regs->syscall_action == 1)	/* Normal syscall, must writeback result */
		err |= __put_user(i386_eax(current_regs()), &sc->eax);

//	err |= __put_user(current->thread.trap_no, &sc->trapno);
//	err |= __put_user(current->thread.error_code, &sc->err);
//	err |= __put_user(i386_trap_no(regs), &sc->trapno);
//	err |= __put_user(i386_error_code(regs), &sc->err);
	err |= __put_user(regs->ip, &sc->eip);
//	err |= __put_user(regs->xcs, (unsigned int *)&sc->cs);
	err |= __put_user(regs->flags, &sc->eflags);
	regs->flags = regs->flags & (~0x80000000ul);	// Cancel restart - restart is saved above
	err |= __put_user(regs->sp, &sc->esp_at_signal);
//	err |= __put_user(regs->xss, (unsigned int *)&sc->ss);

#if 1	/*oklinux*/
	/*
	 * OKLinux: always put the pointer to the fpstate variable in 
	 * because we always save the fpstate in OK Linux.
	 */
	err |= __put_user(fpstate, &sc->fpstate);
#else
	tmp = save_i387(fpstate);
	if (tmp < 0)
		err = 1;
	else
		err |= __put_user(tmp ? fpstate : NULL, &sc->fpstate);
#endif
	

	/* non-iBCS2 extensions.. */
	err |= __put_user(mask, &sc->oldmask);
//	err |= __put_user(current->thread.cr2, &sc->cr2);

	return err;
}

/*
 * Determine which stack to use..
 */
static inline void __user *
get_sigframe(struct k_sigaction *ka, struct exregs_regs * regs, size_t frame_size)
{
	unsigned long esp;

	/* Default to using normal stack */
	esp = regs->sp;

	/* This is the X/Open sanctioned signal stack switching.  */
	if (ka->sa.sa_flags & SA_ONSTACK) {
		if (sas_ss_flags(esp) == 0)
			esp = current->sas_ss_sp + current->sas_ss_size;
	}

	/* This is the legacy signal stack switching. */
/*	else if ((regs->xss & 0xffff) != __USER_DS &&
		 !(ka->sa.sa_flags & SA_RESTORER) &&
		 ka->sa.sa_restorer) {
		esp = (unsigned long) ka->sa.sa_restorer;
	}*/// XXX we have no segments/legacy?

	/* Align the stack pointer according to the i386 ABI,
	 * i.e. so that on function ((sp + 4) & 15) == 0 */
	esp -= frame_size;
	esp &= -16ul;
	return (void __user *) esp;
}

static void setup_frame(int sig, struct k_sigaction *ka,
			sigset_t *set, struct exregs_regs * regs)
{
	void __user *restorer;
	struct sigframe __user *frame;
	int err = 0;
	int usig;

	printk_dbg("%s %d\n", __func__, __LINE__);
	frame = get_sigframe(ka, regs, sizeof(*frame));

	if (!access_ok(VERIFY_WRITE, frame, sizeof(*frame)))
		goto give_sigsegv;

	usig = (current_thread_info()->exec_domain
			&& current_thread_info()->exec_domain->signal_invmap
			&& sig < 32
			? current_thread_info()->exec_domain->signal_invmap[sig]
			: sig);

	err |= __put_user(usig, &frame->sig);
	if (err)
		goto give_sigsegv;

	err |= setup_sigcontext(&frame->sc, &frame->fpstate, regs, set->sig[0]);
	if (err)
		goto give_sigsegv;

	if (_NSIG_WORDS > 1) {
		err |= __copy_to_user(&frame->extramask, &set->sig[1],
				      sizeof(frame->extramask));
	}
	if (err)
		goto give_sigsegv;

	restorer = (void *)(TASK_SIG_BASE +
			((unsigned long)&__wombat_user_sigreturn & ~PAGE_MASK));
	if (ka->sa.sa_flags & SA_RESTORER)
		restorer = ka->sa.sa_restorer;

	/* Set up to return from userspace.  */
	err |= __put_user(restorer, &frame->pretcode);
	 
	printk_dbg("%s %d restorer = %p, frame = %p\n", __func__, __LINE__, restorer, frame);
	/*
	 * This is popl %eax ; movl $,%eax ; int $0x80
	 *
	 * WE DO NOT USE IT ANY MORE! It's only left here for historical
	 * reasons and because gdb uses it as a signature to notice
	 * signal handler stack frames.
	 */
	err |= __put_user(0xb858, (short *)(frame->retcode+0));
	err |= __put_user(__NR_sigreturn, (int *)(frame->retcode+2));
	err |= __put_user(0x80cd, (short *)(frame->retcode+6));

	/* Set up registers for signal handler */
	err |= __put_user(ka->sa.sa_handler, &frame->sig_ip);
	printk_dbg("%s frame->sig_ip = %p\n", __func__, ka->sa.sa_handler);

	if (err)
		goto give_sigsegv;

	regs->sp = (unsigned long) frame;
	regs->ip = TASK_SIG_BASE;
	switch (regs->syscall_action)
	{
	case 1:	/* Syscall */
		regs->ip += ((unsigned long)&__wombat_user_sigentry) & ~PAGE_MASK;
		break;
	case 0:	/* Fault */
	case 2:	/* Restart syscall */
		regs->ip += ((unsigned long)&__wombat_user_sigentry_restart) & ~PAGE_MASK;
		break;
	case 4:	/* Interrupt syscall */
		regs->ip += ((unsigned long)&__wombat_user_sigentry_int) & ~PAGE_MASK;
		break;
	default:
		BUG();
	}

	set_fs(USER_DS);
	regs->flags = regs->flags & ~TF_MASK;

	set_need_restart(current_thread_info(), regs->ip,
		regs->sp, regs->flags);

	printk_dbg("SIG deliver (%s:%d): sp=%p pc=%p ra=%p\n",
		current->comm, current->pid, frame, (void*)regs->ip, restorer);

	return;

give_sigsegv:
	force_sigsegv(sig, current);
}

static void setup_rt_frame(int sig, struct k_sigaction *ka, siginfo_t *info,
			   sigset_t *set, struct exregs_regs * regs)
{
	void *restorer;
	struct rt_sigframe __user *frame;
	int err = 0;
	int usig;

	printk_dbg("%s %d\n", __func__, __LINE__);
	frame = get_sigframe(ka, regs, sizeof(*frame));

	if (!access_ok(VERIFY_WRITE, frame, sizeof(*frame)))
		goto give_sigsegv;

	usig = current_thread_info()->exec_domain
		&& current_thread_info()->exec_domain->signal_invmap
		&& sig < 32
		? current_thread_info()->exec_domain->signal_invmap[sig]
		: sig;

	err |= __put_user(usig, &frame->sig);
	err |= __put_user(&frame->info, &frame->pinfo);
	err |= __put_user(&frame->uc, &frame->puc);
	err |= copy_siginfo_to_user(&frame->info, info);
	if (err)
		goto give_sigsegv;

	/* Create the ucontext.  */
	err |= __put_user(0, &frame->uc.uc_flags);
	err |= __put_user(0, &frame->uc.uc_link);
	err |= __put_user(current->sas_ss_sp, &frame->uc.uc_stack.ss_sp);
	err |= __put_user(sas_ss_flags(regs->sp),
			  &frame->uc.uc_stack.ss_flags);
	err |= __put_user(current->sas_ss_size, &frame->uc.uc_stack.ss_size);
	err |= setup_sigcontext(&frame->uc.uc_mcontext, &frame->fpstate,
			        regs, set->sig[0]);
	err |= __copy_to_user(&frame->uc.uc_sigmask, set, sizeof(*set));
	if (err)
		goto give_sigsegv;

	/* Set up to return from userspace.  */
	restorer = (void *)(TASK_SIG_BASE +
			((unsigned long)&__wombat_user_rt_sigreturn & ~PAGE_MASK));
	if (ka->sa.sa_flags & SA_RESTORER)
		restorer = ka->sa.sa_restorer;
	err |= __put_user(restorer, &frame->pretcode);
	printk_dbg("%s %d restorer = %p, frame = %p\n", __func__, __LINE__, restorer, frame);
	 
	/*
	 * This is movl $,%eax ; int $0x80
	 *
	 * WE DO NOT USE IT ANY MORE! It's only left here for historical
	 * reasons and because gdb uses it as a signature to notice
	 * signal handler stack frames.
	 */
	err |= __put_user(0xb8, (char *)(frame->retcode+0));
	err |= __put_user(__NR_rt_sigreturn, (int *)(frame->retcode+1));
	err |= __put_user(0x80cd, (short *)(frame->retcode+5));

	/* Set up registers for signal handler */
	err |= __put_user(ka->sa.sa_handler, &frame->sig_ip);
	printk_dbg("%s frame->sig_ip = %p\n", __func__, ka->sa.sa_handler);

	if (err)
		goto give_sigsegv;

	regs->sp = (unsigned long) frame;
	regs->ip = TASK_SIG_BASE;
	switch (regs->syscall_action)
	{
	case 1:	/* Syscall */
		regs->ip += ((unsigned long)&__wombat_user_rt_sigentry) & ~PAGE_MASK;
		break;
	case 0:	/* Fault */
	case 2:	/* Restart syscall */
		regs->ip += ((unsigned long)&__wombat_user_rt_sigentry_restart) & ~PAGE_MASK;
		break;
	case 4:	/* Interrupt syscall */
		regs->ip += ((unsigned long)&__wombat_user_rt_sigentry_int) & ~PAGE_MASK;
		break;
	default:
		BUG();
	}

	set_fs(USER_DS);
	regs->flags = regs->flags & ~TF_MASK;

	set_need_restart(current_thread_info(), regs->ip,
		regs->sp, regs->flags);

	printk_dbg("SIG deliver (%s:%d): sp=%p pc=%p ra=%p\n",
		current->comm, current->pid, frame, (void*)regs->ip, restorer);

	return;

give_sigsegv:
	force_sigsegv(sig, current);
}

void l4_arch_setup_frame(int signr, struct k_sigaction * ka, siginfo_t *info,
		                sigset_t *set, struct exregs_regs *regs)
{
	switch (regs->syscall_action)
	{
	case 0:	/* Fault */
	case 2:	/* Restart syscall */
		break;
	case 1:	/* Syscall */
	case 4:	/* Interrupt syscall */
		regs->ip += 2;
		break;
	}

	/*
	 * Set up the stack frame
	 */
	if (ka->sa.sa_flags & SA_SIGINFO)
		setup_rt_frame(signr, ka, info, set, regs);
	else
		setup_frame(signr, ka, set, regs);
}

