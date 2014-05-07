/*
 * arch/l4/sys-arm/mmu.c
 *
 * ARM-specific OKLinux routines
 * Geoffrey Lee < glee at ok-labs dot com >
 */
#include "assert.h"

#include <linux/init.h>
#include <linux/sched.h>
#include <linux/mm.h>

#include <asm/mmu_context.h> 

/*
 * Empty currently.
 *
 * XXX we should move the shared domains stuff here too. -gl
 */
int
l4_arch_init_new_context(struct task_struct *tsk, struct mm_struct *mm)
{
	return 0;
}

/*
 * Empty.
 *
 * XXX move shared domain stuff here.
 */
void
l4_arch_destroy_context(struct mm_struct *mm)
{
}
