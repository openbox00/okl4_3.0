#ifndef __L4_I386_ARCH_H
#define __L4_I386_ARCH_H

#include <l4/kdebug.h>

static inline struct
thread_info* arch_switch(struct thread_info *prev, struct thread_info *next)
{
	register __u32 from asm("eax") = (__u32)prev;
	__asm__ __volatile__ (
		"pushf			\n"
		"movl	%%ebx,	 0(%1)	\n"
		"movl	%%esi,	 4(%1)	\n"
		"movl	%%edi,	 8(%1)	\n"
		"movl	%%ebp,	12(%1)	\n"
		"movl	%%esp,	16(%1)	\n"
		"movl	16(%2),	%%esp	\n"	// restore esp first so we can push
						// prev into esp of context on the new thread
		"push	%%eax		\n"	// for new_thread_handler.
		"movl	$0,     %%eax	\n"
		"movl	$3f,    20(%1)	\n"
		"movl	0(%2),  %%ebx	\n"
		"movl	4(%2),  %%esi	\n"
		"movl	8(%2),  %%edi	\n"
		"movl	12(%2), %%ebp	\n"
		"call	*20(%2)		\n"
		"3:			\n"
		"add	$4, %%esp	\n"	// fixup stack after call
		"popl	%%eax		\n"
		"popf			\n"
		: "+r" (from)
		: "c" (&prev->context), "d"(&next->context)
	);

        return (struct thread_info*)from;
}

static inline void arch_setup_ipsp(struct thread_info *p, void* pc, __u32 sp)
{
	p->context.eip = (__u32)pc;
	p->context.esp  = (__u32)sp;
}

static inline void arch_change_stack(void * stack)
{
	/* Change the stack pointer */
	__asm__ __volatile__ (
		"movl	%0, %%esp;	    \n"
		:: "r" (stack)
		: "esp"
	);
}

#endif /* __L4_I386_ARCH_H */
