#include <l4.h>
#include <asm/macros.h>

#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/personality.h>

#include <asm/signal.h>
#include <asm/syscalls.h>
#include <asm/uaccess.h>
#include <asm/cacheflush.h>

#include <asm/elf.h>
#include <asm/ucontext.h>

#include INC_SYSTEM(unistd.h)
#include "sigframe.h"

#define SIG_DEBUG
#undef SIG_DEBUG

#ifdef SIG_DEBUG
#define printk_dbg  printk
#else
#define printk_dbg(...)
#endif

#define __put_user_error(x,ptr,err)					\
	err |= __put_user(x, ptr)

#define __get_user_error(x,ptr,err)					\
	err |= __get_user(x, ptr)

#define _BLOCKABLE (~(sigmask(SIGKILL) | sigmask(SIGSTOP)))

/*
 * For ARM syscalls, we encode the syscall number into the instruction.
 */
#define SWI_SYS_SIGRETURN       (0xef000000|(__NR_sigreturn))
#define SWI_SYS_RT_SIGRETURN    (0xef000000|(__NR_rt_sigreturn))

/*
 * With EABI, the syscall number has to be loaded into r7.
 */
#define MOV_R7_NR_SIGRETURN     (0xe3a07000 | (__NR_sigreturn - __NR_SYSCALL_BASE))
#define MOV_R7_NR_RT_SIGRETURN  (0xe3a07000 | (__NR_rt_sigreturn - __NR_SYSCALL_BASE))

/*
 * For Thumb syscalls, we pass the syscall number via r7.  We therefore
 * need two 16-bit instructions.
 */
#define SWI_THUMB_SIGRETURN     (0xdf00 << 16 | 0x2700 | (__NR_sigreturn - __NR_SYSCALL_BASE))
#define SWI_THUMB_RT_SIGRETURN  (0xdf00 << 16 | 0x2700 | (__NR_rt_sigreturn - __NR_SYSCALL_BASE))

static const unsigned long sigreturn_codes[7] = {
	MOV_R7_NR_SIGRETURN,	SWI_SYS_SIGRETURN,	SWI_THUMB_SIGRETURN,
	MOV_R7_NR_RT_SIGRETURN,	SWI_SYS_RT_SIGRETURN,	SWI_THUMB_RT_SIGRETURN
};

/* XXX fixme */
unsigned int elf_hwcap = 0;
char elf_platform[ELF_PLATFORM_SIZE];

asmlinkage int l4_do_signal(sigset_t *oldset, struct pt_regs *regs, int syscall);

extern void __user __wombat_user_sigentry;
extern void __user __wombat_user_sigentry_restart;
extern void __user __wombat_user_sigentry_int;
extern void __user __wombat_user_rt_sigentry;
extern void __user __wombat_user_rt_sigentry_restart;
extern void __user __wombat_user_rt_sigentry_int;
extern void __user __wombat_user_sigrestore;

static inline void *
get_sigframe(struct k_sigaction *ka, struct exregs_regs *regs, int framesize)
{
	unsigned long sp = regs->sp;
	printk_dbg("%s called\n", __func__);

	/*
	 * This is the X/Open sanctioned signal stack switching.
	 */
	if ((ka->sa.sa_flags & SA_ONSTACK) && !sas_ss_flags(sp))
		sp = current->sas_ss_sp + current->sas_ss_size;

	/*
	 * ATPCS B01 mandates 8-byte alignment
	 */
	return (void *)((sp - framesize) & ~7);
}

static int
setup_sigcontext(struct sigcontext *sc, /*struct _fpstate *fpstate,*/
		 struct exregs_regs *regs, unsigned long mask)
{
	int err;

	err = 0;

	if (regs->syscall_action == 1) /* continue syscall */
		__put_user_error(ARM_r0(current_regs()), &sc->arm_r0, err);

	/* User registers are saved/restored in user.S (__wombat_user_xxx) */
	__put_user_error(regs->sp, &sc->arm_sp, err);
	__put_user_error(regs->ip, &sc->arm_pc, err);
	__put_user_error(regs->flags, &sc->arm_cpsr, err);

	__put_user_error(/*current->thread.trap_no*/ 0, &sc->trap_no, err);
	__put_user_error(/*current->thread.error_code*/ 0, &sc->error_code, err);
	__put_user_error(/*current->thread.address*/ 0, &sc->fault_address, err);
	__put_user_error(mask, &sc->oldmask, err);

	return err;
}

static int
restore_sigcontext(struct sigcontext *sc)
{
	int err = 0;
	struct exregs_regs regs;
	struct restore_sigframe __user *restore;
	
	printk_dbg("%s called\n", __func__);

	/* User registers are saved/restored in user.S (__wombat_user_xxx) */
	__get_user_error(regs.sp, &sc->arm_sp, err);
	__get_user_error(regs.ip, &sc->arm_pc, err);
	__get_user_error(regs.flags, &sc->arm_cpsr, err);

	restore = (void*)(regs.sp - sizeof(struct restore_sigframe));

	__put_user_error(regs.ip, &restore->ret_ip, err);

	if (!err) {
		set_need_restart(current_thread_info(),
				(unsigned long)TASK_SIG_BASE +
				(((unsigned long)&__wombat_user_sigrestore) & ~PAGE_MASK),
				(unsigned long)sc, regs.flags);

		L4_Stop_Thread(current_thread_info()->user_tid);
		set_user_ipc_cancelled(current_thread_info());
	}

	return err;
}

static int
setup_return(struct exregs_regs *regs, struct k_sigaction *ka,
	     unsigned long __user *rc, void __user *frame,
	     unsigned long __user *lr, int __user *sig,
	     int usig)
{
	unsigned long retcode;
	int thumb = 0;

	unsigned long cpsr = regs->flags & ~PSR_f;
	unsigned long handler = (unsigned long)ka->sa.sa_handler;

	/*
	 * Maybe we need to deliver a 32-bit signal to a 26-bit task.
	 */
	if (ka->sa.sa_flags & SA_THIRTYTWO)
		cpsr = (cpsr & ~MODE_MASK) | USR_MODE;

	/* Thumb mode tests */
	/*
	 * The LSB of the handler determines if we're going to
	 * be using THUMB or ARM mode for this signal handler.
	 */
	thumb = handler & 1;

	if (thumb)
		cpsr |= PSR_T_BIT;
	else
		cpsr &= ~PSR_T_BIT;

	if (thumb)
	{
		printk("%d: Thumb mode handlers not suppoted\n", current->pid);
		return 1;
	}

	if (ka->sa.sa_flags & SA_RESTORER) {
		retcode = (unsigned long)ka->sa.sa_restorer;
	} else {
		unsigned int idx = thumb << 1;

		if (ka->sa.sa_flags & SA_SIGINFO)
			idx += 3;

		if (__put_user(sigreturn_codes[idx], rc) ||
		    __put_user(sigreturn_codes[idx+1], rc+1))
			return 1;

		/*
		 * Ensure that the instruction cache sees
		 * the return code written onto the stack.
		 */
		flush_icache_range((unsigned long)rc,
				   (unsigned long)(rc + 1));

		retcode = ((unsigned long)rc) + thumb;
	}
	if (__put_user(usig, sig))
		return 1;
	if (__put_user(retcode, lr))
		return 1;
	printk_dbg("%s %d restorer = %lx, frame = %p\n", __func__, __LINE__, retcode, frame);

	regs->sp = (unsigned long)frame;
	regs->flags = cpsr;

//	ARM_put_r0(regs, usig);
//	ARM_put_lr(regs, retcode);
	return 0;
}

static void
setup_frame(int usig, struct k_sigaction *ka, sigset_t *set, struct exregs_regs *regs)
{
	struct sigframe *frame = get_sigframe(ka, regs, sizeof(*frame));
	unsigned long handler = (unsigned long)ka->sa.sa_handler;
	int err = 0;
	printk_dbg("%s called\n", __func__);

	if (!access_ok(VERIFY_WRITE, frame, sizeof (*frame)))
		goto badframe;

	err |= setup_sigcontext(&frame->sc, /*&frame->fpstate,*/ regs, set->sig[0]);

	if (_NSIG_WORDS > 1) {
		err |= __copy_to_user(frame->extramask, &set->sig[1],
				      sizeof(frame->extramask));
	}

	err |= __put_user(handler, &frame->sig_ip);
	printk_dbg("%s frame->sig_ip = %lx\n", __func__, handler);

	if (!err)
		err = setup_return(regs, ka, &frame->retcode, frame,
				&frame->lr, &frame->usig, usig);

	if (err)
		goto badframe;

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

	set_need_restart(current_thread_info(), regs->ip,
			regs->sp, regs->flags);

	printk_dbg("SIG deliver (%s:%d:%lx): sp=%p pc=%p\n",
			current->comm, current->pid,
			current_thread_info()->user_tid.raw,
			frame, (void*)regs->ip);

	return;

badframe:
	force_sigsegv(usig, current);
	return;
}

static void
setup_rt_frame(int usig, struct k_sigaction *ka, siginfo_t *info,
	       sigset_t *set, struct exregs_regs *regs)
{
	struct rt_sigframe *frame = get_sigframe(ka, regs, sizeof(*frame));
	unsigned long handler = (unsigned long)ka->sa.sa_handler;
	int err = 0;
	printk_dbg("%s called\n", __func__);

	if (!access_ok(VERIFY_WRITE, frame, sizeof (*frame)))
		goto badframe;

	__put_user_error(&frame->info, &frame->pinfo, err);
	__put_user_error(&frame->uc, &frame->puc, err);
	err |= copy_siginfo_to_user(&frame->info, info);

	/* Clear all the bits of the ucontext we don't use.  */
	err |= clear_user(&frame->uc, offsetof(struct ucontext, uc_mcontext));

	err |= setup_sigcontext(&frame->uc.uc_mcontext, /*&frame->fpstate,*/
				regs, set->sig[0]);
	err |= copy_to_user(&frame->uc.uc_sigmask, set, sizeof(*set));

	err |= __put_user(handler, &frame->sig_ip);
	printk_dbg("%s frame->sig_ip = %lx\n", __func__, handler);

	if (!err)
		err = setup_return(regs, ka, &frame->retcode, frame,
				&frame->lr, &frame->usig, usig);

	if (!err) {
		/*
		 * For realtime signals we must also set the second and third
		 * arguments for the signal handler.
		 *   -- Peter Maydell <pmaydell@chiark.greenend.org.uk> 2000-12-06
		 */
//		ARM_put_r1(regs, (unsigned long)frame->pinfo);
//		ARM_put_r2(regs, (unsigned long)frame->puc);
	}

	if (err)
		goto badframe;

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

	set_need_restart(current_thread_info(), regs->ip,
			regs->sp, regs->flags);

	printk_dbg("SIG rt deliver (%s:%d:%lx): sp=%p pc=%p\n",
			current->comm, current->pid,
			current_thread_info()->user_tid.raw,
			frame, (void*)regs->ip);

	return;

badframe:
	force_sigsegv(usig, current);
	return;
}

void l4_arch_setup_frame(int signr, struct k_sigaction * ka, siginfo_t *info,
		                sigset_t *set, struct exregs_regs *regs)
{
	struct thread_info *thread = current_thread_info();
	int usig = signr;

	/*
	 * translate the signal
	 */
	if (usig < 32 && thread->exec_domain && thread->exec_domain->signal_invmap)
		usig = thread->exec_domain->signal_invmap[usig];

	printk_dbg("%s %d\n", __func__, usig);

	if (regs->syscall_action == 2) /* restart syscall */
		regs->ip -= 4;
	/*
	 * Set up the stack frame
	 */
	if (ka->sa.sa_flags & SA_SIGINFO)
		setup_rt_frame(usig, ka, info, set, regs);
	else
		setup_frame(usig, ka, set, regs);
}

/*
 * atomically swap in the new signal mask, and wait for a signal.
 */
asmlinkage int sys_sigsuspend(int restart, unsigned long oldmask, old_sigset_t mask, struct pt_regs *regs)
{
	sigset_t saveset;

	printk_dbg("XXX - %s() called\n", __func__);

	mask &= _BLOCKABLE;
	spin_lock_irq(&current->sighand->siglock);
	saveset = current->blocked;
	siginitset(&current->blocked, mask);
	recalc_sigpending();
	spin_unlock_irq(&current->sighand->siglock);
	ARM_put_r0(regs, -ERESTART_RESTARTBLOCK);

	while (1) {
		current->state = TASK_INTERRUPTIBLE;
		schedule();
		if (l4_do_signal(&saveset, regs, 1))
			return ARM_r0(regs);
	}
}

asmlinkage int 
sys_sigaction(int sig, const struct old_sigaction *act,
	      struct old_sigaction *oact)
{
	struct k_sigaction new_ka, old_ka;
	int ret;

	printk_dbg("XXX - %s() called\n", __func__);

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

asmlinkage int sys_sigreturn(struct pt_regs *regs)
{
	struct sigframe *frame;
	sigset_t set;

	printk_dbg("XXX - %s() called\n", __func__);

	/* Always make any pending restarted system calls return -EINTR */
	current_thread_info()->restart_block.fn = do_no_restart_syscall;

        /*
	 * Since we stacked the signal on a 64-bit boundary,
	 * then 'sp' should be word aligned here.  If it's
	 * not, then the user is trying to mess with us.
	 */
	if (ARM_sp(regs) & 7)
		goto badframe;

	frame = (struct sigframe *)ARM_sp(regs);

	if (verify_area(VERIFY_READ, frame, sizeof (*frame)))
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

	if (restore_sigcontext(&frame->sc))
		goto badframe;

	return ARM_r0(regs);

badframe:
	force_sig(SIGSEGV, current);
	return 0;
}

asmlinkage int
sys_rt_sigsuspend(sigset_t *unewset, size_t sigsetsize, struct pt_regs *regs)
{
	sigset_t saveset, newset;
	printk_dbg("XXX - %s() called\n", __func__);

	/* XXX: Don't preclude handling different sized sigset_t's. */
	if (sigsetsize != sizeof(sigset_t))
		return -EINVAL;

	if (copy_from_user(&newset, unewset, sizeof(newset)))
		return -EFAULT;
	sigdelsetmask(&newset, ~_BLOCKABLE);

	spin_lock_irq(&current->sighand->siglock);
	saveset = current->blocked;
	current->blocked = newset;
	recalc_sigpending();
	spin_unlock_irq(&current->sighand->siglock);
	ARM_put_r0(regs, -ERESTART_RESTARTBLOCK);

	while (1) {
		current->state = TASK_INTERRUPTIBLE;
		schedule();
		if (l4_do_signal(&saveset, regs, 1))
			return ARM_r0(regs);
	}
}

asmlinkage int
sys_sigaltstack(const stack_t __user *uss, stack_t __user *uoss, struct pt_regs *regs)
{
	printk_dbg("XXX - %s() called\n", __func__);
	return do_sigaltstack(uss, uoss, ARM_sp(regs));
}

asmlinkage int sys_rt_sigreturn(struct pt_regs *regs)
{
	struct rt_sigframe *frame;
	sigset_t set;

	printk_dbg("XXX - %s() called\n", __func__);

	/* Always make any pending restarted system calls return -EINTR */
	current_thread_info()->restart_block.fn = do_no_restart_syscall;

	/*
	 * Since we stacked the signal on a 64-bit boundary,
	 * then 'sp' should be word aligned here.  If it's
	 * not, then the user is trying to mess with us.
	 */
	if (ARM_sp(regs) & 7)
		goto badframe;

	frame = (struct rt_sigframe *)ARM_sp(regs);

	if (verify_area(VERIFY_READ, frame, sizeof (*frame)))
		goto badframe;
	if (__copy_from_user(&set, &frame->uc.uc_sigmask, sizeof(set)))
		goto badframe;

	sigdelsetmask(&set, ~_BLOCKABLE);
	spin_lock_irq(&current->sighand->siglock);
	current->blocked = set;
	recalc_sigpending();
	spin_unlock_irq(&current->sighand->siglock);

	if (restore_sigcontext(&frame->uc.uc_mcontext))
		goto badframe;

	return ARM_r0(regs);

badframe:
	force_sig(SIGSEGV, current);
	return 0;
}
