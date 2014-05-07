/*
 *  linux/include/asm-l4/cpu.h
 *
 *  Copyright (C) 2007 Open Kernel Labs
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __ASM_L4_CPU_H
#define __ASM_L4_CPU_H


#include <linux/percpu.h>

struct cpuinfo_l4 {
	struct cpu	cpu;
};

DECLARE_PER_CPU(struct cpuinfo_l4, cpu_data);

#endif
