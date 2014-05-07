/*
 *  linux/arch/arm/kernel/ptrace.c
 *
 *  By Ross Biro 1/23/92
 * edited by Linus Torvalds
 * ARM modifications Copyright (C) 2000 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/smp.h>
#include <linux/smp_lock.h>
#include <linux/ptrace.h>
#include <linux/user.h>
#include <linux/security.h>
#include <linux/init.h>

#include <asm/uaccess.h>
#include <asm/pgtable.h>
#include <asm/system.h>
#include INC_SYSTEM(traps.h)

//XXX#include "ptrace.h"

//#define printd	printk
#define printd(...)

#define REG_PC	15
#define REG_PSR	16
/*
 * does not yet catch signals sent when the child dies.
 * in exit.c or in signal.c.
 */

#if 0
/*
 * Breakpoint SWI instruction: SWI &9F0001
 */
#define BREAKINST_ARM	0xef9f0001
#define BREAKINST_THUMB	0xdf00		/* fill this in later */
#else
/*
 * New breakpoints - use an undefined instruction.  The ARM architecture
 * reference manual guarantees that the following instruction space
 * will produce an undefined instruction exception on all CPUs:
 *
 *  ARM:   xxxx 0111 1111 xxxx xxxx xxxx 1111 xxxx
 *  Thumb: 1101 1110 xxxx xxxx
 */
#define BREAKINST_ARM	0xe7f001f0
#define BREAKINST_THUMB	0xde01
#endif

/* this struct defines the way the registers are stored on the
   stack during a system call. */

struct arm_pt_regs {
	long uregs[18];
};

#define arm_pt_cpsr	uregs[16]
#define arm_pt_pc	uregs[15]
#define arm_pt_lr	uregs[14]
#define arm_pt_sp	uregs[13]
#define arm_pt_ip	uregs[12]
#define arm_pt_fp	uregs[11]
#define arm_pt_r10	uregs[10]
#define arm_pt_r9	uregs[9]
#define arm_pt_r8	uregs[8]
#define arm_pt_r7	uregs[7]
#define arm_pt_r6	uregs[6]
#define arm_pt_r5	uregs[5]
#define arm_pt_r4	uregs[4]
#define arm_pt_r3	uregs[3]
#define arm_pt_r2	uregs[2]
#define arm_pt_r1	uregs[1]
#define arm_pt_r0	uregs[0]
#define arm_pt_ORIG_r0	uregs[17]

/*
 * Get the address of the live pt_regs for the specified task.
 * These are saved onto the top kernel stack when the process
 * is not running.
 *
 * Note: if a user thread is execve'd from kernel space, the
 * kernel stack will not be empty on entry to the kernel, so
 * ptracing these tasks will fail.
 */
static inline struct pt_regs *
get_user_regs(struct task_struct *task)
{
	return &task_thread_info(task)->regs;
}

/*
 * this routine will get a word off of the processes privileged stack.
 * the offset is how far from the base addr as stored in the THREAD.
 * this routine assumes that all the privileged stacks are in our
 * data space.
 */
#if 1
static inline long get_user_reg(struct task_struct *task, int offset)
{
	switch (offset) {
		case 0:
		case 1:
		case 2:
		case 3:
			return L4_MsgWord(&get_user_regs(task)->msg, offset+4);
			break;
		case 4:
		case 5:
		case 6:
		case 7:
			return L4_MsgWord(&get_user_regs(task)->msg, offset-4);
			break;
		case 8:
		case 9:
		case 10:
		case 11:
		case 12: {
					 L4_Msg_t old_msg;
					 L4_Word_t reg;

					 // Back up L4 message registers
					 L4_MsgStore(L4_MsgTag(), &old_msg);

					 L4_Copy_regs_to_mrs(task_thread_info(task)->user_tid);

					 // This syscall doesn't use messages with tags, so just grab the
					 // appropriate message register
					 L4_StoreMR(offset, &reg);

					 // Restore original L4 message registers
					 L4_MsgLoad(&old_msg);
					 return reg;
					 break;
				 }

		case 13: /* sp */
				 return L4_MsgWord(&get_user_regs(task)->msg, 9);
				 break;
		case 14: /* lr */
				 return L4_MsgWord(&get_user_regs(task)->msg, 10);
				 break;
		case 15: /* pc */
				 return L4_MsgWord(&get_user_regs(task)->msg, 8);
				 break;
		case 16: /* cpsr */
				 return L4_MsgWord(&get_user_regs(task)->msg, 12);
				 break;
		default:
				 return 0;
				 break;
	}
}
#endif

/*
 * this routine will put a word on the processes privileged stack.
 * the offset is how far from the base addr as stored in the THREAD.
 * this routine assumes that all the privileged stacks are in our
 * data space.
 */
#if 1
static inline int
put_user_reg(struct task_struct *task, int offset, long data)
{
	struct pt_regs *regs = get_user_regs(task);

	switch (offset) {
		case 0:
		case 1:
		case 2:
		case 3:
			L4_MsgPutWord(&regs->msg, offset+4, data);
			break;
		case 4:
		case 5:
		case 6:
		case 7:
			L4_MsgPutWord(&regs->msg, offset-4, data);
			break;
		case 13: /* sp */
			L4_MsgPutWord(&regs->msg, 9, data);
			break;
		case 14: /* lr */
			L4_MsgPutWord(&regs->msg, 10, data);
			break;
		case 15: /* pc */
			L4_MsgPutWord(&regs->msg, 8, data);
			break;
		case 16: /* cpsr */
			L4_MsgPutWord(&regs->msg, 12, data);
			break;
		default:
			break;
	}
	return 0;
}
#endif

static inline int
read_u32(struct task_struct *task, unsigned long addr, u32 *res)
{
	int ret;

	ret = access_process_vm(task, addr, res, sizeof(*res), 0);

	return ret == sizeof(*res) ? 0 : -EIO;
}

static inline int
read_instr(struct task_struct *task, unsigned long addr, u32 *res)
{
	int ret;

	if (addr & 1) {
		u16 val;
		ret = access_process_vm(task, addr & ~1, &val, sizeof(val), 0);
		ret = ret == sizeof(val) ? 0 : -EIO;
		*res = val;
	} else {
		u32 val;
		ret = access_process_vm(task, addr & ~3, &val, sizeof(val), 0);
		ret = ret == sizeof(val) ? 0 : -EIO;
		*res = val;
	}
	return ret;
}

/*
 * Get value of register `rn' (in the instruction)
 */
#if 1
static unsigned long
ptrace_getrn(struct task_struct *child, unsigned long insn)
{
	unsigned int reg = (insn >> 16) & 15;
	unsigned long val;

	val = get_user_reg(child, reg);
	if (reg == 15)
		val = pc_pointer(val + 8);

	return val;
}
#endif

/*
 * Get value of operand 2 (in an ALU instruction)
 */
#if 1
static unsigned long
ptrace_getaluop2(struct task_struct *child, unsigned long insn)
{
	unsigned long val;
	int shift;
	int type;

	if (insn & 1 << 25) {
		val = insn & 255;
		shift = (insn >> 8) & 15;
		type = 3;
	} else {
		val = get_user_reg (child, insn & 15);

		if (insn & (1 << 4))
			shift = (int)get_user_reg (child, (insn >> 8) & 15);
		else
			shift = (insn >> 7) & 31;

		type = (insn >> 5) & 3;
	}

	switch (type) {
	case 0:	val <<= shift;	break;
	case 1:	val >>= shift;	break;
	case 2:
		val = (((signed long)val) >> shift);
		break;
	case 3:
 		val = (val >> shift) | (val << (32 - shift));
		break;
	}
	return val;
}
#endif

/*
 * Get value of operand 2 (in a LDR instruction)
 */
#if 1
static unsigned long
ptrace_getldrop2(struct task_struct *child, unsigned long insn)
{
	unsigned long val;
	int shift;
	int type;

	val = get_user_reg(child, insn & 15);
	shift = (insn >> 7) & 31;
	type = (insn >> 5) & 3;

	switch (type) {
	case 0:	val <<= shift;	break;
	case 1:	val >>= shift;	break;
	case 2:
		val = (((signed long)val) >> shift);
		break;
	case 3:
 		val = (val >> shift) | (val << (32 - shift));
		break;
	}
	return val;
}
#endif

#define OP_MASK	0x01e00000
#define OP_AND	0x00000000
#define OP_EOR	0x00200000
#define OP_SUB	0x00400000
#define OP_RSB	0x00600000
#define OP_ADD	0x00800000
#define OP_ADC	0x00a00000
#define OP_SBC	0x00c00000
#define OP_RSC	0x00e00000
#define OP_ORR	0x01800000
#define OP_MOV	0x01a00000
#define OP_BIC	0x01c00000
#define OP_MVN	0x01e00000

#if 1
static unsigned long
get_branch_address(struct task_struct *child, unsigned long pc, unsigned long insn)
{
	u32 alt = 0;

	switch (insn & 0x0e000000) {
	case 0x00000000:
	case 0x02000000: {
		/*
		 * data processing
		 */
		long aluop1, aluop2, ccbit;

		if ((insn & 0xf000) != 0xf000)
			break;

		aluop1 = ptrace_getrn(child, insn);
		aluop2 = ptrace_getaluop2(child, insn);
		ccbit  = get_user_reg(child, REG_PSR) & PSR_C_BIT ? 1 : 0;

		switch (insn & OP_MASK) {
		case OP_AND: alt = aluop1 & aluop2;		break;
		case OP_EOR: alt = aluop1 ^ aluop2;		break;
		case OP_SUB: alt = aluop1 - aluop2;		break;
		case OP_RSB: alt = aluop2 - aluop1;		break;
		case OP_ADD: alt = aluop1 + aluop2;		break;
		case OP_ADC: alt = aluop1 + aluop2 + ccbit;	break;
		case OP_SBC: alt = aluop1 - aluop2 + ccbit;	break;
		case OP_RSC: alt = aluop2 - aluop1 + ccbit;	break;
		case OP_ORR: alt = aluop1 | aluop2;		break;
		case OP_MOV: alt = aluop2;			break;
		case OP_BIC: alt = aluop1 & ~aluop2;		break;
		case OP_MVN: alt = ~aluop2;			break;
		}
		break;
	}

	case 0x04000000:
	case 0x06000000:
		/*
		 * ldr
		 */
		if ((insn & 0x0010f000) == 0x0010f000) {
			unsigned long base;

			base = ptrace_getrn(child, insn);
			if (insn & 1 << 24) {
				long aluop2;

				if (insn & 0x02000000)
					aluop2 = ptrace_getldrop2(child, insn);
				else
					aluop2 = insn & 0xfff;

				if (insn & 1 << 23)
					base += aluop2;
				else
					base -= aluop2;
			}
			if (read_u32(child, base, &alt) == 0)
				alt = pc_pointer(alt);
		}
		break;

	case 0x08000000:
		/*
		 * ldm
		 */
		if ((insn & 0x00108000) == 0x00108000) {
			unsigned long base;
			unsigned int nr_regs;

			if (insn & (1 << 23)) {
				nr_regs = hweight16(insn & 65535) << 2;

				if (!(insn & (1 << 24)))
					nr_regs -= 4;
			} else {
				if (insn & (1 << 24))
					nr_regs = -4;
				else
					nr_regs = 0;
			}

			base = ptrace_getrn(child, insn);

			if (read_u32(child, base + nr_regs, &alt) == 0)
				alt = pc_pointer(alt);
			break;
		}
		break;

	case 0x0a000000: {
		/*
		 * bl or b
		 */
		signed long displ;
		/* It's a branch/branch link: instead of trying to
		 * figure out whether the branch will be taken or not,
		 * we'll put a breakpoint at both locations.  This is
		 * simpler, more reliable, and probably not a whole lot
		 * slower than the alternative approach of emulating the
		 * branch.
		 */
		displ = (insn & 0x00ffffff) << 8;
		displ = (displ >> 6) + 8;
		if (displ != 0 && displ != 4)
			alt = pc + displ;
	    }
	    break;
	}

	return alt;
}
#endif

#if 1
static int
swap_insn(struct task_struct *task, unsigned long addr,
	  void *old_insn, void *new_insn, int size)
{
	int ret;

	ret = access_process_vm(task, addr, old_insn, size, 0);
	if (ret == size)
		ret = access_process_vm(task, addr, new_insn, size, 1);
	return ret;
}

static void
add_breakpoint(struct task_struct *task, struct debug_info *dbg, unsigned long addr)
{
	int nr = dbg->nsaved;

	if (nr < 2) {
		u32 new_insn = BREAKINST_ARM;
		int res;

		res = swap_insn(task, addr, &dbg->bp[nr].insn, &new_insn, 4);

		if (res == 4) {
			dbg->bp[nr].address = addr;
			dbg->nsaved += 1;
		}
	} else
		printk(KERN_ERR "ptrace: too many breakpoints\n");
}
#endif

/*
 * Clear one breakpoint in the user program.  We copy what the hardware
 * does and use bit 0 of the address to indicate whether this is a Thumb
 * breakpoint or an ARM breakpoint.
 */
#if 1
static void clear_breakpoint(struct task_struct *task, struct debug_entry *bp)
{
	unsigned long addr = bp->address;
	union debug_insn old_insn;
	int ret;

	if (addr & 1) {
		ret = swap_insn(task, addr & ~1, &old_insn.thumb,
				&bp->insn.thumb, 2);

		if (ret != 2 || old_insn.thumb != BREAKINST_THUMB)
			printk(KERN_ERR "%s:%d: corrupted Thumb breakpoint at "
				"0x%08lx (0x%04x)\n", task->comm, task->pid,
				addr, old_insn.thumb);
	} else {
		ret = swap_insn(task, addr & ~3, &old_insn.arm,
				&bp->insn.arm, 4);

		if (ret != 4 || old_insn.arm != BREAKINST_ARM)
			printk(KERN_ERR "%s:%d: corrupted ARM breakpoint at "
				"0x%08lx (0x%08x)\n", task->comm, task->pid,
				addr, old_insn.arm);
	}
}
#endif

#if 1
void ptrace_set_bpt(struct task_struct *child)
{
	struct pt_regs *regs;
	unsigned long pc;
	u32 insn;
	int res;

	regs = get_user_regs(child);
	pc = instruction_pointer(regs);

	if (thumb_mode(regs)) {
		printk(KERN_WARNING "ptrace: can't handle thumb mode\n");
		return;
	}

	res = read_instr(child, pc, &insn);
	if (!res) {
		struct debug_info *dbg = &child->thread.debug;
		unsigned long alt;

		dbg->nsaved = 0;

		alt = get_branch_address(child, pc, insn);
		if (alt)
			add_breakpoint(child, dbg, alt);

		/*
		 * Note that we ignore the result of setting the above
		 * breakpoint since it may fail.  When it does, this is
		 * not so much an error, but a forewarning that we may
		 * be receiving a prefetch abort shortly.
		 *
		 * If we don't set this breakpoint here, then we can
		 * lose control of the thread during single stepping.
		 */
		if (!alt || predicate(insn) != PREDICATE_ALWAYS)
			add_breakpoint(child, dbg, pc + 4);
	}
}
#endif

/*
 * Ensure no single-step breakpoint is pending.  Returns non-zero
 * value if child was being single-stepped.
 */
#if 1
void ptrace_cancel_bpt(struct task_struct *child)
{
	int i, nsaved = child->thread.debug.nsaved;

	child->thread.debug.nsaved = 0;

	if (nsaved > 2) {
		printk("ptrace_cancel_bpt: bogus nsaved: %d!\n", nsaved);
		nsaved = 2;
	}

	for (i = 0; i < nsaved; i++)
		clear_breakpoint(child, &child->thread.debug.bp[i]);
}
#endif

/*
 * Called by kernel/ptrace.c when detaching..
 *
 * Make sure the single step bit is not set.
 */
#if 1
void ptrace_disable(struct task_struct *child)
{
	child->ptrace &= ~PT_SINGLESTEP;
	ptrace_cancel_bpt(child);
}
#endif

/*
 * Handle hitting a breakpoint.
 */
void ptrace_break(struct task_struct *tsk, struct pt_regs *regs)
{
	siginfo_t info;
#if 0
	ptrace_cancel_bpt(tsk);
#endif
	info.si_signo = SIGTRAP;
	info.si_errno = 0;
	info.si_code  = TRAP_BRKPT;
	info.si_addr  = (void __user *)instruction_pointer(regs);

	force_sig_info(SIGTRAP, &info, tsk);
}

#if 1
asmlinkage long sys_arm_breakpoint(struct pt_regs *regs)
{
	ARM_put_pc(regs, thumb_mode(regs)? ARM_pc(regs)-2 : ARM_pc(regs)-4);
	ptrace_break(current, regs);
	return ARM_r0(regs);
}
#endif

/*
 * Read the word at offset "off" into the "struct user".  We
 * actually access the pt_regs stored on the kernel stack.
 */
#if 1
static int ptrace_read_user(struct task_struct *tsk, unsigned long off,
			    unsigned long __user *ret)
{
	unsigned long tmp;

	if (off & 3 || off >= sizeof(struct user))
		return -EIO;

	tmp = 0;
	if (off < sizeof(struct pt_regs))
		tmp = get_user_reg(tsk, off >> 2);

	return put_user(tmp, ret);
}
#endif

/*
 * Write the word at offset "off" into "struct user".  We
 * actually access the pt_regs stored on the kernel stack.
 */
#if 1
static int ptrace_write_user(struct task_struct *tsk, unsigned long off,
			     unsigned long val)
{
	if (off & 3 || off >= sizeof(struct user))
		return -EIO;

	if (off >= sizeof(struct pt_regs))
		return 0;

	return put_user_reg(tsk, off >> 2, val);
}
#endif

/*
 * Get all user integer registers.
 */
static int ptrace_getregs(struct task_struct *tsk, void __user *uregs)
{
	struct pt_regs *regs = get_user_regs(tsk);
	struct arm_pt_regs arm_regs;

	L4_Msg_t old_msg;
	
	/* Back up L4 message registers */
	L4_MsgStore(L4_MsgTag(), &old_msg);
	
	/* grab r8-10, fp & ip */
	L4_Copy_regs_to_mrs(task_thread_info(tsk)->user_tid);
	
	L4_StoreMR(8, &arm_regs.arm_pt_r8);
	L4_StoreMR(9, &arm_regs.arm_pt_r9);
	L4_StoreMR(10, &arm_regs.arm_pt_r10);
	L4_StoreMR(11, &arm_regs.arm_pt_fp);
	//L4_StoreMR(12, &arm_regs.arm_pt_ip);
	
	/* Restore original L4 message registers */
	L4_MsgLoad(&old_msg);
	
	/* 
	 * get the remaining regsiters
	 * saved on fault
	 */
	arm_regs.arm_pt_cpsr =	ARM_cpsr(regs);
	arm_regs.arm_pt_pc =	ARM_pc(regs) & 0xfffffffe;
	arm_regs.arm_pt_lr =	ARM_lr(regs);
	arm_regs.arm_pt_sp =	ARM_sp(regs);
	if (regs->strace_flag)
		arm_regs.arm_pt_ip =    ARM_ip(regs);

	arm_regs.arm_pt_r0 =	ARM_r0(regs);
	arm_regs.arm_pt_r1 =	ARM_r1(regs);
	arm_regs.arm_pt_r2 =	ARM_r2(regs);
	arm_regs.arm_pt_r3 =	ARM_r3(regs);
	arm_regs.arm_pt_r4 =	ARM_r4(regs);
	arm_regs.arm_pt_r5 =	ARM_r5(regs);
	arm_regs.arm_pt_r6 =	ARM_r6(regs);
	arm_regs.arm_pt_r7 =	ARM_r7(regs);

	arm_regs.arm_pt_ORIG_r0 = ARM_save(regs);

	printd("  r0 = %08lx, r1 = %08lx, r2 = %08lx, r3 = %08lx\r\n",
	      arm_regs.arm_pt_r0, arm_regs.arm_pt_r1, 
	      arm_regs.arm_pt_r2, arm_regs.arm_pt_r3);
	
	printd("  r4 = %08lx, r5 = %08lx, r6 = %08lx, r7 = %08lx\r\n",
	      arm_regs.arm_pt_r4, arm_regs.arm_pt_r5, 
	      arm_regs.arm_pt_r6, arm_regs.arm_pt_r7);
	
	printd("  r8 = %08lx, r9 = %08lx, r10= %08lx, fp = %08lx\r\n",
	      arm_regs.arm_pt_r8, arm_regs.arm_pt_r9, 
	      arm_regs.arm_pt_r10, arm_regs.arm_pt_fp);
	
	printd("  ip = %08lx, sp = %08lx, lr = %08lx, pc = %08lx\r\n",
	      arm_regs.arm_pt_ip, arm_regs.arm_pt_sp, 
	      arm_regs.arm_pt_lr, arm_regs.arm_pt_pc);

	return copy_to_user(uregs, &arm_regs, sizeof(struct arm_pt_regs)) ? -EFAULT : 0;
}

/*
 * Store all the register values in message registers
 */
static void store_regs(struct arm_pt_regs *arm_regs, struct pt_regs *regs)
{
	L4_LoadMR(8, arm_regs->arm_pt_r8);
	L4_LoadMR(9, arm_regs->arm_pt_r9);
	L4_LoadMR(10, arm_regs->arm_pt_r10);
	L4_LoadMR(11, arm_regs->arm_pt_fp);
	L4_LoadMR(12, arm_regs->arm_pt_ip);
	
	ARM_put_cpsr(regs, arm_regs->arm_pt_cpsr);
	ARM_put_pc(regs, arm_regs->arm_pt_pc & 0xfffffffe);
	ARM_put_lr(regs, arm_regs->arm_pt_lr);
	ARM_put_sp(regs, arm_regs->arm_pt_sp);
	
	ARM_put_r0(regs, arm_regs->arm_pt_r0);
	ARM_put_r1(regs, arm_regs->arm_pt_r1);
	ARM_put_r2(regs, arm_regs->arm_pt_r2);
	ARM_put_r3(regs, arm_regs->arm_pt_r3);
	ARM_put_r4(regs, arm_regs->arm_pt_r4);
	ARM_put_r5(regs, arm_regs->arm_pt_r5);
	ARM_put_r6(regs, arm_regs->arm_pt_r6);
	ARM_put_r7(regs, arm_regs->arm_pt_r7);
	ARM_put_save(regs, arm_regs->arm_pt_ORIG_r0);
}

#define arm_user_mode(regs) \
    (((regs)->arm_pt_cpsr & 0xf) == 0)

int valid_user_regs(struct arm_pt_regs* regs)
{
	if (arm_user_mode(regs) &&
			((regs->arm_pt_cpsr & (PSR_I_BIT)) == 0))
		return 1;
	return 0;
}

/*
 * Set all user integer registers.
 */
#if 1
static int ptrace_setregs(struct task_struct *tsk, void __user *uregs)
{
	struct arm_pt_regs newregs;
	int ret;

	ret = -EFAULT;
	if (copy_from_user(&newregs, uregs, sizeof(struct arm_pt_regs)) == 0) {
		struct pt_regs *regs = get_user_regs(tsk);

		ret = -EINVAL;
		if (valid_user_regs(&newregs)) {
			store_regs(&newregs, regs);
			L4_Copy_mrs_to_regs(task_thread_info(tsk)->user_tid);
			ret = 0;
		}
	}

	return ret;
}
#endif

/*
 * Get the child FPU state.
 */
#if 0
static int ptrace_getfpregs(struct task_struct *tsk, void __user *ufp)
{
	return copy_to_user(ufp, &(task_thread_info(tsk)->fpstate),
			    sizeof(struct user_fp)) ? -EFAULT : 0;
}
#endif

/*
 * Set the child FPU state.
 */
#if 0
static int ptrace_setfpregs(struct task_struct *tsk, void __user *ufp)
{
	struct thread_info *thread = task_thread_info(tsk);
	thread->used_cp[1] = thread->used_cp[2] = 1;
	return copy_from_user(&thread->fpstate, ufp,
			      sizeof(struct user_fp)) ? -EFAULT : 0;
}
#endif

long arch_ptrace(struct task_struct *child, long request, long addr, long data)
{
	unsigned long tmp;
	int ret;

	switch (request) {
		/*
		 * read word at location "addr" in the child process.
		 */
		case PTRACE_PEEKTEXT:
		case PTRACE_PEEKDATA:
printd(KERN_WARNING " ptrace peek\n");
			ret = access_process_vm(child, addr, &tmp,
						sizeof(unsigned long), 0);
			if (ret == sizeof(unsigned long))
			{
printd(KERN_WARNING "  peek %lx = %lx\n", addr, tmp);
				ret = put_user(tmp, (unsigned long __user *) data);
			}
			else
			{
printd(KERN_WARNING "  peek at %lx failed!\n", addr);
				ret = -EIO;
			}
			break;

		case PTRACE_PEEKUSR:
printd(KERN_WARNING " ptrace %d\n", __LINE__);
			ret = ptrace_read_user(child, addr, (unsigned long __user *)data);
			break;

		/*
		 * write the word at location addr.
		 */
		case PTRACE_POKETEXT:
		case PTRACE_POKEDATA:
printd(KERN_WARNING " ptrace poke\n");
			ret = access_process_vm(child, addr, &data,
						sizeof(unsigned long), 1);
			if (ret == sizeof(unsigned long))
				ret = 0;
			else
				ret = -EIO;
			break;

		case PTRACE_POKEUSR:
printd(KERN_WARNING " ptrace %d\n", __LINE__);
			ret = ptrace_write_user(child, addr, data);
			break;

		/*
		 * continue/restart and stop at next (return from) syscall
		 */
		case PTRACE_SYSCALL:
		case PTRACE_CONT:
			ret = -EIO;
printd(KERN_WARNING " ptrace cont/restart\n");
			if ((unsigned long) data > _NSIG)
				break;
			if (request == PTRACE_SYSCALL)
				set_tsk_thread_flag(child, TIF_SYSCALL_TRACE);
			else
				clear_tsk_thread_flag(child, TIF_SYSCALL_TRACE);

			child->exit_code = data;
			/* make sure single-step breakpoint is gone. */
			child->ptrace &= ~PT_SINGLESTEP;
//XXX			ptrace_cancel_bpt(child);
			wake_up_process(child);
			ret = 0;
			break;

		/*
		 * make the child exit.  Best I can do is send it a sigkill.
		 * perhaps it should be put in the status that it wants to
		 * exit.
		 */
		case PTRACE_KILL:
printd(KERN_WARNING " ptrace kill\n");
			/* make sure single-step breakpoint is gone. */
			child->ptrace &= ~PT_SINGLESTEP;
//XXX			ptrace_cancel_bpt(child);
			if (child->state != EXIT_ZOMBIE) {
				child->exit_code = SIGKILL;
				wake_up_process(child);
			}
			ret = 0;
			break;

		/*
		 * execute single instruction.
		 */
		case PTRACE_SINGLESTEP:
printd(KERN_WARNING " ptrace singlestep\n");
			ret = -EIO;
			if ((unsigned long) data > _NSIG)
				break;
			child->ptrace |= PT_SINGLESTEP;
			clear_tsk_thread_flag(child, TIF_SYSCALL_TRACE);
			child->exit_code = data;
			/* give it a chance to run. */
			wake_up_process(child);
			ret = 0;
			break;

		case PTRACE_DETACH:
printd(KERN_WARNING " ptrace detatch\n");
			ret = ptrace_detach(child, data);
			break;

		case PTRACE_GETREGS:
printd(KERN_WARNING " ptrace getregs\n");
			ret = ptrace_getregs(child, (void __user *)data);
			break;

		case PTRACE_SETREGS:
ret = -EIO;
printk(KERN_WARNING " ptrace %d\n", __LINE__);
#if 1
			ret = ptrace_setregs(child, (void __user *)data);
#endif
			break;

		case PTRACE_GETFPREGS:
ret = -EIO;
printk(KERN_WARNING " ptrace %d\n", __LINE__);
#if 0
			ret = ptrace_getfpregs(child, (void __user *)data);
#endif
			break;
		
		case PTRACE_SETFPREGS:
ret = -EIO;
printk(KERN_WARNING " ptrace %d\n", __LINE__);
#if 0
			ret = ptrace_setfpregs(child, (void __user *)data);
#endif
			break;
		case PTRACE_GET_THREAD_AREA:
printk(KERN_WARNING " ptrace_get_thrd_area()\n");
			ret = put_user(task_thread_info(child)->tp_value,
					(unsigned long __user *) data);
			break;
 
		default:
printd(KERN_WARNING " ptrace_request()\n");
			ret = ptrace_request(child, request, addr, data);
			break;
	}

	return ret;
}

void do_syscall_trace(int why, struct pt_regs *regs)
{
	unsigned long ip;
printd(KERN_WARNING "do_syscall_trace %d\n", why);

	if (!test_thread_flag(TIF_SYSCALL_TRACE))
		return;
	if (!(current->ptrace & PT_PTRACED))
		return;


	/*
	 * Save IP.  IP is used to denote syscall entry/exit:
	 *  IP = 0 -> entry, = 1 -> exit
	 */
	ARM_set_strace(regs, 1);
	ip = ARM_ip(regs);
	ARM_put_ip(regs, why);

	/* the 0x80 provides a way for the tracing parent to distinguish
	   between a syscall stop and SIGTRAP delivery */
	ptrace_notify(SIGTRAP | ((current->ptrace & PT_TRACESYSGOOD)
				 ? 0x80 : 0));
	/*
	 * this isn't the same as continuing with a signal, but it will do
	 * for normal use.  strace only continues with a signal if the
	 * stopping signal is not SIGTRAP.  -brl
	 */
	if (current->exit_code) {
		send_sig(current->exit_code, current, 1);
		current->exit_code = 0;
	}
	ARM_put_ip(regs, ip);
	ARM_set_strace(regs, 0);
}
