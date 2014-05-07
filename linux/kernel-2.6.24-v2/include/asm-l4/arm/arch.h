#ifndef __L4_ARM_ARCH_H
#define __L4_ARM_ARCH_H

/* Single kernel server thread switch_to ABI
 *  Return previous (process/thread)'s thread_info
 *  structure in the first argument register. So
 *  we can do thread startup. (new_process_handler etc)
 */
static inline struct thread_info* arch_switch(struct thread_info *prev, struct thread_info *next)
{
/* printk("switch  to  ip=%p, sp = %p\n", (void*)next->context.pc, (void*)next->context.sp); { */
	register __u32 from asm("r0") = (__u32)prev;
	register __u32 save asm("r1") = (__u32)&prev->context;
	register __u32 rest asm("r2") = (__u32)&next->context;

	__asm__ __volatile__ (
		"add	lr, pc,	#4			\n"
		"stmia	%[save], {r4-r11, sp, lr}	\n"
		"ldmia	%[rest], {r4-r11, sp, pc}	\n"
		: "+r" (from)
		: [rest] "r" (rest), [save] "r" (save)
		: "r3", "lr", "ip"
	);
	return (struct thread_info*)from;
/*}*/
}

static inline void arch_setup_ipsp(struct thread_info *p, void* pc, __u32 sp)
{
	p->context.pc = (__u32)pc;
	p->context.sp = (__u32)sp;
}

static inline void arch_change_stack(void * stack)
{
	/* Change the stack pointer */
	__asm__ __volatile__ (
		"mov	%%sp, %0;	    \n"
		:: "r" (stack)
	);
}

#endif /* __L4_ARM_ARCH_H */
