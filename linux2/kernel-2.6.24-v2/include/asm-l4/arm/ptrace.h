/*
 *  linux/include/asm-arm/ptrace.h
 *
 *  Copyright (C) 1996-2003 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __ASM_ARM_PTRACE_H
#define __ASM_ARM_PTRACE_H

#include INC_SYSTEM(unistd.h)


#define PTRACE_GETREGS		12
#define PTRACE_SETREGS		13
#define PTRACE_GETFPREGS	14
#define PTRACE_SETFPREGS	15

#define PTRACE_OLDSETOPTIONS	21

#define PTRACE_GET_THREAD_AREA	22

/*
 * PSR bits
 */
#define USR26_MODE	0x00000000
#define FIQ26_MODE	0x00000001
#define IRQ26_MODE	0x00000002
#define SVC26_MODE	0x00000003
#define USR_MODE	0x00000010
#define FIQ_MODE	0x00000011
#define IRQ_MODE	0x00000012
#define SVC_MODE	0x00000013
#define ABT_MODE	0x00000017
#define UND_MODE	0x0000001b
#define SYSTEM_MODE	0x0000001f
#define MODE32_BIT	0x00000010
#define MODE_MASK	0x0000001f
#define PSR_T_BIT	0x00000020
#define PSR_F_BIT	0x00000040
#define PSR_I_BIT	0x00000080
#define PSR_J_BIT	0x01000000
#define PSR_Q_BIT	0x08000000
#define PSR_V_BIT	0x10000000
#define PSR_C_BIT	0x20000000
#define PSR_Z_BIT	0x40000000
#define PSR_N_BIT	0x80000000
#define PCMASK		0

/*
 * Groups of PSR bits
 */
#define PSR_f		0xff000000	/* Flags		*/
#define PSR_s		0x00ff0000	/* Status		*/
#define PSR_x		0x0000ff00	/* Extension		*/
#define PSR_c		0x000000ff	/* Control		*/

#ifndef __ASSEMBLY__

#include "l4.h"
#include <asm/signal.h>
#include <asm/regs.h>

#define PT_FLAGS_RESTART	(1<<0)

#define ARM_r0(regs)	 L4_MsgWord(&regs->msg, 4)
#define ARM_r1(regs)	 L4_MsgWord(&regs->msg, 5)
#define ARM_r2(regs)	 L4_MsgWord(&regs->msg, 6)
#define ARM_r3(regs)	 L4_MsgWord(&regs->msg, 7)
#define ARM_r4(regs)	 L4_MsgWord(&regs->msg, 0)
#define ARM_r5(regs)	 L4_MsgWord(&regs->msg, 1)
#define ARM_r6(regs)	 L4_MsgWord(&regs->msg, 2)
#define ARM_r7(regs)	 L4_MsgWord(&regs->msg, 3)
#define ARM_pc(regs)	 L4_MsgWord(&regs->msg, 8)
#define ARM_sp(regs)	 L4_MsgWord(&regs->msg, 9)
#define ARM_lr(regs)	 L4_MsgWord(&regs->msg, 10)
#if defined(CONFIG_AEABI)
#define ARM_syscall(regs)	ARM_r7(regs)
#else
#define ARM_syscall(regs)	 L4_MsgWord(&regs->msg, 11)
#endif
#define ARM_cpsr(regs)	 L4_MsgWord(&regs->msg, 12)
#define ARM_save(regs)	 L4_MsgWord(&regs->msg, 13)

#define ARM_r(n, regs)	 L4_MsgWord(&regs->msg, 4+(n))
#define ARM_put_r(n, regs, val)	 L4_MsgPutWord(&regs->msg, 4+(n), val)

#define ARM_put_r0(regs, val)	 L4_MsgPutWord(&regs->msg, 4, val)
#define ARM_put_r1(regs, val)	 L4_MsgPutWord(&regs->msg, 5, val)
#define ARM_put_r2(regs, val)	 L4_MsgPutWord(&regs->msg, 6, val)
#define ARM_put_r3(regs, val)	 L4_MsgPutWord(&regs->msg, 7, val)
#define ARM_put_r4(regs, val)	 L4_MsgPutWord(&regs->msg, 0, val)
#define ARM_put_r5(regs, val)	 L4_MsgPutWord(&regs->msg, 1, val)
#define ARM_put_r6(regs, val)	 L4_MsgPutWord(&regs->msg, 2, val)
#define ARM_put_r7(regs, val)	 L4_MsgPutWord(&regs->msg, 3, val)
#define ARM_put_pc(regs, val)	 L4_MsgPutWord(&regs->msg, 8, val)
#define ARM_put_sp(regs, val)	 L4_MsgPutWord(&regs->msg, 9, val)
#define ARM_put_lr(regs, val)	 L4_MsgPutWord(&regs->msg, 10, val)
#if defined(CONFIG_AEABI)
#define ARM_put_syscall(regs, val)	ARM_put_r7(regs, val)
#else
#define ARM_put_syscall(regs, val) L4_MsgPutWord(&regs->msg, 11, val)
#endif
#define ARM_put_cpsr(regs, val)	 L4_MsgPutWord(&regs->msg, 12, val)
#define ARM_put_save(regs, val)	 L4_MsgPutWord(&regs->msg, 13, val)

#define ARCH_sp(regs)			ARM_sp(regs)
#define ARCH_pc(regs)			ARM_pc(regs)
#define ARCH_put_sp(regs, val)	ARM_put_sp(regs, val)
#define ARCH_put_pc(regs, val)	ARM_put_pc(regs, val)

/* XXX */
#define ARM_ip(regs)				regs->ip
#define ARM_put_ip(regs, val)		regs->ip = val
#define ARM_set_strace(regs, val)	regs->strace_flag = val

#define ARM_EXCEPT_pc(regs)			L4_MsgWord(&regs->msg, 0)
#define ARM_EXCEPT_sp(regs)			L4_MsgWord(&regs->msg, 1)
#define ARM_EXCEPT_flags(regs)		L4_MsgWord(&regs->msg, 2)
#define ARM_EXCEPT_number(regs)		L4_MsgWord(&regs->msg, 3)
#define ARM_EXCEPT_error_code(regs)	L4_MsgWord(&regs->msg, 4)
#define ARM_EXCEPT_local_id(regs)	L4_MsgWord(&regs->msg, 5)

extern inline void
l4_arch_setup_restart(struct pt_regs *regs)
{
	ARM_put_r0(regs, ARM_save(regs));
	ARM_put_pc(regs, ARM_pc(regs) | 1);
}

extern inline void
l4_arch_setup_sys_restart(struct pt_regs *regs)
{
	/* swi __restart_syscall */
	ARM_put_syscall(regs, __NR_restart_syscall);
	ARM_put_pc(regs, ARM_pc(regs) | 1);
}


#ifdef __KERNEL__

#ifdef CONFIG_ARM_THUMB
#define thumb_mode(regs) \
	(((regs)->ARM_cpsr & PSR_T_BIT))
#else
#define thumb_mode(regs) (0)
#endif

#define processor_mode(regs) \
	((regs)->ARM_cpsr & MODE_MASK)

#define interrupts_enabled(regs) \
	(!((regs)->ARM_cpsr & PSR_I_BIT))

#define fast_interrupts_enabled(regs) \
	(!((regs)->ARM_cpsr & PSR_F_BIT))

#define condition_codes(regs) \
	((regs)->ARM_cpsr & (PSR_V_BIT|PSR_C_BIT|PSR_Z_BIT|PSR_N_BIT))
	
#endif	/* __KERNEL__ */

#define pc_pointer(v) \
    ((v) & ~PCMASK)

#define instruction_pointer(regs)   ARM_pc(regs)

#define profile_pc(regs) instruction_pointer(regs)

#ifdef __KERNEL__
extern void show_regs(struct pt_regs *);

#define predicate(x)	(x & 0xf0000000)
#define PREDICATE_ALWAYS	0xe0000000

#endif

#endif /* __ASSEMBLY__ */

#endif

