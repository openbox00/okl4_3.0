/* 
 * asm-l4/mips/syscalls.h
 *
 * Copyright 2004 National ICT Australia.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */

#ifndef _L4_MIPS_SYSCALLS_H_
#define _L4_MIPS_SYSCALLS_H_

#define L4_RET_TYPE_FORK	L4_RET_TYPE_INT
#define L4_RET_TYPE_EXEC	L4_RET_TYPE_INT

#define L4_ARCH_IS_SYSCALL(tag)		(L4_UntypedWords(tag) == 13)
#define L4_ARCH_IS_EXCEPTION(tag)	(L4_UntypedWords(tag) == 6)

#include INC_SYSTEM(sgidefs.h)
#include <asm/errno.h>
#include <asm/ptrace.h>
#include <asm/uaccess.h>

typedef struct {
	unsigned short sys_num;		/* L4Linux syscall number */
} l4_mips_abi_syscalls_t;

typedef struct {
	l4_mips_abi_syscalls_t *map;
	unsigned short abi_start;	/* Base offset of ABI calls */
	unsigned short abi_last;	/* Last ABI syscall number */
	unsigned short abi;		/* This abi number */
	unsigned short __x;
} l4_mips_abi_t;

/* List of mips ABIs */
extern l4_mips_abi_t	    *l4_mips_abi_list[];

extern inline long
l4_arch_lookup_syscall (struct pt_regs *regs, L4_Word_t *abi)
{
	L4_Word_t call;
	L4_Word_t i;

	call = MIPS_v0(regs);

	/* Find the syscall in all the ABIs */
	for (i = 0; l4_mips_abi_list[i]; i++)
	{
		if (l4_mips_abi_list[i]->map) {
			if ( (call >= l4_mips_abi_list[i]->abi_start) &&
			     (call <= l4_mips_abi_list[i]->abi_last) )
			{
				call = call - l4_mips_abi_list[i]->abi_start;

				*abi = l4_mips_abi_list[i]->abi;
				if (l4_mips_abi_list[i]->map[call].sys_num == 0)
					printk("unknown mips %ld\n", call + l4_mips_abi_list[i]->abi_start);

				return l4_mips_abi_list[i]->map[call].sys_num;
			}
		}
	}

	printk("unknown mips %ld\n", call);
	return -1;
}

typedef L4_Word_t func_one_arg_t(L4_Word_t);
typedef L4_Word_t func_two_arg_t(L4_Word_t, L4_Word_t);
typedef L4_Word_t func_three_arg_t(L4_Word_t, L4_Word_t, L4_Word_t);
typedef L4_Word_t func_four_arg_t(L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t);
typedef L4_Word_t func_five_arg_t(L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t);
typedef L4_Word_t func_six_arg_t(L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t);

int printk(const char * fmt, ...);
extern inline L4_Word_t
l4_arch_abi_call(struct pt_regs *regs, L4_Word_t sys_num, L4_Word_t abi)
{
    L4_Word_t result = -1ul;
    L4_Word_t pc;
    L4_Word_t arg[7], nargs = l4_syscall_table[sys_num].args;

    switch (abi) {
    case _MIPS_SIM_ABI64:
	switch (nargs)  {
	    case 6: arg[5] = MIPS_a5(regs);
	    case 5: arg[4] = MIPS_a4(regs);
	    case 4: arg[3] = MIPS_a3(regs);
	    case 3: arg[2] = MIPS_a2(regs);
	    case 2: arg[1] = MIPS_a1(regs);
	    case 1: arg[0] = MIPS_a0(regs);
	    default: break;
	}
	break;
    case _MIPS_SIM_ABI32:
	if (nargs >= 5)
	{
	    if (MIPS_sp(regs) & 3) {
		assert(!"bad stack");
	    }
	}
	switch (nargs)  {
	    case 6: if (__get_user(arg[5], (s32 *)(MIPS_sp(regs)+20))) result = -EFAULT;
	    case 5: if (__get_user(arg[4], (s32 *)(MIPS_sp(regs)+16))) result = -EFAULT;
	    case 4: arg[3] = (s32)MIPS_a3(regs);
	    case 3: arg[2] = (s32)MIPS_a2(regs);
	    case 2: arg[1] = (s32)MIPS_a1(regs);
	    case 1: arg[0] = (s32)MIPS_a0(regs);
	    default: break;
	}
	break;
    default:
	assert(!"Invalid/unimplemented ABI");
    }

    if (result != -EFAULT)
    {
	if (l4_syscall_table[sys_num].flags & L4_SYS_FLAGS_NEED_REGS)
	{
	    arg[nargs] = (L4_Word_t)regs;
	    nargs++;
	}

	switch (nargs)  {
	    case 0: result = l4_syscall_table[sys_num].fn(); break;
	    case 1: result = ((func_one_arg_t *)
				l4_syscall_table[sys_num].fn)(arg[0]);
		break;
	    case 2: result = ((func_two_arg_t *)
				l4_syscall_table[sys_num].fn)(arg[0], arg[1]);
		break;
	    case 3: result = ((func_three_arg_t *)
				l4_syscall_table[sys_num].fn)(arg[0], arg[1], arg[2]);
		break;
	    case 4: result = ((func_four_arg_t *)
				l4_syscall_table[sys_num].fn)(arg[0], arg[1], arg[2], arg[3]);
		break;
	    case 5: result = ((func_five_arg_t *)
				l4_syscall_table[sys_num].fn)(arg[0], arg[1], arg[2], arg[3], arg[4]);
		break;
	    case 6: result = ((func_six_arg_t *)
				l4_syscall_table[sys_num].fn)(arg[0], arg[1], arg[2], arg[3], arg[4], arg[5]);
		break;
	    default: assert(!"unsupported argument no"); break;
	}
    }

    pc = MIPS_pc(regs);
    pc += 4;

    switch (l4_syscall_table[sys_num].ret_type) {
    case L4_RET_TYPE_INT:   result = (int)result; break;
    case L4_RET_TYPE_LONG:  break;
    default:		    break;
    }

    /* save registers for syscall restarting */
    MIPS_put_save_a3(regs, MIPS_a3(regs));
    MIPS_put_save_v0(regs, MIPS_v0(regs));

    /* Was there an error? */
    if (((long)result < 0) && ((long)result >= -EMAXERRNO))
    {
	MIPS_put_a3(regs, 1);		/* Put 1 in a3 */
	result = -(long)result;
    } else
    {
	MIPS_put_a3(regs, 0);		/* Put 0 in a3 */
    }

    MIPS_put_pc(regs, pc);
    MIPS_put_v0(regs, result);

    return result;
}

extern inline long
l4_arch_get_error(struct pt_regs *regs)
{
    if (MIPS_a3(regs) == 1)
    {
	return (long)MIPS_v0(regs);
    }
    return 0;
}

extern inline int
l4_arch_restart_syscall(struct pt_regs *regs)
{
    L4_Word_t temp;
    temp = MIPS_pc(regs);

    if (temp & 1) {
    	MIPS_put_pc(regs, temp & (~1ul));
	return 1;
    }

    return 0;
}

/* MIPS specific syscalls */
//#define	    __L4_old_mmap	__L4_sys_l4_last + 0

#ifdef CONFIG_COMPAT

#define	    __L4_sys_stime		__L4_sys_l4_last + 1

#define	    __L4_sys_bind		__L4_sys_l4_last + 2
#define	    __L4_sys_connect		__L4_sys_l4_last + 3
#define	    __L4_sys_listen		__L4_sys_l4_last + 4
#define	    __L4_sys_accept		__L4_sys_l4_last + 5
#define	    __L4_sys_getsockname	__L4_sys_l4_last + 6
#define	    __L4_sys_getpeername	__L4_sys_l4_last + 7
#define	    __L4_sys_socketpair		__L4_sys_l4_last + 8
#define	    __L4_sys_send		__L4_sys_l4_last + 9
#define	    __L4_sys_sendto		__L4_sys_l4_last + 10
#define	    __L4_sys_recv		__L4_sys_l4_last + 11
#define	    __L4_sys_recvfrom		__L4_sys_l4_last + 12
#define	    __L4_sys_shutdown		__L4_sys_l4_last + 13
#define	    __L4_sys_setsockopt		__L4_sys_l4_last + 14
#define	    __L4_sys_getsockopt		__L4_sys_l4_last + 15
#define	    __L4_sys_sendmsg		__L4_sys_l4_last + 16
#define	    __L4_sys_recvmsg		__L4_sys_l4_last + 17

#define	    __L4_mips_compat		__L4_sys_l4_last + 17

#define	    __L4_sys32_newuname		__L4_mips_compat + 1
#define	    __L4_compat_sys_fcntl64	__L4_mips_compat + 2

#define	    __L4_sys32_sigreturn	__L4_mips_compat + 3
#define	    __L4_sys32_sigaction	__L4_mips_compat + 4
#define	    __L4_sys32_sigprocmask	__L4_mips_compat + 5
#define	    __L4_sys32_sigpending	__L4_mips_compat + 6
#define	    __L4_sys32_sigsuspend	__L4_mips_compat + 7

#define	    __L4_sys32_rt_sigreturn	__L4_mips_compat + 8
#define	    __L4_sys32_rt_sigaction	__L4_mips_compat + 9
#define	    __L4_sys32_rt_sigprocmask	__L4_mips_compat + 10
#define	    __L4_sys32_rt_sigpending	__L4_mips_compat + 11
#define	    __L4_sys32_rt_sigtimedwait	__L4_mips_compat + 12
#define	    __L4_sys32_rt_sigqueueinfo	__L4_mips_compat + 13
#define	    __L4_sys32_rt_sigsuspend	__L4_mips_compat + 14

#define	    __L4_sys32_waitpid		__L4_mips_compat + 15
#define	    __L4_compat_sys_wait4	__L4_mips_compat + 16

#define	    __L4_compat_sys_ioctl	__L4_mips_compat + 17
#define	    __L4_compat_sys_fcntl	__L4_mips_compat + 18
#define	    __L4_sys_newstat		__L4_mips_compat + 19
#define	    __L4_sys_newlstat		__L4_mips_compat + 20
#define	    __L4_sys_newfstat		__L4_mips_compat + 21
#define	    __L4_compat_sys_newstat	__L4_mips_compat + 22
#define	    __L4_compat_sys_newlstat	__L4_mips_compat + 23
#define	    __L4_compat_sys_newfstat	__L4_mips_compat + 24

#define	    __L4_sys32_sysinfo		__L4_mips_compat + 25

#define	    __L4_sys32_execve		__L4_mips_compat + 26

#define	    __L4_compat_sys_nanosleep	__L4_mips_compat + 27

#define	    __L4_old_mmap		__L4_mips_compat + 28
#define	    __L4_sys32_mmap2		__L4_mips_compat + 29

#define	    __L4_sys32_getdents		__L4_mips_compat + 30
#define	    __L4_sys_oldumount		__L4_sys_l4_last + 31
#define	    __L4_compat_sys_times	__L4_mips_compat + 32

#define	    __L4_compat_setsockopt	__L4_mips_compat + 33
#define	    __L4_compat_sendmsg		__L4_mips_compat + 34
#define	    __L4_compat_recvmsg		__L4_mips_compat + 35

#define	    __L4_compat_sys_readv	__L4_mips_compat + 36
#define	    __L4_compat_sys_writev	__L4_mips_compat + 37

#define	    __L4_compat_sys_statfs	__L4_mips_compat + 38
#define	    __L4_compat_sys_fstatfs	__L4_mips_compat + 39

#define	    __L4_sys32_gettimeofday	__L4_mips_compat + 40

#define	    __L4_compat_sys_getrusage	__L4_mips_compat + 41
#define	    __L4_compat_sys_getrlimit	__L4_mips_compat + 42
#define	    __L4_compat_sys_setrlimit	__L4_mips_compat + 43

#define	    __L4_sys32__llseek		__L4_mips_compat + 44
#define	    __L4_compat_sys_utime	__L4_mips_compat + 45

#define	    __L4_sys32_sched_rr_get_interval	__L4_mips_compat + 46

#define	    __L4_sys32_adjtimex		__L4_mips_compat + 47

#endif

#endif /* _L4_MIPS_SYSCALLS_H_ */
