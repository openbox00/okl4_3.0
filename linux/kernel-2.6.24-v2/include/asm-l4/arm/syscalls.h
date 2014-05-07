/* 
 * asm-l4/arm/syscalls.h
 *
 * Copyright 2004 National ICT Australia.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */

#ifndef _L4_ARM_SYSCALLS_H_
#define _L4_ARM_SYSCALLS_H_

#include <asm/errno.h>
#include <asm/ptrace.h>
#include <asm/current.h>
#include <linux/sched.h>

#define L4_RET_TYPE_FORK	L4_RET_TYPE_INT
#define L4_RET_TYPE_EXEC	L4_RET_TYPE_INT

#define L4_ARCH_IS_SYSCALL(tag)		likely(L4_UntypedWords(tag) == 13)
#define L4_ARCH_IS_EXCEPTION(tag)	(L4_UntypedWords(tag) == 5)

typedef struct {
	unsigned short sys_num;		/* L4Linux syscall number */
} l4_arm_abi_syscalls_t;

extern unsigned int l4_arm_abi_syscall_start;
extern unsigned int l4_arm_abi_syscall_end;
extern l4_arm_abi_syscalls_t l4_arm_abi_syscalls[];
extern l4_arm_abi_syscalls_t l4_arm_arm_syscalls[];

extern inline long
l4_arch_get_error(struct pt_regs *regs)
{
	return -(long)ARM_r0(regs);
}

/* ARM specific syscalls */
#define	    __L4_old_mmap	__L4_sys_l4_last + 1
#define	    __L4_sys_oldumount	__L4_sys_l4_last + 2
#define	    __L4_sys_stime	__L4_sys_l4_last + 3
#define	    __L4_old_select	__L4_sys_l4_last + 4
#define     __L4_arm_set_tls	__L4_sys_l4_last + 5
#define     __L4_arm_breakpoint	__L4_sys_l4_last + 6

#endif /* _L4_ARM_SYSCALLS_H_ */
