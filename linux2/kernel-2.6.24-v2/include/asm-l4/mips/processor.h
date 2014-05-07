#ifndef __L4_MIPS64_PROCESSOR_H
#define __L4_MIPS64_PROCESSOR_H

#define cpu_relax()	barrier()

#define MF_ABI_MASK     (MF_32BIT_REGS | MF_32BIT_ADDR)
#define MF_O32          (MF_32BIT_REGS | MF_32BIT_ADDR)
#define MF_N32          MF_32BIT_ADDR
#define MF_N64          0

struct thread_struct {
#define MF_FIXADE	1		/* Fix address errors in software */
#define MF_LOGADE	2		/* Log address errors to syslog */
#define MF_32BIT_REGS	4		/* also implies 16/32 fprs */
#define MF_32BIT_ADDR	8		/* 32-bit address space (o32/n32) */
	unsigned long mflags;
};

#define INIT_THREAD  { MF_FIXADE | MF_LOGADE }

/*
 * User space process size: 512GB. This is hardcoded into a few places,
 * so don't change it unless you know what you are doing.
 */
#define TASK_SIZE32	0x7fff8000UL
#define TASK_SIZE	0x8000000000UL
// XXX TASK_SHIFT

/*
 * This decides where the kernel will search for a free chunk of vm
 * space during mmap's.
 */
#define TASK_UNMAPPED_BASE	((current->thread.mflags & MF_32BIT_ADDR) ? \
	PAGE_ALIGN(TASK_SIZE32 / 3) : PAGE_ALIGN(TASK_SIZE / 3))

/*
 * This decides where the kernel will map the __wombat_user_sig_handler
 * page to for signal and thread startup/modification.
 * This must not conflict with the utcb and kip area setup in
 * arch/l4/kernel/setup.c
 */
#define TASK_SIG_BASE	((current->thread.mflags & MF_32BIT_ADDR) ? \
	PAGE_ALIGN(0x7fffe000UL) : PAGE_ALIGN(0x9876543000UL))

#endif /* __L4_MIPS64_PROCESSOR_H */
