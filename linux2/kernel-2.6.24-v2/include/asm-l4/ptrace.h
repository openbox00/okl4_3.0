#ifndef _L4_PTRACE_H_
#define _L4_PTRACE_H_

#include <asm/macros.h>

#include INC_SYSTEM2(ptrace.h)

/*
 * Does the process account for user or for system time?
 */
#define user_mode(regs)	((regs)->mode < 2)

#endif /* _L4_PTRACE_H_ */
