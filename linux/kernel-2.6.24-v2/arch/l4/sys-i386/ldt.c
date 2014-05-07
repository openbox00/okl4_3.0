/*
 * linux/kernel/ldt.c
 *
 * Copyright (C) 1992 Krishna Balasubramanian and Linus Torvalds
 * Copyright (C) 1999 Ingo Molnar <mingo@redhat.com>
 */

#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/smp.h>
#include <linux/smp_lock.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>

#include <asm/uaccess.h>
#include <asm/system.h>

#include INC_SYSTEM(segment.h)
#include INC_SYSTEM2(ldt.h)
#include INC_SYSTEM2(desc.h)

#include <l4/thread.h>

#if 0

#ifdef CONFIG_SMP /* avoids "defined but not used" warnig */
static void flush_ldt(void *null)
{
	if (current->active_mm)
		load_LDT(&current->active_mm->context);
}
#endif
#endif

static int sync_ldt(struct task_struct *tsk, u32 entry_1, u32 entry_2, 
		    int index)
{
	struct task_struct *p;
	struct mm_struct *mm;
	L4_Word_t dummy;

	p = tsk;
	mm = tsk->mm;
	
	do {
		L4_LoadMR(0, index);
		L4_LoadMR(1, entry_1);
		L4_LoadMR(2, entry_2);
		if (0 == L4_ExchangeRegisters(
		    task_thread_info(p)->user_tid,
		    L4_ExReg_Tls, 0, 0, 0, 0, L4_nilthread,
		    &dummy, &dummy, &dummy, &dummy, &dummy)) {
			printk("OKLinux: ldtctrl() failed.  errcode 0x%lx\n",
			    L4_ErrorCode());
			return -EINVAL;
		}
	} while ((p = next_thread(p)) != tsk);

	return 0;
}

static int alloc_ldt(mm_context_t *pc, int mincount, int reload)
{
	void *oldldt;
	void *newldt;
	int oldsize;

	if (mincount <= pc->size)
		return 0;
	oldsize = pc->size;
	mincount = (mincount+511)&(~511);
	if (mincount*LDT_ENTRY_SIZE > PAGE_SIZE)
		newldt = vmalloc(mincount*LDT_ENTRY_SIZE);
	else
		newldt = kmalloc(mincount*LDT_ENTRY_SIZE, GFP_KERNEL);

	if (!newldt)
		return -ENOMEM;

	if (oldsize)
		memcpy(newldt, pc->ldt, oldsize*LDT_ENTRY_SIZE);
	oldldt = pc->ldt;
	memset(newldt+oldsize*LDT_ENTRY_SIZE, 0, (mincount-oldsize)*LDT_ENTRY_SIZE);
	pc->ldt = newldt;
	wmb();
	pc->size = mincount;
	wmb();

	if (reload) {
#ifdef CONFIG_SMP
#error fixme: broken
		cpumask_t mask;
		preempt_disable();
		load_LDT(pc);
		mask = cpumask_of_cpu(smp_processor_id());
		if (!cpus_equal(current->mm->cpu_vm_mask, mask))
			smp_call_function(flush_ldt, NULL, 1, 1);
		preempt_enable();
#else
		/*load_LDT(pc);*/
#endif
	}
	if (oldsize) {
		if (oldsize*LDT_ENTRY_SIZE > PAGE_SIZE)
			vfree(oldldt);
		else
			kfree(oldldt);
	}
	return 0;
}


static inline int copy_ldt(mm_context_t *new, mm_context_t *old)
{
	int err = alloc_ldt(new, old->size, 0);
	if (err < 0)
		return err;
	memcpy(new->ldt, old->ldt, 
	    old->size*LDT_ENTRY_SIZE);
	return 0;
}

/*
 * we do not have to muck with descriptors here, that is
 * done in switch_mm() as needed.
 */
int l4_arch_init_new_context(struct task_struct *tsk, struct mm_struct *mm)
{
	struct mm_struct * old_mm;
	int retval = 0;

	init_MUTEX(&mm->context.sem);
	mm->context.size = 0;
	old_mm = current->mm;
	if (old_mm && old_mm->context.size > 0) {
		down(&old_mm->context.sem);
		retval = copy_ldt(&mm->context, &old_mm->context);
		up(&old_mm->context.sem);
	}
	return retval;
}

/*
 * No need to lock the MM as we are the last user
 */
void l4_arch_destroy_context(struct mm_struct *mm)
{
	if (mm->context.size) {
#if 0	/* no need to clear LDT on OKLinux -gl */
		if (mm == current->active_mm)
			clear_LDT();
#endif
		if (mm->context.size*LDT_ENTRY_SIZE > PAGE_SIZE)
			vfree(mm->context.ldt);
		else
			kfree(mm->context.ldt);
		mm->context.size = 0;
	}
}

static int read_ldt(void __user * ptr, unsigned long bytecount)
{
	int err;
	unsigned long size;
	struct mm_struct * mm = current->mm;

	if (!mm->context.size)
		return 0;
	if (bytecount > LDT_ENTRY_SIZE*LDT_ENTRIES)
		bytecount = LDT_ENTRY_SIZE*LDT_ENTRIES;

	down(&mm->context.sem);
	size = mm->context.size*LDT_ENTRY_SIZE;
	if (size > bytecount)
		size = bytecount;

	err = 0;
	if (copy_to_user(ptr, mm->context.ldt, size))
		err = -EFAULT;
	up(&mm->context.sem);
	if (err < 0)
		goto error_return;
	if (size != bytecount) {
		/* zero-fill the rest */
		if (clear_user(ptr+size, bytecount-size) != 0) {
			err = -EFAULT;
			goto error_return;
		}
	}
	return bytecount;
error_return:
	return err;
}

static int read_default_ldt(void __user * ptr, unsigned long bytecount)
{
	int err;
	unsigned long size;
	void *address;

	err = 0;
	address = &default_ldt[0];
	size = 5*sizeof(struct desc_struct);
	if (size > bytecount)
		size = bytecount;

	err = size;
	if (copy_to_user(ptr, address, size))
		err = -EFAULT;

	return err;
}

static int write_ldt(void __user * ptr, unsigned long bytecount, int oldmode)
{
	struct mm_struct * mm = current->mm;
	__u32 entry_1, entry_2, *lp;
	int error;
	struct user_desc ldt_info;

	error = -EINVAL;
	if (bytecount != sizeof(ldt_info))
		goto out;
	error = -EFAULT; 	
	if (copy_from_user(&ldt_info, ptr, sizeof(ldt_info)))
		goto out;

	error = -EINVAL;
	if (ldt_info.entry_number >= LDT_ENTRIES)
		goto out;
	if (ldt_info.contents == 3) {
		if (oldmode)
			goto out;
		if (ldt_info.seg_not_present == 0)
			goto out;
	}

	down(&mm->context.sem);
	if (ldt_info.entry_number >= mm->context.size) {
		error = alloc_ldt(&current->mm->context, ldt_info.entry_number+1, 1);
		if (error < 0)
			goto out_unlock;
	}

	lp = (__u32 *) ((ldt_info.entry_number << 3) + (char *) mm->context.ldt);

   	/* Allow LDTs to be cleared by the user. */
   	if (ldt_info.base_addr == 0 && ldt_info.limit == 0) {
		if (oldmode || LDT_empty(&ldt_info)) {
			entry_1 = 0;
			entry_2 = 0;
			goto install;
		}
	}

	entry_1 = LDT_entry_a(&ldt_info);
	entry_2 = LDT_entry_b(&ldt_info);
	if (oldmode)
		entry_2 &= ~(1 << 20);

	/* Install the new entry ...  */
install:
	/*
	 * Synchronize the LDT with all the other threads in this
	 * process.
	 */
	error = sync_ldt(current, 
	    entry_1, 
	    entry_2, 
	    ldt_info.entry_number);
	if (!error) {
		*lp	= entry_1;
		*(lp+1)	= entry_2;
	}
out_unlock:
	up(&mm->context.sem);
out:
	return error;
}

asmlinkage int sys_modify_ldt(int func, void __user *ptr, unsigned long bytecount)
{
	int ret = -ENOSYS;

	switch (func) {
	case 0:
		ret = read_ldt(ptr, bytecount);
		break;
	case 1:
		ret = write_ldt(ptr, bytecount, 1);
		break;
	case 2:
		ret = read_default_ldt(ptr, bytecount);
		break;
	case 0x11:
		ret = write_ldt(ptr, bytecount, 0);
		break;
	}
	return ret;
}
