#ifndef __L4_ARM_CONTEXT_H
#define __L4_ARM_CONTEXT_H

struct arch_kernel_context {
	__u32	r4;
	__u32	r5;
	__u32	r6;
	__u32	r7;
	__u32	r8;
	__u32	r9;
	__u32	r10;	/* sl */
	__u32	r11;	/* fp */
	__u32	sp;	/* r13 */
	__u32	pc;
};

#endif /* __L4_ARM_CONTEXT_H */
