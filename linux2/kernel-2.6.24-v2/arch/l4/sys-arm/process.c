/*
 *  linux/arch/arm/kernel/process.c
 *
 *  Copyright (C) 1996-2000 Russell King - Converted to ARM.
 *  Original Copyright (C) 1995  Linus Torvalds
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Modified for OKLinux Geoffrey Lee < glee at ok dash labs dot com >
 */
#include <stdarg.h>

#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/stddef.h>
#include <linux/unistd.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/user.h>
#include <linux/a.out.h>
#include <linux/delay.h>
#include <linux/reboot.h>
#include <linux/interrupt.h>
#include <linux/kallsyms.h>
#include <linux/init.h>

int
l4_arch_set_tls(int do_tls, struct task_struct *p, struct pt_regs *regs)
{
	if (do_tls)
		task_thread_info(p)->tp_value = ARM_r3(regs);
	/*
	 * No, this is not a mistake.  For some reason on ARM this always
	 * need to be done.
	 */
	L4_Set_Tls(task_thread_info(p)->user_tid, 
	    task_thread_info(p)->tp_value);
        return 0;
}
