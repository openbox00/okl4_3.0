/* 
 * Copyright (C) 2002 Jeff Dike (jdike@karaya.com)
 * Licensed under the GPL
 */

#ifndef __UM_MMU_CONTEXT_H
#define __UM_MMU_CONTEXT_H

#include <linux/sched.h>

#include <asm-generic/mm_hooks.h>

extern void l4_mm_thread_delete(struct task_struct *tsk, struct mm_struct *mm);
static inline void deactivate_mm(struct task_struct *tsk, struct mm_struct *mm)
{
}

static inline void tsk_done_mm(struct task_struct *tsk, struct mm_struct *mm)
{
	l4_mm_thread_delete(tsk, mm);
}

extern void resume_context(struct mm_struct *new_mm);

static inline void switch_mm(struct mm_struct *prev, struct mm_struct *next, 
			     struct task_struct *tsk)
{
#ifdef ARM_PID_RELOC
	if (next->context.pid == -1)
	{
		resume_context(next);
	}
#endif
}

static inline void enter_lazy_tlb(struct mm_struct *mm, 
				  struct task_struct *tsk)
{
	//printk("ENTER LAZY TLB\n");
}

void activate_mm(struct mm_struct *old, struct mm_struct *new);
extern int init_new_context(struct task_struct *task, struct mm_struct *mm);
extern void destroy_context(struct mm_struct *mm);

/*
 * Architecture hooks for OKLinux.
 */
extern int l4_arch_init_new_context(struct task_struct *, struct mm_struct *);
extern void l4_arch_destroy_context(struct mm_struct *);

#endif

/*
 * Overrides for Emacs so that we follow Linus's tabbing style.
 * Emacs will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-file-style: "linux"
 * End:
 */
