#ifndef _ASM_L4_I386_FUTEX_H
#define _ASM_L4_I386_FUTEX_H

#ifdef __KERNEL__

#include <linux/futex.h>
#include <asm/errno.h>
#include <asm/system.h>
#include <asm/processor.h>
#include <asm/uaccess.h>

#define __futex_atomic_op(inst, val, mem, oldval)			\
	__asm__ __volatile__ (						\
		LOCK_PREFIX inst "\n"					\
		: "=r" (oldval), "+m" (*mem)				\
		: "0" (val)						\
	);

#define __futex_atomic_op_xchg(inst, val, mem, oldval)  		\
	do {								\
		int tmp;						\
		__asm__ __volatile__ (					\
			"1: movl   %1, %0\n"				\
			"   movl   %0, %2\n"				\
			LOCK_PREFIX inst "\n"				\
			LOCK_PREFIX "cmpxchgl %2, %1\n"			\
			"   jnz	1b\n"					\
			: "=a" (oldval), "+m" (*mem), "=r" (tmp)	\
			: "r" (val)					\
		);							\
	} while(0);

static inline int
__futex_atomic_op_inuser(int encoded_op, int *kaddr)
{
	int op = (encoded_op >> 28) & 7;
	int cmp = (encoded_op >> 24) & 15;
	int oparg = (encoded_op << 8) >> 20;
	int cmparg = (encoded_op << 20) >> 20;
	int oldval = 0, ret = 0;
	if (encoded_op & (FUTEX_OP_OPARG_SHIFT << 28))
		oparg = 1 << oparg;

#if 0
	/*	
	 * OK Linux: don't do access_ok() check because
	 * we are operating on phys at this point	-gl
	 */
	if (!access_ok(VERIFY_WRITE, kaddr, sizeof(int)))
		return -EFAULT;
#endif

	pagefault_disable();

	if (op == FUTEX_OP_SET) {
		__futex_atomic_op("xchgl %0, %1", oparg, kaddr, oldval);
	} else {
#ifndef CONFIG_X86_BSWAP
		if (boot_cpu_data.x86 == 3)
			ret = -ENOSYS;
		else
#endif
		switch (op) {
		case FUTEX_OP_ADD:
			__futex_atomic_op("xaddl %0, %1", oparg, kaddr,
								 oldval);
			break;
		case FUTEX_OP_OR:
			__futex_atomic_op_xchg("orl %3, %2", oparg, kaddr,
								    oldval);
			break;
		case FUTEX_OP_ANDN:
			__futex_atomic_op_xchg("andl %3, %2", oparg, kaddr,
								     oldval);
			break;
		case FUTEX_OP_XOR:
			__futex_atomic_op_xchg("xorl %3, %2", oparg, kaddr,
								     oldval);
			break;
		default:
			ret = -ENOSYS;
		}
	}

	pagefault_enable();

	if (!ret) {
		switch (cmp) {
		case FUTEX_OP_CMP_EQ: ret = (oldval == cmparg); break;
		case FUTEX_OP_CMP_NE: ret = (oldval != cmparg); break;
		case FUTEX_OP_CMP_LT: ret = (oldval < cmparg); break;
		case FUTEX_OP_CMP_GE: ret = (oldval >= cmparg); break;
		case FUTEX_OP_CMP_LE: ret = (oldval <= cmparg); break;
		case FUTEX_OP_CMP_GT: ret = (oldval > cmparg); break;
		default: ret = -ENOSYS;
		}
	}
	return ret;
}

static inline int
__futex_atomic_cmpxchg_inatomic(int *kaddr, int oldval, int newval)
{
#if 0
	/*	
	 * OK Linux: don't do access_ok() check because
	 * we are operating on phys at this point	-gl
	 */
	if (!access_ok(VERIFY_WRITE, kaddr, sizeof(int)))
		return -EFAULT;
#endif

	__asm__ __volatile__(
		LOCK_PREFIX "cmpxchgl %2, %1\n"
		: "=a" (oldval), "+m" (*kaddr)
		: "r" (newval), "0" (oldval)
		: "memory"
	);

	return oldval;
}

#endif
#endif
