/*
 *  linux/include/asm-arm/ptrace.h
 *
 *  Copyright (C) 1996-2003 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __ASM_I386_PTRACE_H
#define __ASM_I386_PTRACE_H



#ifndef __ASSEMBLY__

#include "l4.h"
#include <asm/signal.h>
#include <asm/regs.h>

#define i386_eip(regs)		L4_MsgWord(&regs->msg, 0)
#define i386_eflags(regs)	L4_MsgWord(&regs->msg, 1)
#define i386_edi(regs)		L4_MsgWord(&regs->msg, 4)
#define i386_esi(regs)		L4_MsgWord(&regs->msg, 5)
#define i386_ebp(regs)		L4_MsgWord(&regs->msg, 6)
#define i386_esp(regs)		L4_MsgWord(&regs->msg, 7)
#define i386_ebx(regs)		L4_MsgWord(&regs->msg, 8)
#define i386_edx(regs)		L4_MsgWord(&regs->msg, 9)
#define i386_ecx(regs)		L4_MsgWord(&regs->msg, 10)
#define i386_eax(regs)		L4_MsgWord(&regs->msg, 11)
#define i386_save(regs)		L4_MsgWord(&regs->msg, 12)
#define i386_trap_no(regs)	L4_MsgWord(&regs->msg, 2)
#define i386_error_code(regs)	L4_MsgWord(&regs->msg, 3)

#define i386_put_eip(regs, val)		L4_MsgPutWord(&regs->msg, 0, val)
#define i386_put_eflags(regs, val)	L4_MsgPutWord(&regs->msg, 1, val)
#define i386_put_edi(regs, val)		L4_MsgPutWord(&regs->msg, 4, val)
#define i386_put_esi(regs, val)		L4_MsgPutWord(&regs->msg, 5, val)
#define i386_put_ebp(regs, val)		L4_MsgPutWord(&regs->msg, 6, val)
#define i386_put_esp(regs, val)		L4_MsgPutWord(&regs->msg, 7, val)
#define i386_put_ebx(regs, val)		L4_MsgPutWord(&regs->msg, 8, val)
#define i386_put_edx(regs, val)		L4_MsgPutWord(&regs->msg, 9, val)
#define i386_put_ecx(regs, val)		L4_MsgPutWord(&regs->msg, 10, val)
#define i386_put_eax(regs, val)		L4_MsgPutWord(&regs->msg, 11, val)
#define i386_put_save(regs, val)	L4_MsgPutWord(&regs->msg, 12, val)

#define ARCH_sp(regs)		i386_esp(regs)
#define ARCH_pc(regs)		i386_eip(regs)
#define ARCH_put_sp(regs, val)	i386_put_esp(regs, val)
#define ARCH_put_pc(regs, val)	i386_put_eip(regs, val)

#define PTRACE_GETREGS			12
#define PTRACE_SETREGS			13
#define PTRACE_GETFPREGS		14
#define PTRACE_SETFPREGS		15
#define PTRACE_GETFPXREGS		18
#define PTRACE_SETFPXREGS		19

#define PTRACE_OLDSETOPTIONS		21

#define PTRACE_GET_THREAD_AREA		25
#define PTRACE_SET_THREAD_AREA		26

extern inline void
l4_arch_setup_restart(struct pt_regs *regs)
{
	i386_put_eax(regs, i386_save(regs));
	i386_put_eflags(regs, i386_eflags(regs) | 0x80000000ul);	// Syscall must be restarted
}

#define __NR_restart_syscall	0

extern inline void
l4_arch_setup_sys_restart(struct pt_regs *regs)
{
	i386_put_eax(regs, __NR_restart_syscall);
	i386_put_eflags(regs, i386_eflags(regs) | 0x80000000ul);	// Syscall must be restarted
}


/*#define pc_pointer(v) \
	((v) & ~PCMASK)
*/
#define instruction_pointer(regs)   i386_eip(regs)

#define profile_pc(regs) instruction_pointer(regs)

#ifdef __KERNEL__
extern void show_regs(struct pt_regs *);

#endif

#endif /* __ASSEMBLY__ */

#endif
