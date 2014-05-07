/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994, 95, 96, 97, 98, 99, 2000 by Ralf Baechle
 * Copyright (C) 1999, 2000 Silicon Graphics, Inc.
 */
#ifndef _L4_MIPS_PTRACE_H
#define _L4_MIPS_PTRACE_H



/* 0 - 31 are integer registers, 32 - 63 are fp registers.  */
#define FPR_BASE	32
#define PC		64
#define CAUSE		65
#define BADVADDR	66
#define MMHI		67
#define MMLO		68
#define FPC_CSR		69
#define FPC_EIR		70

#ifndef __ASSEMBLY__

#include "l4.h"
#include <asm/signal.h>
#include <asm/regs.h>

#define PT_FLAGS_RESTART	(1<<0)

#define MIPS_pc(regs)		L4_MsgWord(&regs->msg, 0)
#define MIPS_sp(regs)		L4_MsgWord(&regs->msg, 1)
#define MIPS_status(regs)	L4_MsgWord(&regs->msg, 2)
#define MIPS_v0(regs)		L4_MsgWord(&regs->msg, 3)
#define MIPS_v1(regs)		L4_MsgWord(&regs->msg, 4)
#define MIPS_a0(regs)		L4_MsgWord(&regs->msg, 5)
#define MIPS_a1(regs)		L4_MsgWord(&regs->msg, 6)
#define MIPS_a2(regs)		L4_MsgWord(&regs->msg, 7)
#define MIPS_a3(regs)		L4_MsgWord(&regs->msg, 8)
#define MIPS_a4(regs)		L4_MsgWord(&regs->msg, 9)
#define MIPS_a5(regs)		L4_MsgWord(&regs->msg, 10)
#define MIPS_a6(regs)		L4_MsgWord(&regs->msg, 11)
#define MIPS_a7(regs)		L4_MsgWord(&regs->msg, 12)
#define MIPS_save_a3(regs)	L4_MsgWord(&regs->msg, 13)
#define MIPS_save_v0(regs)	L4_MsgWord(&regs->msg, 14)

#define MIPS_put_v0(regs, val)		L4_MsgPutWord(&regs->msg, 3, val)
#define MIPS_put_v1(regs, val)		L4_MsgPutWord(&regs->msg, 4, val)
#define MIPS_put_a3(regs, val)		L4_MsgPutWord(&regs->msg, 8, val)
#define MIPS_put_pc(regs, val)		L4_MsgPutWord(&regs->msg, 0, val)
#define MIPS_put_sp(regs, val)		L4_MsgPutWord(&regs->msg, 1, val)
#define MIPS_put_save_a3(regs, val)	L4_MsgPutWord(&regs->msg, 13, val)
#define MIPS_put_save_v0(regs, val)	L4_MsgPutWord(&regs->msg, 14, val)

#define L4_MIPS_STATUS_BRANCH	1

#define MIPS_EXCEPT_pc(regs)		L4_MsgWord(&regs->msg, 0)
#define MIPS_EXCEPT_sp(regs)		L4_MsgWord(&regs->msg, 1)
#define MIPS_EXCEPT_status(regs)	L4_MsgWord(&regs->msg, 2)
#define MIPS_EXCEPT_number(regs)	L4_MsgWord(&regs->msg, 3)
#define MIPS_EXCEPT_error_code(regs)	L4_MsgWord(&regs->msg, 4)
#define MIPS_EXCEPT_local_id(regs)	L4_MsgWord(&regs->msg, 5)

#define MIPS_EXCEPT_put_pc(regs, val)	L4_MsgPutWord(&regs->msg, 0, val)
#define MIPS_EXCEPT_put_sp(regs, val)	L4_MsgPutWord(&regs->msg, 1, val)

#define MIPS_EXCEPT_BadVaddr(regs)	MIPS_EXCEPT_error_code(regs)

#define ARCH_sp(regs)		MIPS_sp(regs)
#define ARCH_pc(regs)		MIPS_pc(regs)
#define ARCH_put_sp(regs, val)	MIPS_put_sp(regs, val)
#define ARCH_put_pc(regs, val)	MIPS_put_pc(regs, val)

extern inline void
l4_arch_setup_restart(struct pt_regs *regs)
{
	MIPS_put_a3(regs, MIPS_save_a3(regs));
	MIPS_put_v0(regs, MIPS_save_v0(regs));

	MIPS_put_pc(regs, MIPS_pc(regs) | 1);
}

extern inline void
l4_arch_setup_sys_restart(struct pt_regs *regs)
{
	MIPS_put_v0(regs, 5213);    /* __NR_restart_syscall */
	MIPS_put_pc(regs, MIPS_pc(regs) | 1);
}

#endif /* !__ASSEMBLY__ */

/* Arbitrarily choose the same ptrace numbers as used by the Sparc code. */
/* #define PTRACE_GETREGS		12 */
/* #define PTRACE_SETREGS		13 */
/* #define PTRACE_GETFPREGS		14 */
/* #define PTRACE_SETFPREGS		15 */
/* #define PTRACE_GETFPXREGS		18 */
/* #define PTRACE_SETFPXREGS		19 */

#define PTRACE_OLDSETOPTIONS	21

#define PTRACE_GET_THREAD_AREA	25
#define PTRACE_SET_THREAD_AREA	26

#ifdef __ASSEMBLY__
#include <asm/offset.h>
#endif

#ifdef __KERNEL__

#ifndef __ASSEMBLY__

#define instruction_pointer(regs)   MIPS_pc(regs)

#define profile_pc(regs) instruction_pointer(regs)

extern void show_regs(struct pt_regs *);
#endif /* !__ASSEMBLY__ */

#endif

#endif /* _L4_MIPS_PTRACE_H */
