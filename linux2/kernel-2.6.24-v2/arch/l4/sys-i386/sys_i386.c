#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/shm.h>
#include <linux/syscalls.h>
#include <linux/file.h>

#include <asm/uaccess.h>

#include INC_SYSTEM2(elf.h)

/*
 * sys_pipe() is the normal C calling standard for creating
 * a pipe. It's not the way unix traditionally does this, though.
 */
int sys_pipe(unsigned long * fildes)
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
	fd_set __user *inp, *outp, *exp;
	struct timeval __user *tvp;
};

int old_select(struct sel_arg_struct __user *arg)
{
	struct sel_arg_struct a;

	if (copy_from_user(&a, arg, sizeof(a)))
		return -EFAULT;
	/* sys_select() does the appropriate kernel locking */
	return sys_select(a.n, a.inp, a.outp, a.exp, a.tvp);
}

int sys_clone(struct pt_regs *regs)
{
	unsigned long clone_flags;
	unsigned long newsp;
	int __user *parent_tidptr, *child_tidptr;

	clone_flags = i386_ebx(regs);
	newsp = i386_ecx(regs);
	parent_tidptr = (int __user *)i386_edx(regs);
	child_tidptr = (int __user *)i386_edi(regs);
	if (!newsp)
		newsp = i386_esp(regs);

	current_thread_info()->request.op = OP_FORK;

	return do_fork(clone_flags, newsp, regs, 0, parent_tidptr, child_tidptr);
}

/*
 * sys_ipc() is the de-multiplexer for the SysV IPC calls..
 *
 * This is really horribly ugly.
 */
asmlinkage int sys_ipc (uint call, int first, int second,
			int third, void __user *ptr, long fifth)
{
	int version, ret;

	version = call >> 16; /* hack for backward compatibility */
	call &= 0xffff;

	switch (call) {
	case SEMOP:
		return sys_semtimedop (first, (struct sembuf __user *)ptr, second, NULL);
	case SEMTIMEDOP:
		return sys_semtimedop(first, (struct sembuf __user *)ptr, second,
					(const struct timespec __user *)fifth);

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
		return sys_msgsnd (first, (struct msgbuf __user *) ptr, 
				   second, third);
	case MSGRCV:
		switch (version) {
		case 0: {
			struct ipc_kludge tmp;
			if (!ptr)
				return -EINVAL;
			
			if (copy_from_user(&tmp,
					   (struct ipc_kludge __user *) ptr, 
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
		return sys_msgctl (first, second, (struct msqid_ds __user *) ptr);

	case SHMAT:
		switch (version) {
		default: {
			ulong raddr;
			ret = do_shmat (first, (char __user *) ptr, second, &raddr);
			if (ret)
				return ret;
			return put_user (raddr, (ulong __user *) third);
		}
		case 1:	/* iBCS2 emulator entry point */
			if (!segment_eq(get_fs(), get_ds()))
				return -EINVAL;
			/* The "(ulong *) third" is valid _only_ because of the kernel segment thing */
			return do_shmat (first, (char __user *) ptr, second, (ulong *) third);
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

/*
 * elf_gregset_t layout for i386:
 *
 * 0: %ebx
 * 1: %ecx
 * 2: %edx
 * 3: %esi
 * 4: %edi
 * 5: %ebp
 * 6: %eax
 * 7: %ds
 * 8: %es
 * 9: %fs
 * 10: %gs
 * 11: orig_eax
 * 12: %eip
 * 13: %cs
 * 14: %efl
 * 15: %esp
 * 16: %ss
 *
 * See include/asm-i386/elf.h in ELF_CORE_COPY_REGS() to find the right
 * offset.  
 *
 * XXX
 *
 * Some of these aren't provided by L4.  In this case we just
 * zero it out.
 * What is orig_eax?  What do we do about that?
 */
void
i386_elf_core_do_copy_regs(L4_ThreadId_t tid, elf_gregset_t *elfregs)
{
	L4_Copy_regs_to_mrs(current_thread_info()->user_tid);

	L4_StoreMR(6, &(*elfregs)[0/*ebx*/]);
	L4_StoreMR(8, &(*elfregs)[1/*ecx*/]);
	L4_StoreMR(7, &(*elfregs)[2/*edx*/]);
	L4_StoreMR(3, &(*elfregs)[3/*esi*/]);
	L4_StoreMR(2, &(*elfregs)[4/*edi*/]);
	L4_StoreMR(4, &(*elfregs)[5/*ebp*/]);
	L4_StoreMR(9, &(*elfregs)[6/*eax*/]);
	*elfregs[7/*ds*/] = 0;
	*elfregs[8/*es*/] = 0;
	*elfregs[9/*fs*/] = 0;
	*elfregs[10/*gs*/] = 0;
	*elfregs[11/*orig_eax*/] = 0;
	L4_StoreMR(0, &(*elfregs)[12/*eip*/]);
	*elfregs[13/*cs*/] = 0;
	L4_StoreMR(1, &(*elfregs)[14/*efl*/]);
	L4_StoreMR(5, &(*elfregs)[15/*esp*/]);
	*elfregs[16/*ss*/] = 0;
}

int
i386_elf_core_copy_regs_task(struct task_struct *t, elf_gregset_t *elfregs)
{
	i386_elf_core_do_copy_regs(task_thread_info(t)->user_tid, elfregs);
	return 1;
}

void
i386_elf_core_copy_regs(elf_gregset_t *elfregs, struct pt_regs *regs)
{
	i386_elf_core_do_copy_regs(current_thread_info()->user_tid, elfregs);
}
