#ifndef __L4_MIPS64_CONTEXT_H
#define __L4_MIPS64_CONTEXT_H

struct arch_kernel_context {
	__u64	s0;
	__u64	s1;
	__u64	s2;
	__u64	s3;
	__u64	s4;
	__u64	s5;
	__u64	s6;
	__u64	s7;
	__u64	s8;
	__u64	sp;
	__u64	pc;
};

#endif /* __L4_MIPS64_CONTEXT_H */
