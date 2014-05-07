/* 
 * asm-l4/arm/syscalls_inline.h
 *
 * Copyright 2004 National ICT Australia.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */

#ifndef _L4_ARM_SYSCALLS_INLINE_H_
#define _L4_ARM_SYSCALLS_INLINE_H_

#include INC_SYSTEM(unistd.h)

extern inline long
l4_arch_lookup_syscall (struct pt_regs *regs, L4_Word_t *abi)
{
	L4_Word_t call;

	*abi = 0;

	call = ARM_syscall(regs) & 0x00ffffff;

	if (likely( (call >= __NR_SYSCALL_BASE) &&
		    (call <= (__NR_SYSCALL_BASE + 352)) ))
	{
		call -= __NR_SYSCALL_BASE;

		if (unlikely( l4_arm_abi_syscalls[call].sys_num == 0 ))
			printk("unsupported arm syscall: %d\n", call + l4_arm_abi_syscall_start);

		return l4_arm_abi_syscalls[call].sys_num;
	} else if ((call >= __ARM_NR_BASE) &&
	    (call <= (__ARM_NR_BASE + 5))) {
#if 0
		printk("ARM syscall: %x\n", call);
#endif
		call -= __ARM_NR_BASE;
		if (unlikely(l4_arm_arm_syscalls[call].sys_num == 0))
			printk("Unsupported ARM syscall: %d\n", call + 
			    __ARM_NR_BASE);
			
		return l4_arm_arm_syscalls[call].sys_num;
	}

	printk("%d: unknown arm syscall: 0x%lx at 0x%lx\n", current->pid, call, ARM_pc(regs));
	return -1;
}

typedef L4_Word_t func_six_arg_t(L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t);

extern inline L4_Word_t
l4_arch_abi_call(struct pt_regs *regs, L4_Word_t sys_num, L4_Word_t abi)
{
	L4_Word_t result = -1ul;
	L4_Word_t arg0, arg1, arg2, arg3, arg4, arg5;
	l4_syscall_entry_t *syscall = &l4_syscall_table[sys_num];

	arg0 = ARM_r0(regs);
	arg1 = ARM_r1(regs);
	arg2 = ARM_r2(regs);
	arg3 = ARM_r3(regs);
	arg4 = ARM_r4(regs);
	arg5 = ARM_r5(regs);

	if (unlikely( syscall->flags & L4_SYS_FLAGS_NEED_REGS ))
	{
		L4_Word_t nargs = syscall->args;
		switch (nargs)  {
			case 5: arg5 = (L4_Word_t)regs; break;
			case 4: arg4 = (L4_Word_t)regs; break;
			case 3: arg3 = (L4_Word_t)regs; break;
			case 2: arg2 = (L4_Word_t)regs; break;
			case 1: arg1 = (L4_Word_t)regs; break;
			case 0: arg0 = (L4_Word_t)regs; break;
			default: break;
		}
	}

	result = ((func_six_arg_t *) syscall->fn)(
			arg0, arg1, arg2, arg3, arg4, arg5);

	switch (syscall->ret_type)
	{
		case L4_RET_TYPE_INT:   result = (int)result; break;
		case L4_RET_TYPE_LONG:  break;
		default: break;
	}

	/* save for syscall restarting */
	ARM_put_save(regs, ARM_r0(regs));

	ARM_put_r0(regs, result);

	return result;
}

extern inline int
l4_arch_restart_syscall(struct pt_regs *regs)
{
	L4_Word_t temp;
	temp = ARM_pc(regs);

	if (temp & 1) {
		ARM_put_pc(regs, temp & (~1ul));
		return 1;
	}

	return 0;
}

#endif /* _L4_ARM_SYSCALLS_INLINE_H_ */
