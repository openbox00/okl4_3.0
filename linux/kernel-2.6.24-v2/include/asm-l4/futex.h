#ifndef _ASM_L4_FUTEX_H
#define _ASM_L4_FUTEX_H

#ifdef __KERNEL__

#include <linux/futex.h>
#include <asm/errno.h>
#include <asm/system.h>
#include <asm/processor.h>
#include <asm/uaccess.h>
#include INC_SYSTEM2(futex.h)

// XXX: I think it should be declared somewhere else. - Carlos
unsigned long
parse_ptabs(unsigned long address, unsigned long *offset,
					unsigned long access);
#define PF_WRITE (2)

static inline int
futex_atomic_op_inuser(int encoded_op, int __user *uaddr)
{
	unsigned long offset, page;

	page = parse_ptabs((unsigned long)uaddr, &offset, PF_WRITE);
	if(page != -EFAULT) {
		return __futex_atomic_op_inuser(encoded_op,
						(int*)(page + offset));
	}

	return -EFAULT;
}

static inline int
futex_atomic_cmpxchg_inatomic(int __user *uaddr, int oldval, int newval)
{
	unsigned long offset, page;

	page = parse_ptabs((unsigned long)uaddr, &offset, PF_WRITE);
	if(page != -EFAULT) {
		return __futex_atomic_cmpxchg_inatomic((int*)(page + offset),
							     oldval, newval);
	}

	return -EFAULT;
}

#endif
#endif
