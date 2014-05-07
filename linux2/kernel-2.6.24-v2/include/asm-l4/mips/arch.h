#ifndef __L4_MIPS_ARCH_H
#define __L4_MIPS_ARCH_H

/* Single kernel server thread switch_to ABI
 *  Return previous (process/thread)'s thread_info
 *  structure in the first argument register. So
 *  we can so thread startup. (new_process_handler etc)
 */
static inline struct thread_info* arch_switch(struct thread_info *prev, struct thread_info *next)
{
/* printk("switch  to  ip=%p, sp = %p\n", (void*)next->context.pc, (void*)next->context.sp); { */
	register __u64 from asm("$4") = (__u64)prev;
	register __u64 save asm("$5") = (__u64)&prev->context;
	register __u64 rest asm("$6") = (__u64)&next->context;

	__asm__ __volatile__ (
		"sd	$16,	 0(%[save])		\n"
		"sd	$17,	 8(%[save])		\n"
		"sd	$18,	16(%[save])		\n"
		"sd	$19,	24(%[save])		\n"
		"dla	$31,	0f			\n"
		"sd	$20,	32(%[save])		\n"
		"sd	$21,	40(%[save])		\n"
		"sd	$22,	48(%[save])		\n"
		"sd	$23,	56(%[save])		\n"
		"sd	$30,	64(%[save])		\n"
		"sd	$29,	72(%[save])		\n"
		"sd	$31,	80(%[save])		\n"

		"ld	$20,	32(%[rest])		\n"
		"ld	$21,	40(%[rest])		\n"
		"ld	$22,	48(%[rest])		\n"
		"ld	$23,	56(%[rest])		\n"
		"ld	$30,	64(%[rest])		\n"
		"ld	$29,	72(%[rest])		\n"
		"ld	$31,	80(%[rest])		\n"
		"ld	$16,	 0(%[rest])		\n"
		"ld	$17,	 8(%[rest])		\n"
		"ld	$18,	16(%[rest])		\n"
		"ld	$19,	24(%[rest])		\n"
		"jr	$31				\n"
		"0:					\n"
		: "+r" (from)
		: [rest] "r" (rest), [save] "r" (save)
		: "$1", "$2", "$3", "$7", "$8", "$9", "$10",
		  "$11", "$12", "$13", "$14", "$15", "$24",
		  "$25", "$28", "memory"
	);

	return (struct thread_info*)from;
/*}*/
}

static inline void arch_setup_ipsp(struct thread_info *p, void* pc, __u64 sp)
{
	p->context.pc = (__u64)pc;
	p->context.sp = (__u64)sp;
}

static inline void arch_change_stack(void * stack)
{
	/* Change the stack pointer */
	__asm__ __volatile__ (
		"move	$29, %0;	    \n"
		:: "r" (stack)
		: "$29"
	);
}

#endif /* __L4_MIPS_ARCH_H */

