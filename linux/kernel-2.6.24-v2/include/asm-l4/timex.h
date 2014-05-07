#ifndef __L4_TIMEX_H
#define __L4_TIMEX_H

#define cacheflush_time (0)

#include <linux/sysdev.h>

/*
 * Using the vtimer you can have up to nanosecond resolution.  Of course
 * the real resolution is going to be less than that.
 */
#if defined(CONFIG_L4_CLOCKSOURCE)
#define CLOCK_TICK_RATE 1000000000

typedef unsigned long cycles_t;

static inline cycles_t get_cycles (void)
{
	return 0;
}

#else
#include INC_SYSTEM2(timex.h)
#endif

#endif
