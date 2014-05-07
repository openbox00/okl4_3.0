#include <linux/sched.h>
#include <linux/errno.h>
#include <asm/signal.h>
#include <asm/syscalls.h>

void l4_arch_setup_frame(int signr, struct k_sigaction * ka, siginfo_t *info,
		                sigset_t *set, struct pt_regs *regs, int syscall)
{
    printk("l4_arch_setup_frame called\n");
    return;
}

#ifdef CONFIG_COMPAT

#include <asm/uaccess.h>
#include <asm/siginfo.h>

#define _BLOCKABLE (~(sigmask(SIGKILL) | sigmask(SIGSTOP)))

/* 32-bit compatibility types */

#define _NSIG_BPW32	32
#define _NSIG_WORDS32	(_NSIG / _NSIG_BPW32)

typedef struct {
	unsigned int sig[_NSIG_WORDS32];
} sigset_t32;

typedef unsigned int __sighandler32_t;
typedef void (*vfptr_t)(void);

struct sigaction32 {
	unsigned int		sa_flags;
	__sighandler32_t	sa_handler;
	compat_sigset_t		sa_mask;
};

/* IRIX compatible stack_t  */
typedef struct sigaltstack32 {
	s32 ss_sp;
	compat_size_t ss_size;
	int ss_flags;
} stack32_t;

struct ucontext32 {
	u32                 uc_flags;
	s32                 uc_link;
	stack32_t           uc_stack;
	struct sigcontext32 uc_mcontext;
	sigset_t32          uc_sigmask;   /* mask last for extensibility */
};

struct sigframe {
	u32 sf_ass[4];			/* argument save space for o32 */
	u32 sf_code[2];			/* signal trampoline */
	struct sigcontext32 sf_sc;
	sigset_t sf_mask;
};

struct rt_sigframe32 {
	u32 rs_ass[4];			/* argument save space for o32 */
	u32 rs_code[2];			/* signal trampoline */
	struct siginfo32 rs_info;
	struct ucontext32 rs_uc;
};

extern void __put_sigset_unknown_nsig(void);
extern void __get_sigset_unknown_nsig(void);

static inline int put_sigset(const sigset_t *kbuf, compat_sigset_t *ubuf)
{
	int err = 0;

	if (!access_ok(VERIFY_WRITE, ubuf, sizeof(*ubuf)))
		return -EFAULT;

	switch (_NSIG_WORDS) {
	default:
		__put_sigset_unknown_nsig();
	case 2:
		err |= __put_user (kbuf->sig[1] >> 32, &ubuf->sig[3]);
		err |= __put_user (kbuf->sig[1] & 0xffffffff, &ubuf->sig[2]);
	case 1:
		err |= __put_user (kbuf->sig[0] >> 32, &ubuf->sig[1]);
		err |= __put_user (kbuf->sig[0] & 0xffffffff, &ubuf->sig[0]);
	}

	return err;
}

static inline int get_sigset(sigset_t *kbuf, const compat_sigset_t *ubuf)
{
	int err = 0;
	unsigned long sig[4];

	if (!access_ok(VERIFY_READ, ubuf, sizeof(*ubuf)))
		return -EFAULT;

	switch (_NSIG_WORDS) {
	default:
		__get_sigset_unknown_nsig();
	case 2:
		err |= __get_user (sig[3], &ubuf->sig[3]);
		err |= __get_user (sig[2], &ubuf->sig[2]);
		kbuf->sig[1] = sig[2] | (sig[3] << 32);
	case 1:
		err |= __get_user (sig[1], &ubuf->sig[1]);
		err |= __get_user (sig[0], &ubuf->sig[0]);
		kbuf->sig[0] = sig[0] | (sig[1] << 32);
	}

	return err;
}

static int restore_sigcontext32(struct pt_regs *regs,
					   struct sigcontext32 *sc)
{
	int err = 0;
BUG();
#if 0

	err |= __get_user(regs->cp0_epc, &sc->sc_pc);
	err |= __get_user(regs->hi, &sc->sc_mdhi);
	err |= __get_user(regs->lo, &sc->sc_mdlo);

#define restore_gp_reg(i) do {						\
	err |= __get_user(regs->regs[i], &sc->sc_regs[i]);		\
} while(0)
	restore_gp_reg( 1); restore_gp_reg( 2); restore_gp_reg( 3);
	restore_gp_reg( 4); restore_gp_reg( 5); restore_gp_reg( 6);
	restore_gp_reg( 7); restore_gp_reg( 8); restore_gp_reg( 9);
	restore_gp_reg(10); restore_gp_reg(11); restore_gp_reg(12);
	restore_gp_reg(13); restore_gp_reg(14); restore_gp_reg(15);
	restore_gp_reg(16); restore_gp_reg(17); restore_gp_reg(18);
	restore_gp_reg(19); restore_gp_reg(20); restore_gp_reg(21);
	restore_gp_reg(22); restore_gp_reg(23); restore_gp_reg(24);
	restore_gp_reg(25); restore_gp_reg(26); restore_gp_reg(27);
	restore_gp_reg(28); restore_gp_reg(29); restore_gp_reg(30);
	restore_gp_reg(31);
#undef restore_gp_reg

	err |= __get_user(current->used_math, &sc->sc_used_math);

	if (current->used_math) {
		/* restore fpu context if we have used it before */
		own_fpu();
		err |= restore_fp_context32(sc);
	} else {
		/* signal handler may have used FPU.  Give it up. */
		lose_fpu();
	}

#endif
	return err;
}

void sys32_sigreturn(struct pt_regs *regs)
{
	struct sigframe *frame;
	sigset_t blocked;

	frame = (struct sigframe *) MIPS_sp(regs);
	if (!access_ok(VERIFY_READ, frame, sizeof(*frame)))
		goto badframe;
	if (__copy_from_user(&blocked, &frame->sf_mask, sizeof(blocked)))
		goto badframe;

	sigdelsetmask(&blocked, ~_BLOCKABLE);
	spin_lock_irq(&current->sighand->siglock);
	current->blocked = blocked;
	recalc_sigpending();
	spin_unlock_irq(&current->sighand->siglock);

	if (restore_sigcontext32(regs, &frame->sf_sc))
		goto badframe;

	/*
	 * Don't let your children do this ...
	 */
	BUG();
//XXX	if (current_thread_info()->flags & TIF_SYSCALL_TRACE)
//		do_syscall_trace();
//	__asm__ __volatile__(
//		"move\t$29, %0\n\t"
//		"j\tsyscall_exit"
//		:/* no outputs */
//		:"r" (&regs));
	/* Unreached */

badframe:
	force_sig(SIGSEGV, current);
}

/*
 * Atomically swap in the new signal mask, and wait for a signal.
 */
inline int sys32_sigsuspend(struct pt_regs regs)
{
	BUG();
	return -EFAULT;
#if 0
	compat_sigset_t *uset;
	sigset_t newset, saveset;

	save_static(&regs);
	uset = (compat_sigset_t *) regs.regs[4];
	if (get_sigset(&newset, uset))
		return -EFAULT;
	sigdelsetmask(&newset, ~_BLOCKABLE);

	spin_lock_irq(&current->sighand->siglock);
	saveset = current->blocked;
	current->blocked = newset;
	recalc_sigpending();
	spin_unlock_irq(&current->sighand->siglock);

	regs.regs[2] = EINTR;
	regs.regs[7] = 1;
	while (1) {
		current->state = TASK_INTERRUPTIBLE;
		schedule();
		if (do_signal32(&saveset, &regs))
			return -EINTR;
	}
#endif
}

void sys32_rt_sigreturn(struct pt_regs *regs)
{
	struct rt_sigframe32 *frame;
	sigset_t set;
	stack_t st;
	s32 sp;

	frame = (struct rt_sigframe32 *) MIPS_sp(regs);
	if (!access_ok(VERIFY_READ, frame, sizeof(*frame)))
		goto badframe;
	if (__copy_from_user(&set, &frame->rs_uc.uc_sigmask, sizeof(set)))
		goto badframe;

	sigdelsetmask(&set, ~_BLOCKABLE);
	spin_lock_irq(&current->sighand->siglock);
	current->blocked = set;
	recalc_sigpending();
	spin_unlock_irq(&current->sighand->siglock);

	if (restore_sigcontext32(regs, &frame->rs_uc.uc_mcontext))
		goto badframe;

	/* The ucontext contains a stack32_t, so we must convert!  */
	if (__get_user(sp, &frame->rs_uc.uc_stack.ss_sp))
		goto badframe;
	st.ss_size = (long) sp;
	if (__get_user(st.ss_size, &frame->rs_uc.uc_stack.ss_size))
		goto badframe;
	if (__get_user(st.ss_flags, &frame->rs_uc.uc_stack.ss_flags))
		goto badframe;

	/* It is more difficult to avoid calling this function than to
	   call it and ignore errors.  */
	do_sigaltstack(&st, NULL, MIPS_sp(regs));

BUG();
	/*
	 * Don't let your children do this ...
	 */
//	__asm__ __volatile__(
//		"move\t$29, %0\n\t"
//		"j\tsyscall_exit"
//		:/* no outputs */
//		:"r" (&regs));
	/* Unreached */

badframe:
	force_sig(SIGSEGV, current);
}

asmlinkage int sys32_sigaction(int sig, const struct sigaction32 *act,
                               struct sigaction32 *oact)
{
	struct k_sigaction new_ka, old_ka;
	int ret;
	int err = 0;

	if (act) {
		old_sigset_t mask;
		s32 handler;

		if (!access_ok(VERIFY_READ, act, sizeof(*act)))
			return -EFAULT;
		err |= __get_user(handler, &act->sa_handler);
		new_ka.sa.sa_handler = (__sighandler_t)(s64)handler;
		err |= __get_user(new_ka.sa.sa_flags, &act->sa_flags);
		err |= __get_user(mask, &act->sa_mask.sig[0]);
		if (err)
			return -EFAULT;

		siginitset(&new_ka.sa.sa_mask, mask);
	}

	ret = do_sigaction(sig, act ? &new_ka : NULL, oact ? &old_ka : NULL);

	if (!ret && oact) {
		if (!access_ok(VERIFY_WRITE, oact, sizeof(*oact)))
                        return -EFAULT;
		err |= __put_user(old_ka.sa.sa_flags, &oact->sa_flags);
		err |= __put_user((u32)(u64)old_ka.sa.sa_handler,
		                  &oact->sa_handler);
		BUG();
//		err |= __put_user(old_ka.sa.sa_mask.sig[0], oact->sa_mask.sig);
                err |= __put_user(0, &oact->sa_mask.sig[1]);
                err |= __put_user(0, &oact->sa_mask.sig[2]);
                err |= __put_user(0, &oact->sa_mask.sig[3]);
                if (err)
			return -EFAULT;
	}

	return ret;
}

int sys32_rt_sigaction(int sig, const struct sigaction32 *act,
				  struct sigaction32 *oact,
				  unsigned int sigsetsize)
{
	struct k_sigaction new_sa, old_sa;
	int ret = -EINVAL;

	/* XXX: Don't preclude handling different sized sigset_t's.  */
	if (sigsetsize != sizeof(sigset_t))
		goto out;

	if (act) {
		s32 handler;

		int err = 0;

		if (!access_ok(VERIFY_READ, act, sizeof(*act)))
			return -EFAULT;
		err |= __get_user(handler, &act->sa_handler);
		new_sa.sa.sa_handler = (__sighandler_t)(s64)handler;
		err |= __get_user(new_sa.sa.sa_flags, &act->sa_flags);
		err |= get_sigset(&new_sa.sa.sa_mask, &act->sa_mask);
		if (err)
			return -EFAULT;
	}

	ret = do_sigaction(sig, act ? &new_sa : NULL, oact ? &old_sa : NULL);

	if (!ret && oact) {
		int err = 0;

		if (!access_ok(VERIFY_WRITE, oact, sizeof(*oact)))
			return -EFAULT;

		err |= __put_user((u32)(u64)old_sa.sa.sa_handler,
		                   &oact->sa_handler);
		err |= __put_user(old_sa.sa.sa_flags, &oact->sa_flags);
		err |= put_sigset(&old_sa.sa.sa_mask, &oact->sa_mask);
		if (err)
			return -EFAULT;
	}
out:
	return ret;
}

asmlinkage long sys_rt_sigprocmask(int how, sigset_t *set, sigset_t *oset,
				   size_t sigsetsize);

int sys32_rt_sigprocmask(int how, compat_sigset_t *set,
	compat_sigset_t *oset, unsigned int sigsetsize)
{
	sigset_t old_set, new_set;
	int ret;
	mm_segment_t old_fs = get_fs();

	if (set && get_sigset(&new_set, set))
		return -EFAULT;

	set_fs (KERNEL_DS);
	ret = sys_rt_sigprocmask(how, set ? &new_set : NULL,
				 oset ? &old_set : NULL, sigsetsize);
	set_fs (old_fs);

	if (!ret && oset && put_sigset(&old_set, oset))
		return -EFAULT;

	return ret;
}

asmlinkage long sys_rt_sigpending(sigset_t *set, size_t sigsetsize);

int sys32_rt_sigpending(compat_sigset_t *uset,
	unsigned int sigsetsize)
{
	int ret;
	sigset_t set;
	mm_segment_t old_fs = get_fs();

	set_fs (KERNEL_DS);
	ret = sys_rt_sigpending(&set, sigsetsize);
	set_fs (old_fs);

	if (!ret && put_sigset(&set, uset))
		return -EFAULT;

	return ret;
}

static int copy_siginfo_to_user32(siginfo_t32 *to, siginfo_t *from)
{
	int err;

	if (!access_ok (VERIFY_WRITE, to, sizeof(siginfo_t32)))
		return -EFAULT;

	/* If you change siginfo_t structure, please be sure
	   this code is fixed accordingly.
	   It should never copy any pad contained in the structure
	   to avoid security leaks, but must copy the generic
	   3 ints plus the relevant union member.
	   This routine must convert siginfo from 64bit to 32bit as well
	   at the same time.  */
	err = __put_user(from->si_signo, &to->si_signo);
	err |= __put_user(from->si_errno, &to->si_errno);
	err |= __put_user((short)from->si_code, &to->si_code);
	if (from->si_code < 0)
		err |= __copy_to_user(&to->_sifields._pad, &from->_sifields._pad, SI_PAD_SIZE);
	else {
		switch (from->si_code >> 16) {
		case __SI_CHLD >> 16:
			err |= __put_user(from->si_utime, &to->si_utime);
			err |= __put_user(from->si_stime, &to->si_stime);
			err |= __put_user(from->si_status, &to->si_status);
		default:
			err |= __put_user(from->si_pid, &to->si_pid);
			err |= __put_user(from->si_uid, &to->si_uid);
			break;
		case __SI_FAULT >> 16:
			err |= __put_user((long)from->si_addr, &to->si_addr);
			break;
		case __SI_POLL >> 16:
			err |= __put_user(from->si_band, &to->si_band);
			err |= __put_user(from->si_fd, &to->si_fd);
			break;
		/* case __SI_RT: This is not generated by the kernel as of now.  */
		}
	}
	return err;
}

int sys32_rt_sigtimedwait(compat_sigset_t *uthese,
	siginfo_t32 *uinfo, struct compat_timespec *uts,
	compat_time_t sigsetsize)
{
	int ret, sig;
	sigset_t these;
	compat_sigset_t these32;
	struct timespec ts;
	siginfo_t info;
	long timeout = 0;

	/*
	 * As the result of a brainfarting competition a few years ago the
	 * size of sigset_t for the 32-bit kernel was choosen to be 128 bits
	 * but nothing so far is actually using that many, 64 are enough.  So
	 * for now we just drop the high bits.
	 */
	if (copy_from_user (&these32, uthese, sizeof(compat_old_sigset_t)))
		return -EFAULT;

	switch (_NSIG_WORDS) {
#ifdef __MIPSEB__
	case 4: these.sig[3] = these32.sig[6] | (((long)these32.sig[7]) << 32);
	case 3: these.sig[2] = these32.sig[4] | (((long)these32.sig[5]) << 32);
	case 2: these.sig[1] = these32.sig[2] | (((long)these32.sig[3]) << 32);
	case 1: these.sig[0] = these32.sig[0] | (((long)these32.sig[1]) << 32);
#endif
#ifdef __MIPSEL__
	case 4: these.sig[3] = these32.sig[7] | (((long)these32.sig[6]) << 32);
	case 3: these.sig[2] = these32.sig[5] | (((long)these32.sig[4]) << 32);
	case 2: these.sig[1] = these32.sig[3] | (((long)these32.sig[2]) << 32);
	case 1: these.sig[0] = these32.sig[1] | (((long)these32.sig[0]) << 32);
#endif
	}

	/*
	 * Invert the set of allowed signals to get those we
	 * want to block.
	 */
	sigdelsetmask(&these, sigmask(SIGKILL)|sigmask(SIGSTOP));
	signotset(&these);

	if (uts) {
		if (get_user (ts.tv_sec, &uts->tv_sec) ||
		    get_user (ts.tv_nsec, &uts->tv_nsec))
			return -EINVAL;
		if (ts.tv_nsec >= 1000000000L || ts.tv_nsec < 0
		    || ts.tv_sec < 0)
			return -EINVAL;
	}

	spin_lock_irq(&current->sighand->siglock);
	sig = dequeue_signal(current, &these, &info);
	if (!sig) {
		/* None ready -- temporarily unblock those we're interested
		   in so that we'll be awakened when they arrive.  */
		sigset_t oldblocked = current->blocked;
		sigandsets(&current->blocked, &current->blocked, &these);
		recalc_sigpending();
		spin_unlock_irq(&current->sighand->siglock);

		timeout = MAX_SCHEDULE_TIMEOUT;
		if (uts)
			timeout = (timespec_to_jiffies(&ts)
				   + (ts.tv_sec || ts.tv_nsec));

		current->state = TASK_INTERRUPTIBLE;
		timeout = schedule_timeout(timeout);

		spin_lock_irq(&current->sighand->siglock);
		sig = dequeue_signal(current, &these, &info);
		current->blocked = oldblocked;
		recalc_sigpending();
	}
	spin_unlock_irq(&current->sighand->siglock);

	if (sig) {
		ret = sig;
		if (uinfo) {
			if (copy_siginfo_to_user32(uinfo, &info))
				ret = -EFAULT;
		}
	} else {
		ret = -EAGAIN;
		if (timeout)
			ret = -EINTR;
	}

	return ret;
}

extern asmlinkage int sys_rt_sigqueueinfo(int pid, int sig, siginfo_t *uinfo);

int sys32_rt_sigqueueinfo(int pid, int sig, siginfo_t32 *uinfo)
{
	siginfo_t info;
	int ret;
	mm_segment_t old_fs = get_fs();

	if (copy_from_user (&info, uinfo, 3*sizeof(int)) ||
	    copy_from_user (info._sifields._pad, uinfo->_sifields._pad, SI_PAD_SIZE))
		return -EFAULT;
	set_fs (KERNEL_DS);
	ret = sys_rt_sigqueueinfo(pid, sig, &info);
	set_fs (old_fs);
	return ret;
}
#endif
