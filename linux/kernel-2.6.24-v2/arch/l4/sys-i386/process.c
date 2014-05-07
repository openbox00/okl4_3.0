/* 
 *  linux/arch/i386/kernel/process.c
 *       
 *  Copyright (C) 1995  Linus Torvalds
 *
 *  Pentium III FXSR, SSE support
 *      Gareth Hughes <gareth@valinux.com>, May 2000 
 *
 *  OKLinux modifications Geoffrey Lee < glee at ok-labs dot com > 
 */

/*
 * This file handles the architecture-dependent parts of process handling..
 */

#include <stdarg.h>
        
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/elfcore.h>
#include <linux/smp.h>
#include <linux/smp_lock.h>
#include <linux/stddef.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/user.h>
#include <linux/a.out.h>
#include <linux/interrupt.h>
#include <linux/utsname.h>
#include <linux/delay.h>
#include <linux/reboot.h>
#include <linux/init.h>

#include <asm/arch.h>
#include <asm/current.h>
#include <asm/page.h>
#include <asm/uaccess.h>

#include INC_SYSTEM(segment.h)
#include INC_SYSTEM2(ldt.h)
#include INC_SYSTEM2(desc.h)

#include <l4/thread.h>

static int
get_free_idx(struct thread_info *info)
{
	int i;

	for (i = 0; i < GDT_ENTRY_TLS_ENTRIES; i++) {
		if ((info->tls_bitmap & (1ul << i)) == 0)
			return i + GDT_ENTRY_TLS_MIN;
	}
	return -ESRCH;
}

asmlinkage int
sys_get_thread_area(struct user_desc __user *u_info)
{
	printk("get_thread_area() is not yet supported on OKLinux.\n");
	return -EINVAL;
}

/*              
 * Set a given TLS descriptor:
 */             
asmlinkage int
sys_set_thread_area(struct user_desc __user *u_info)
{
	struct thread_info *curinfo;
	struct user_desc info;
	int idx;
	L4_Word_t dummy, args[3];

	curinfo = current_thread_info();

	if (copy_from_user(&info, u_info, sizeof(info)))
		return -EFAULT;
	idx = info.entry_number;
	/*
	 * index -1  means the kernel should try to find and 
	 * allocate an empty descriptor.
	 */
	if (idx == -1) {
		if ((idx = get_free_idx(curinfo)) < 0)
			return idx;
		if (put_user(idx, &u_info->entry_number))
			return -EFAULT;
	}
	if (idx < GDT_ENTRY_TLS_MIN || idx > GDT_ENTRY_TLS_MAX)
		return -EINVAL;
	/*
	 * Setup the LDT.
	 */
	args[0] = idx|L4_LDT_Set_Gs;
	args[1] = LDT_entry_a(&info);
	args[2] = LDT_entry_b(&info);
	L4_LoadMRs(0, 3, args);
	L4_ExchangeRegisters(curinfo->user_tid,
	    L4_ExReg_Tls, 0, 0, 0, 0, L4_nilthread,
	    &dummy, &dummy, &dummy, &dummy, &dummy);
	if (info.limit_in_pages)
		curinfo->tp_value = (info.base_addr & PAGE_MASK);
	return 0;
}

int
l4_arch_set_tls(int do_tls, struct task_struct *p, struct pt_regs *regs)
{
	L4_Word_t dummy, args[3];
	struct user_desc info;
	int err;

	if (!do_tls)
		return 0;

	err = -EFAULT;
	if (copy_from_user(&info, 
	    (void __user *)L4_MsgWord(&regs->msg, 5/*esi*/),
	    sizeof(info)))
		return err;
	err = -EINVAL;
	/*
	 * This is incomplete and will need to be fixed later
	 */
	if (info.entry_number != GDT_ENTRY_TLS_MIN)
		return err;
	task_thread_info(p)->tp_value = info.base_addr;
	/*
	 * Setup the LDT.
	 */
	args[0] = info.entry_number|L4_LDT_Set_Gs;
	args[1] = LDT_entry_a(&info);
	args[2] = LDT_entry_b(&info);
	L4_LoadMRs(0, 3, args);
	L4_ExchangeRegisters(task_thread_info(p)->user_tid,
	    L4_ExReg_Tls, 0, 0, 0, 0, L4_nilthread,
	    &dummy, &dummy, &dummy, &dummy, &dummy);

	return 0;
}

unsigned long arch_align_stack(unsigned long sp)
{
#if 0   /* does this work in oklinux? -gl */
	if (randomize_va_space)
		sp -= get_random_int() % 8192;
#endif
	return sp & ~0xf;
}
