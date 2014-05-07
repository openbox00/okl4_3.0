/* 
 *	linux/arch/i386/kernel/ioport.c
 */

#include <linux/kernel.h>
#include <linux/capability.h>
#include <linux/errno.h>

/*
 * fixme: Just a stub.
 *
 * We could at least pretend to be doing what the user asked for.
 */
asmlinkage long sys_iopl(unsigned long unused)
{
	unsigned long old = 0;	/* XXX */
	unsigned long level;	/* in %ebx */

	printk("%s: not implemented.\n", __func__);
#if 0
	if (level > 3)
		return -EINVAL;
	if (level > old) {
		if (!capable(CAP_SYS_RAWIO))
			return -EPERM;
	}
#endif
	return 0;
}

asmlinkage long sys_ioperm(unsigned long from, unsigned long num, int turn_on)
{
	printk("%s: not implemented.\n", __func__);
#if 0

	if ((from + num <= from) || (from + num > IO_BITMAP_BITS))
		return -EINVAL;
	if (turn_on && !capable(CAP_SYS_RAWIO))
		return -EPERM;
#endif

	return 0;
}

