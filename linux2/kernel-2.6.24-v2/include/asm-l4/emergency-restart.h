#ifndef _ASM_EMERGENCY_RESTART_H
#define _ASM_EMERGENCY_RESTART_H

#include <l4/kdebug.h>
static inline void machine_emergency_restart(void)
{
	extern void _Exit(int);

	_Exit(5);
}

#endif /* _ASM_EMERGENCY_RESTART_H */
