/* 
 * asm-l4/i386/syscalls.h
 *
 * Copyright 2004 National ICT Australia.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */

#ifndef _L4_I386_SYSCALLS_H_
#define _L4_I386_SYSCALLS_H_

#include <asm/errno.h>
#include <asm/ptrace.h>
#include <asm/uaccess.h>

#define L4_RET_TYPE_FORK	L4_RET_TYPE_INT
#define L4_RET_TYPE_EXEC	L4_RET_TYPE_INT

#define IA32_EXC_GENERAL_PROTECTION	13
#define IS_IA32_GP			(L4_MsgWord (&current_regs()->msg, 2) == IA32_EXC_GENERAL_PROTECTION)

#ifndef CONFIG_IA32_VDSO_ENABLE

# define L4_ARCH_IS_SYSCALL(tag)	((L4_UntypedWords(tag) == 12) && IS_IA32_GP)
# define L4_ARCH_IS_EXCEPTION(tag)	((L4_UntypedWords(tag) == 12) && !IS_IA32_GP)

#else /* ifdef CONFIG_IA32_VDSO_ENABLE */

# define IA32_EXC_SYSENTER              130
# define IS_IA32_SYSENTER         	(L4_MsgWord (&current_regs()->msg, 2) == IA32_EXC_SYSENTER)
# define L4_ARCH_IS_SYSCALL(tag)	((L4_UntypedWords(tag) == 12) && (IS_IA32_GP || IS_IA32_SYSENTER))
# define L4_ARCH_IS_EXCEPTION(tag)	((L4_UntypedWords(tag) == 12) && !(IS_IA32_GP || IS_IA32_SYSENTER))
# define L4_I386_IS_SYSCALL_FAST(tag)   ((L4_UntypedWords(tag) == 12) && (IS_IA32_SYSENTER))

#endif /* ifndef CONFIG_IA32_VDSO_ENABLE */

typedef struct {
	unsigned short sys_num;		/* L4Linux syscall number */
} l4_i386_abi_syscalls_t;

extern unsigned int l4_i386_abi_syscall_start;
extern unsigned int l4_i386_abi_syscall_end;
extern l4_i386_abi_syscalls_t l4_i386_abi_syscalls[];

#ifdef CONFIG_IA32_VDSO_ENABLE

extern inline void
l4_i386_get_fast_syscall_args(struct pt_regs *regs)
{
    /* TODO: ensure sp is valid */

    /* Obtain the user sp, in ebp */
    L4_Word_t arg_edi, arg_ebp;
    L4_Word_t* usp = (L4_Word_t*)i386_ebp(regs);
        
    /* Copy edi arg from user stack. It's normally passed in edi, but
     * return ip is passed in edi and edi arg is pushed. */
    copy_from_user(&arg_edi, usp, sizeof(L4_Word_t));
    i386_put_edi(regs, arg_edi);
        
    /* Copy ebp arg (arg6) from user stack. It's normally passed in ebp, but
     * sp is passed in ebp and arg6 is pushed. */
    copy_from_user(&arg_ebp, usp + 1, sizeof(L4_Word_t));
    i386_put_ebp(regs, arg_ebp);
}

#endif /* ifdef CONFIG_IA32_VDSO_ENABLE */

extern inline long
l4_arch_lookup_syscall (struct pt_regs *regs, L4_Word_t *abi)
{
    L4_Word_t call;

    *abi = 0;

    call = i386_eax(regs);

    if ((call >= l4_i386_abi_syscall_start) &&
		    (call <= l4_i386_abi_syscall_end))
    {
	if (l4_i386_abi_syscalls[call - l4_i386_abi_syscall_start].sys_num == 0) {
		printk("syscall %ld\n", call);
		L4_KDB_Enter("");
	}
	return l4_i386_abi_syscalls[call - l4_i386_abi_syscall_start].sys_num;
    }

    return -1;
}

typedef L4_Word_t func_one_arg_t(L4_Word_t);
typedef L4_Word_t func_two_arg_t(L4_Word_t, L4_Word_t);
typedef L4_Word_t func_three_arg_t(L4_Word_t, L4_Word_t, L4_Word_t);
typedef L4_Word_t func_four_arg_t(L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t);
typedef L4_Word_t func_five_arg_t(L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t);
typedef L4_Word_t func_six_arg_t(L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t);

extern inline L4_Word_t
l4_arch_abi_call(struct pt_regs *regs, L4_Word_t sys_num, L4_Word_t abi)
{
    L4_Word_t result = -1ul;
    /* Initilized because gcc (wrongly) thinks they might be unitizialed... */
    L4_Word_t arg[7], nargs = l4_syscall_table[sys_num].args;

    switch (nargs)  {
	case 6: arg[5] = i386_ebp(regs);
	case 5: arg[4] = i386_edi(regs);
	case 4: arg[3] = i386_esi(regs);
	case 3: arg[2] = i386_edx(regs);
	case 2: arg[1] = i386_ecx(regs);
	case 1: arg[0] = i386_ebx(regs);
	default: break;
    }

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

    switch (l4_syscall_table[sys_num].ret_type) {
    case L4_RET_TYPE_INT:   result = (int)result; break;
    case L4_RET_TYPE_LONG:  break;
    default:		    break;
    }

    i386_put_eip(regs, i386_eip(regs)+2);

    /* save for syscall restarting */
    i386_put_save(regs, i386_eax(regs));

    i386_put_eax(regs, result);
    return result;
}

extern inline long
l4_arch_get_error(struct pt_regs *regs)
{
    return -(long)i386_eax(regs);
}

extern inline int
l4_arch_restart_syscall(struct pt_regs *regs)
{
    L4_Word_t temp;
    temp = i386_eflags(regs);

    if (temp & 0x80000000ul) {
	i386_put_eip(regs, i386_eip(regs)-2);
	i386_put_eflags(regs, i386_eflags(regs) & (~0x80000000ul));
	return 1;
    }

    return 0;
}

/* I386 specific syscalls */
#define	    __L4_old_mmap	__L4_sys_l4_last + 1
#define	    __L4_sys_waitpid	__L4_sys_l4_last + 2
#define	    __L4_sys_oldumount	__L4_sys_l4_last + 3
#define	    __L4_sys_stime	__L4_sys_l4_last + 4
#define	    __L4_old_select	__L4_sys_l4_last + 5
#define     __L4_sys_modify_ldt	__L4_sys_l4_last + 6
#define     __L4_sys_iopl	__L4_sys_l4_last + 7
#define     __L4_sys_ioperm	__L4_sys_l4_last + 8


#endif /* _L4_I386_SYSCALLS_H_ */
