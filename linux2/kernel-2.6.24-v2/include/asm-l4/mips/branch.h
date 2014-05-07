/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1996, 1997, 1998, 2001 by Ralf Baechle
 */
#ifndef _ASM_L4_BRANCH_H
#define _ASM_L4_BRANCH_H

#include <asm/ptrace.h>

static inline int delay_slot(struct pt_regs *regs)
{
	return MIPS_status(regs) & L4_MIPS_STATUS_BRANCH;
}

static inline unsigned long exception_epc(struct pt_regs *regs)
{
	if (!delay_slot(regs))
		return MIPS_EXCEPT_pc(regs);

	return MIPS_EXCEPT_pc(regs) + 4;
}

extern int __compute_return_epc(struct pt_regs *regs);

static inline int compute_return_epc(struct pt_regs *regs)
{
	if (!delay_slot(regs)) {
		MIPS_EXCEPT_put_pc(regs, MIPS_EXCEPT_pc(regs) + 4);
		return 0;
	}

	BUG();
//	return __compute_return_epc(regs);
}

#endif /* _ASM_L4_BRANCH_H */
