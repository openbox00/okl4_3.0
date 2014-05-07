/*
 * linux/arch/l4/sys-arm/mach-sa1100/generic.c
 *
 * Author: Nicolas Pitre
 * Author: David Snowdon
 *
 * Code common to all SA11x0 machines.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/cpufreq.h>
#include <linux/ioport.h>

/* This is a placeholder in case we ever want to do SA1100 specific setup */ 

static int __init sa1100_init(void)
{
	return 0;
}

arch_initcall(sa1100_init);
