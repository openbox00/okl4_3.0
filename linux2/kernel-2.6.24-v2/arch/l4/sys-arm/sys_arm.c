#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/shm.h>
#include <linux/file.h>
#include <linux/module.h>

#include <asm/uaccess.h>

/*
 * sys_pipe() is the normal C calling standard for creating
 * a pipe. It's not the way unix traditionally does this, though.
 */
asmlinkage int sys_pipe(unsigned long * fildes)
{
	int fd[2];
	int error;

	error = do_pipe(fd);
	if (!error) {
		if (copy_to_user(fildes, fd, 2*sizeof(int)))
			error = -EFAULT;
	}
	return error;
}

/*
 * Perform the select(nd, in, out, ex, tv) and mmap() system
 * calls.
 */

struct sel_arg_struct {
	unsigned long n;
	fd_set *inp, *outp, *exp;
	struct timeval *tvp;
};

asmlinkage int old_select(struct sel_arg_struct *arg)
{
	struct sel_arg_struct a;

	if (copy_from_user(&a, arg, sizeof(a)))
		return -EFAULT;
	/* sys_select() does the appropriate kernel locking */
	return sys_select(a.n, a.inp, a.outp, a.exp, a.tvp);
}

/* Clone a task - this clones the calling program thread.
 * This is called indirectly via a small wrapper
 */
asmlinkage int sys_clone(struct pt_regs *regs)
{
	unsigned long clone_flags = ARM_r0(regs);
	unsigned long newsp = ARM_r1(regs);

	if (!newsp)
		newsp = ARM_sp(regs);

	current_thread_info()->request.op = OP_FORK;

	return do_fork(clone_flags, newsp, regs, 0,
		(void*)ARM_r2(regs),	// parent_tidptr
		(void*)ARM_r4(regs));	// child_tidptr
}

/*
 * sys_ipc() is the de-multiplexer for the SysV IPC calls..
 *
 * This is really horribly ugly.
 */
asmlinkage int sys_ipc(uint call, int first, int second, int third,
		       void __user *ptr, long fifth)
{
	int version, ret;

	version = call >> 16; /* hack for backward compatibility */
	call &= 0xffff;

	switch (call) {
	case SEMOP:
		return sys_semop(first, (struct sembuf __user *)ptr, second);
	case SEMGET:
		return sys_semget (first, second, third);
	case SEMCTL: {
		union semun fourth;
		if (!ptr)
			return -EINVAL;
		if (get_user(fourth.__pad, (void __user * __user *) ptr))
			return -EFAULT;
		return sys_semctl (first, second, third, fourth);
	}

	case MSGSND:
		return sys_msgsnd(first, (struct msgbuf __user *) ptr, 
				  second, third);
	case MSGRCV:
		switch (version) {
		case 0: {
			struct ipc_kludge tmp;
			if (!ptr)
				return -EINVAL;
			if (copy_from_user(&tmp,(struct ipc_kludge __user *)ptr,
					   sizeof (tmp)))
				return -EFAULT;
			return sys_msgrcv (first, tmp.msgp, second,
					   tmp.msgtyp, third);
		}
		default:
			return sys_msgrcv (first,
					   (struct msgbuf __user *) ptr,
					   second, fifth, third);
		}
	case MSGGET:
		return sys_msgget ((key_t) first, second);
	case MSGCTL:
		return sys_msgctl(first, second, (struct msqid_ds __user *)ptr);

	case SHMAT:
		switch (version) {
		default: {
			ulong raddr;
			ret = do_shmat(first, (char __user *)ptr, second, &raddr);
			if (ret)
				return ret;
			return put_user(raddr, (ulong __user *)third);
		}
		case 1: /* Of course, we don't support iBCS2! */
			return -EINVAL;
		}
	case SHMDT: 
		return sys_shmdt ((char __user *)ptr);
	case SHMGET:
		return sys_shmget (first, second, third);
	case SHMCTL:
		return sys_shmctl (first, second,
				   (struct shmid_ds __user *) ptr);
	default:
		return -ENOSYS;
	}
}

void
armelf_core_copy_regs(elf_gregset_t *elfregs, struct pt_regs *regs)
{
	int i;

	L4_Copy_regs_to_mrs(current_thread_info()->user_tid);

	for (i = 0; i < ELF_NGREG - 1; i++)
		L4_StoreMR(i, &(*elfregs)[i]);
	/* XXX: what should we do about orig_r0? */
}

int
armelf_core_copy_regs_task(struct task_struct *t, elf_gregset_t *elfregs)
{
	int i;

	L4_Copy_regs_to_mrs(task_thread_info(t)->user_tid);

	for (i = 0; i < ELF_NGREG - 1; i++)
		L4_StoreMR(i, &(*elfregs)[i]);
	return 1;
}


extern void __modsi3(void);
extern void __umodsi3(void);
extern void __udivsi3(void);

EXPORT_SYMBOL(__modsi3);
EXPORT_SYMBOL(__umodsi3);
EXPORT_SYMBOL(__udivsi3);
EXPORT_SYMBOL(memset);
EXPORT_SYMBOL(memcpy);
EXPORT_SYMBOL(memmove);
