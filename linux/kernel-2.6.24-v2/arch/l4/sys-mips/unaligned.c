/*
 * Handle unaligned accesses by emulation.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1996, 1998, 1999, 2002 by Ralf Baechle
 * Copyright (C) 1999 Silicon Graphics, Inc.
 *
 * This file contains exception handler for address error exception with the
 * special capability to execute faulting instructions in software.  The
 * handler does not try to handle the case when the program counter points
 * to an address not aligned to a word boundary.
 *
 * Putting data to unaligned addresses is a bad practice even on Intel where
 * only the performance is affected.  Much worse is that such code is non-
 * portable.  Due to several programs that die on MIPS due to alignment
 * problems I decided to implement this handler anyway though I originally
 * didn't intend to do this at all for user code.
 *
 * For now I enable fixing of address errors by default to make life easier.
 * I however intend to disable this somewhen in the future when the alignment
 * problems with user programs have been fixed.  For programmers this is the
 * right way to go.
 *
 * Fixing address errors is a per process option.  The option is inherited
 * across fork(2) and execve(2) calls.  If you really want to use the
 * option in your user programs - I discourage the use of the software
 * emulation strongly - use the following code in your userland stuff:
 *
 * #include <sys/sysmips.h>
 *
 * ...
 * sysmips(MIPS_FIXADE, x);
 * ...
 *
 * The argument x is 0 for disabling software emulation, enabled otherwise.
 *
 * Below a little program to play around with this feature.
 *
 * #include <stdio.h>
 * #include <sys/sysmips.h>
 *
 * struct foo {
 *         unsigned char bar[8];
 * };
 *
 * main(int argc, char *argv[])
 * {
 *         struct foo x = {0, 1, 2, 3, 4, 5, 6, 7};
 *         unsigned int *p = (unsigned int *) (x.bar + 3);
 *         int i;
 *
 *         if (argc > 1)
 *                 sysmips(MIPS_FIXADE, atoi(argv[1]));
 *
 *         printf("*p = %08lx\n", *p);
 *
 *         *p = 0xdeadface;
 *
 *         for(i = 0; i <= 7; i++)
 *         printf("%02x ", x.bar[i]);
 *         printf("\n");
 * }
 *
 * Coprocessor loads are not supported; I think this case is unimportant
 * in the practice.
 *
 * TODO: Handle ndc (attempted store to doubleword in uncached memory)
 *       exception for the R6000.
 *       A store crossing a page boundary might be executed only partially.
 *       Undo the partial store in this case.
 */

#include <asm/macros.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/signal.h>
#include <linux/smp.h>
#include <linux/smp_lock.h>

#include <asm/asm.h>
#include INC_SYSTEM2(branch.h)
#include <asm/byteorder.h>
#include INC_SYSTEM(inst.h)
#include <asm/uaccess.h>
#include <asm/system.h>

#define STR(x)  __STR(x)
#define __STR(x)  #x

#ifdef CONFIG_PROC_FS
unsigned long unaligned_instructions;
#endif

extern unsigned long  
parse_ptabs_read(unsigned long address, unsigned long *offset); 

#define USER_STATE_GET_STATE    (1)
#define USER_STATE_SET_STATE    (2)

#if 0
void write_reg(struct pt_regs *regs, UserState_t *state, int reg, unsigned long value)
{
	switch(reg) {
	case 1:	state->gpRegs.at = value; break;
	case 2: state->gpRegs.v0 = value; break;
	case 3: state->gpRegs.v1 = value; break;
	case 4: state->gpRegs.a0 = value; break;
	case 5: state->gpRegs.a1 = value; break;
	case 6: state->gpRegs.a2 = value; break;
	case 7: state->gpRegs.a3 = value; break;
	case 8: state->gpRegs.t0 = value; break;
	case 9: state->gpRegs.t1 = value; break;
	case 10: state->gpRegs.t2 = value; break;
	case 11: state->gpRegs.t3 = value; break;
	case 12: state->gpRegs.t4 = value; break;
	case 13: state->gpRegs.t5 = value; break;
	case 14: state->gpRegs.t6 = value; break;
	case 15: state->gpRegs.t7 = value; break;
	case 16: state->gpRegs.s0 = value; break;
	case 17: state->gpRegs.s1 = value; break;
	case 18: state->gpRegs.s2 = value; break;
	case 19: state->gpRegs.s3 = value; break;
	case 20: state->gpRegs.s4 = value; break;
	case 21: state->gpRegs.s5 = value; break;
	case 22: state->gpRegs.s6 = value; break;
	case 23: state->gpRegs.s7 = value; break;
	case 24: state->gpRegs.t8 = value; break;
	case 25: state->gpRegs.t9 = value; break;
	case 28: state->gpRegs.gp = value; break;
	case 29: MIPS_EXCEPT_put_sp(regs, value); break;
	case 30: state->gpRegs.s8 = value; break;
	case 31: state->gpRegs.ra = value; break;
	default: break;
	};
}

unsigned long get_reg(struct pt_regs *regs, UserState_t *state, int reg)
{
	switch(reg) {
	case 1:	return state->gpRegs.at;
	case 2: return state->gpRegs.v0;
	case 3: return state->gpRegs.v1;
	case 4: return state->gpRegs.a0;
	case 5: return state->gpRegs.a1;
	case 6: return state->gpRegs.a2;
	case 7: return state->gpRegs.a3;
	case 8: return state->gpRegs.t0;
	case 9: return state->gpRegs.t1;
	case 10: return state->gpRegs.t2;
	case 11: return state->gpRegs.t3;
	case 12: return state->gpRegs.t4;
	case 13: return state->gpRegs.t5;
	case 14: return state->gpRegs.t6;
	case 15: return state->gpRegs.t7;
	case 16: return state->gpRegs.s0;
	case 17: return state->gpRegs.s1;
	case 18: return state->gpRegs.s2;
	case 19: return state->gpRegs.s3;
	case 20: return state->gpRegs.s4;
	case 21: return state->gpRegs.s5;
	case 22: return state->gpRegs.s6;
	case 23: return state->gpRegs.s7;
	case 24: return state->gpRegs.t8;
	case 25: return state->gpRegs.t9;
	case 28: return state->gpRegs.gp;
	case 29: return MIPS_EXCEPT_sp(regs);
	case 30: return state->gpRegs.s8;
	case 31: return state->gpRegs.ra;
	default: return 0;
	};
}
#endif

#if 0
static inline int emulate_load_store_insn(struct pt_regs *regs,
	UserState_t *cur_state, void *addr, unsigned long pc,
	unsigned long *reg, unsigned long *newvalue)
{
	union mips_instruction insn;
	unsigned long value;

	*reg = 0;

	/*
	 * This load never faults. (XXX under iguana?)
	 */
	__get_user(insn.word, (unsigned int *)pc);
printk("insn = %08x\n", insn.word);

	switch (insn.i_format.opcode) {
	/*
	 * These are instructions that a compiler doesn't generate.  We
	 * can assume therefore that the code is MIPS-aware and
	 * really buggy.  Emulating these instructions would break the
	 * semantics anyway.
	 */
	case ll_op:
	case lld_op:
	case sc_op:
	case scd_op:

	/*
	 * For these instructions the only way to create an address
	 * error is an attempted access to kernel/supervisor address
	 * space.
	 */
	case ldl_op:
	case ldr_op:
	case lwl_op:
	case lwr_op:
	case sdl_op:
	case sdr_op:
	case swl_op:
	case swr_op:
	case lb_op:
	case lbu_op:
	case sb_op:
		goto sigbus;

	/*
	 * The remaining opcodes are the ones that are really of interest.
	 */
	case lh_op:
		if (verify_area(VERIFY_READ, addr, 2))
			goto sigbus;

		__asm__ __volatile__ (".set\tnoat\n"
#ifdef __BIG_ENDIAN
			"1:\tlb\t%0, 0(%1)\n"
			"2:\tlbu\t$1, 1(%1)\n\t"
#endif
#ifdef __LITTLE_ENDIAN
			"1:\tlb\t%0, 1(%1)\n"
			"2:\tlbu\t$1, 0(%1)\n\t"
#endif
			"sll\t%0, 0x8\n\t"
			"or\t%0, $1\n\t"
			: "=&r" (value)
			: "r" (addr));
		*newvalue = value;
		*reg = insn.i_format.rt;
		break;

	case lw_op:
		if (verify_area(VERIFY_READ, addr, 4))
			goto sigbus;

		__asm__ __volatile__ (
#ifdef __BIG_ENDIAN
			"1:\tlwl\t%0, (%1)\n"
			"2:\tlwr\t%0, 3(%1)\n\t"
#endif
#ifdef __LITTLE_ENDIAN
			"1:\tlwl\t%0, 3(%1)\n"
			"2:\tlwr\t%0, (%1)\n\t"
#endif
			: "=&r" (value)
			: "r" (addr));
		*newvalue = value;
		*reg = insn.i_format.rt;
		break;

	case lhu_op:
		if (verify_area(VERIFY_READ, addr, 2))
			goto sigbus;

		__asm__ __volatile__ (
			".set\tnoat\n"
#ifdef __BIG_ENDIAN
			"1:\tlbu\t%0, 0(%1)\n"
			"2:\tlbu\t$1, 1(%1)\n\t"
#endif
#ifdef __LITTLE_ENDIAN
			"1:\tlbu\t%0, 1(%1)\n"
			"2:\tlbu\t$1, 0(%1)\n\t"
#endif
			"sll\t%0, 0x8\n\t"
			"or\t%0, $1\n\t"
			: "=&r" (value)
			: "r" (addr));
		*newvalue = value;
		*reg = insn.i_format.rt;
		break;

	case lwu_op:
#ifdef CONFIG_MIPS64
		/*
		 * A 32-bit kernel might be running on a 64-bit processor.  But
		 * if we're on a 32-bit processor and an i-cache incoherency
		 * or race makes us see a 64-bit instruction here the sdl/sdr
		 * would blow up, so for now we don't handle unaligned 64-bit
		 * instructions on 32-bit kernels.
		 */
		if (verify_area(VERIFY_READ, addr, 4))
			goto sigbus;

		__asm__ __volatile__ (
#ifdef __BIG_ENDIAN
			"1:\tlwl\t%0, (%1)\n"
			"2:\tlwr\t%0, 3(%1)\n\t"
#endif
#ifdef __LITTLE_ENDIAN
			"1:\tlwl\t%0, 3(%1)\n"
			"2:\tlwr\t%0, (%1)\n\t"
#endif
			"dsll\t%0, %0, 32\n\t"
			"dsrl\t%0, %0, 32\n\t"
			: "=&r" (value)
			: "r" (addr));
		*newvalue = value;
		*reg = insn.i_format.rt;
		break;
#endif /* CONFIG_MIPS64 */

		/* Cannot handle 64-bit instructions in 32-bit kernel */
		goto sigill;

	case ld_op:
#ifdef CONFIG_MIPS64
		/*
		 * A 32-bit kernel might be running on a 64-bit processor.  But
		 * if we're on a 32-bit processor and an i-cache incoherency
		 * or race makes us see a 64-bit instruction here the sdl/sdr
		 * would blow up, so for now we don't handle unaligned 64-bit
		 * instructions on 32-bit kernels.
		 */
		if (verify_area(VERIFY_READ, addr, 8))
			goto sigbus;

		__asm__ __volatile__ (
#ifdef __BIG_ENDIAN
			"1:\tldl\t%0, (%1)\n"
			"2:\tldr\t%0, 7(%1)\n\t"
#endif
#ifdef __LITTLE_ENDIAN
			"1:\tldl\t%0, 7(%1)\n"
			"2:\tldr\t%0, (%1)\n\t"
#endif
			: "=&r" (value)
			: "r" (addr));
		*newvalue = value;
		*reg = insn.i_format.rt;
		break;
#endif /* CONFIG_MIPS64 */

		/* Cannot handle 64-bit instructions in 32-bit kernel */
		goto sigill;

	case sh_op:
		if (verify_area(VERIFY_WRITE, addr, 2))
			goto sigbus;

		value = get_reg(regs, cur_state, insn.i_format.rt);
		__asm__ __volatile__ (
#ifdef __BIG_ENDIAN
			".set\tnoat\n"
			"1:\tsb\t%0, 1(%1)\n\t"
			"srl\t$1, %0, 0x8\n"
			"2:\tsb\t$1, 0(%1)\n\t"
			".set\tat\n\t"
#endif
#ifdef __LITTLE_ENDIAN
			".set\tnoat\n"
			"1:\tsb\t%0, 0(%1)\n\t"
			"srl\t$1,%0, 0x8\n"
			"2:\tsb\t$1, 1(%1)\n\t"
			".set\tat\n\t"
#endif
			:: "r" (value), "r" (addr));
		break;

	case sw_op:
		if (verify_area(VERIFY_WRITE, addr, 4))
			goto sigbus;

		value = get_reg(regs, cur_state, insn.i_format.rt);
		__asm__ __volatile__ (
#ifdef __BIG_ENDIAN
			"1:\tswl\t%0,(%1)\n"
			"2:\tswr\t%0, 3(%1)\n\t"
#endif
#ifdef __LITTLE_ENDIAN
			"1:\tswl\t%0, 3(%1)\n"
			"2:\tswr\t%0, (%1)\n\t"
#endif
		:: "r" (value), "r" (addr));
		break;

	case sd_op:
#ifdef CONFIG_MIPS64
		/*
		 * A 32-bit kernel might be running on a 64-bit processor.  But
		 * if we're on a 32-bit processor and an i-cache incoherency
		 * or race makes us see a 64-bit instruction here the sdl/sdr
		 * would blow up, so for now we don't handle unaligned 64-bit
		 * instructions on 32-bit kernels.
		 */
		if (verify_area(VERIFY_WRITE, addr, 8))
			goto sigbus;

		value = get_reg(regs, cur_state, insn.i_format.rt);
		__asm__ __volatile__ (
#ifdef __BIG_ENDIAN
			"1:\tsdl\t%0,(%1)\n"
			"2:\tsdr\t%0, 7(%1)\n\t"
#endif
#ifdef __LITTLE_ENDIAN
			"1:\tsdl\t%0, 7(%1)\n"
			"2:\tsdr\t%0, (%1)\n\t"
#endif
		:: "r" (value), "r" (addr));
		break;
#endif /* CONFIG_MIPS64 */

		/* Cannot handle 64-bit instructions in 32-bit kernel */
		goto sigill;

	case lwc1_op:
	case ldc1_op:
	case swc1_op:
	case sdc1_op:
		/*
		 * I herewith declare: this does not happen.  So send SIGBUS.
		 */
		goto sigbus;

	case lwc2_op:
	case ldc2_op:
	case swc2_op:
	case sdc2_op:
		/*
		 * These are the coprocessor 2 load/stores.  The current
		 * implementations don't use cp2 and cp2 should always be
		 * disabled in c0_status.  So send SIGILL.
                 * (No longer true: The Sony Praystation uses cp2 for
                 * 3D matrix operations.  Dunno if that thingy has a MMU ...)
		 */
	default:
		/*
		 * Pheeee...  We encountered an yet unknown instruction or
		 * cache coherence problem.  Die sucker, die ...
		 */
		goto sigill;
	}

#ifdef CONFIG_PROC_FS
	unaligned_instructions++;
#endif

	return 0;

sigbus:
	send_sig(SIGBUS, current, 1);

	return 0;

sigill:
	send_sig(SIGILL, current, 1);

	return 0;
}

#endif
void do_ade(struct pt_regs *regs)
{
#if 0
	unsigned long reg, newval;
	unsigned long fault_addr, offset;
//	extern int do_dsemulret(struct pt_regs *);
	mm_segment_t seg;
	unsigned long pc;
	UserState_t cur_state;
	int r;

	/*
	 * Address errors may be deliberately induced by the FPU emulator to
	 * retake control of the CPU after executing the instruction in the
	 * delay slot of an emulated branch.
	 */
	/* Terminate if exception was recognized as a delay slot return */
//	if (do_dsemulret(regs))
//		return;

	/* Otherwise handle as normal */

	/*
	 * Did we catch a fault trying to load an instruction?
	 * Or are we running in MIPS16 mode?
	 */
	if ((MIPS_EXCEPT_BadVaddr(regs) == MIPS_EXCEPT_pc(regs)) ||
			(MIPS_EXCEPT_pc(regs) & 0x1))
		goto sigbus;

	pc = exception_epc(regs);
	if ((current->thread.mflags & MF_FIXADE) == 0)
		goto sigbus;

//	if ((current->thread.mflags & MF_LOGADE) != 0)
		printk(KERN_INFO "%s (%d): unaligned access at %lx\n", current->comm,
				current->pid, MIPS_EXCEPT_pc(regs));

	/*
	 * Do branch emulation only if we didn't forward the exception.
	 * This is all so but ugly ...
	 */
	seg = get_fs();
	if (!user_mode(regs))
		set_fs(KERNEL_DS);

	fault_addr = parse_ptabs_read( MIPS_EXCEPT_BadVaddr(regs), &offset);
printk("bad vaddr = %lx, lookup = %lx\n", MIPS_EXCEPT_BadVaddr(regs), fault_addr);
	if (fault_addr == -EFAULT)
		goto sigbus;
	fault_addr += offset;

{ //XXX
L4_Word_t attributes[4] = {0,0,0,0};
L4_MemoryControl (-2, attributes);
}
	/* FIXME: There should be a better way of doing this */
	memset(&cur_state, 0, sizeof(UserState_t));
	r = L4_UserState (current->thread_info->user_tid, 
			  USER_STATE_GET_STATE,
			  (L4_Word_t) &cur_state);
	BUG_ON(r != 1);

	if (!emulate_load_store_insn(regs, &cur_state, (void *)fault_addr, pc,
	                             &reg, &newval)) {
		compute_return_epc(regs);
    printk("compute\n");
		/*
		 * Now that branch is evaluated, update the dest
		 * register if necessary
		 */
		if (reg)
		{
    printk("write reg %ld = %lx\n", reg, newval);
			write_reg(regs, &cur_state, reg, newval);
			r = L4_UserState (current->thread_info->user_tid, 
					  USER_STATE_SET_STATE,
					  (L4_Word_t) &cur_state);
			BUG_ON(r != 1);
		}
{ //XXX
L4_Word_t attributes[4] = {0,0,0,0};
L4_MemoryControl (-2, attributes);
}
	}
	set_fs(seg);

	return;

sigbus:
#endif
	printk("SIGBUG\n");
	force_sig(SIGBUS, current);

	/*
	 * XXX On return from the signal handler we should advance the epc
	 */
}
