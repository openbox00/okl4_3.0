#ifndef __L4_ARM_PROCESSOR_H
#define __L4_ARM_PROCESSOR_H

#define cpu_relax()	barrier()

union debug_insn {
	u32	arm;
	u16	thumb;
};

struct debug_entry {
	u32			address;
	union debug_insn	insn;
};

struct debug_info {
	int			nsaved;
	struct debug_entry	bp[2];
};

struct thread_struct {
							/* fault info	  */
	unsigned long		address;
	unsigned long		trap_no;
	unsigned long		error_code;
							/* debugging	  */
	struct debug_info	debug;
};

#define INIT_THREAD  { }


#ifdef ARM_PID_RELOC
/*
 * User space process size: 32MB. This is hardcoded into a few places,
 * so don't change it unless you know what you are doing.
 *
 * Large VM support has increased this to 2GB
 */
#define TASK_SIZE	0x7fff8000UL /* PID relocation */
#define TASK_SHIFT	31

/*
 * This decides where the kernel will search for a free chunk of vm
 * space during mmap's.
 */
#define TASK_UNMAPPED_BASE	(0x00800000)	/* PID relocation */

/*
 * This decides where the kernel will map the __wombat_user_sig_handler
 * page to for signal and thread startup/modification.
 * This must not conflict with the utcb and kip area setup in
 * arch/l4/kernel/setup.c
 */
#define TASK_SIG_BASE	(0x01ffa000UL)		/* PID relocation */

#else /* NO PID RELOCATION */

/*
 * User space process size: 2GB. This is hardcoded into a few places,
 * so don't change it unless you know what you are doing.
 */
#define TASK_SIZE	0x7fff8000UL
#define TASK_SHIFT	31

/*
 * This decides where the kernel will search for a free chunk of vm
 * space during mmap's.
 */
#define TASK_UNMAPPED_BASE	(0x30000000)

/*
 * This decides where the kernel will map the __wombat_user_sig_handler
 * page to for signal and thread startup/modification.
 * This must not conflict with the utcb and kip area setup in
 * arch/l4/kernel/setup.c
 * NPTL support (thread pointer, cmpxchg) is also added to this page.
 */
#define TASK_SIG_BASE	(0x98000000UL)

#endif

#endif /* __L4_ARM_PROCESSOR_H */
