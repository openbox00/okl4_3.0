#ifndef __L4_I386_CONTEXT_H
#define __L4_I386_CONTEXT_H

struct arch_kernel_context {
	__u32	ebx;
	__u32	esi;
	__u32	edi;
	__u32	ebp;
	__u32	esp;
	__u32	eip;
};

#endif /* __L4_I386_CONTEXT_H */
