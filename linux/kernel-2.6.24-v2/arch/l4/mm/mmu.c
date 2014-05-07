#include "assert.h"

#if defined(CONFIG_IGUANA)
#include <iguana/eas.h>
#include <iguana/thread.h>
#endif

#include <linux/init.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/pagemap.h>

#ifdef ARM_PID_RELOC
#include <asm/range_fl.h>
#include <mmap.h>
#endif

#include <asm/mmu_context.h>
#include <asm/okl4.h>
#include <asm/tlb.h>

extern L4_Fpage_t utcb_area;

DEFINE_PER_CPU(struct mmu_gather, mmu_gathers);

#ifdef ARM_PID_RELOC

/* We support 32 ARM PIDS
 * This means user applications can use up to 1GB of MVA
 */
#define MAX_PIDS    32

static unsigned long pid_mask = 0;

static struct mm_struct* pid_owner[MAX_PIDS];

#endif

#define SPACE_CACHE_SIZE	3
static struct space_cache {
#if defined(CONFIG_IGUANA)
	eas_ref_t eas;
#endif
	L4_SpaceId_t space_id;
	int pid;
} space_cache[SPACE_CACHE_SIZE] = { { { 0 }, 0 }, };

extern L4_ThreadId_t main_thread;

void activate_mm(struct mm_struct *old, struct mm_struct *new)
{
	L4_ThreadId_t old_thrd = current_thread_info()->user_tid;
	//printk("%s: %p, old = %p, new = %p\n", __func__, current->thread_info, old, new);

	if (old_thrd.raw != L4_nilthread.raw)
	{
		int r;
#if defined(CONFIG_IGUANA)
		thread_delete(old_thrd);
#elif defined(CONFIG_CELL)
		okl4_delete_thread(old_thrd);
#endif
		current_thread_info()->user_tid =
#if defined(CONFIG_IGUANA)
			eas_create_thread(new->context.eas,
					L4_myselfconst, L4_myselfconst,
					(void*) L4_Address(utcb_area),
					&current_thread_info()->user_handle);
#elif defined(CONFIG_CELL)
			okl4_create_thread(new->context.space_id,
					main_thread, main_thread,
					(void *)L4_Address(utcb_area),
					&current_thread_info()->user_handle);
#endif

		assert(current_thread_info()->user_tid.raw != L4_nilthread.raw);
		r  = L4_Set_Priority(current_thread_info()->user_tid, 98);
		assert(r != 0);

		{
			char name[16];
			snprintf(name, 15, "L_%d", current->pid);
			L4_KDB_SetThreadName(current_thread_info()->user_tid, name);
		}

	}

#ifdef ARM_PID_RELOC
	if (new->context.pid == -1) {
		resume_context(new);
	}
#endif
#if defined(CONFIG_DIRECT_VM)
	{
		int r;
		unsigned long resource;

		resource = new->context.space_id.raw << 16;
		resource |= (1UL << 31);

		r = L4_SpaceControl(linux_space, L4_SpaceCtrl_resources,
				L4_ClistId(0), L4_Nilpage, resource, NULL);
	}
#endif
}

#ifdef ARM_PID_RELOC
static unsigned long rand = 0;
void resume_context(struct mm_struct *new_mm)
{
	int pid, r;
	unsigned long flags;

	rand += 5659;

	local_irq_save(flags);

	/* If free PID available */
	if ((~pid_mask) != 0)
	{
		/* Create new space */
		pid = 31 - __builtin_clzl(~pid_mask);

		pid_mask |= (1UL << pid);
	}
	else
	{
		/* No free PID, use random replacement */
		struct mm_struct *old;

		rand = rand + (rand << 7) + 6277;
		rand = (rand << 7) ^ (rand >> 17) ^ (rand << 11);

		pid = rand % MAX_PIDS;
		/* need to recycle a PID */
		old = pid_owner[pid];
		assert(old != NULL);

		//printk("replace %p with %p (%d)\n", old, new_mm, pid);

		/* Is this in the cache? */
		if ((unsigned long)old & 1)
		{
			struct space_cache* entry = (struct space_cache*)((unsigned long)old & (~1ul));
			entry->pid = -1;
#if defined(ARM_SHARED_DOMAINS)
#if defined(CONFIG_CELL)
			okl4_unshare_domain(entry->space_id);
#elif defined(CONFIG_IGUANA)
			eas_unshare_domain(entry->eas);
#else
#error
#endif
#endif
		}
		else
		{
			/* Blow away old address space mappings
			 * We can directly unmap with the deprivilleged changes regardless
			 * of the physical backing.
			 */
#if defined(CONFIG_CELL)
			okl4_unmap_page_size(&old->context, pid * 0x2000000UL, 25);
#elif defined(CONFIG_IGUANA)
			L4_Fpage_t fpg;
			fpg = L4_Fpage(pid * 0x2000000UL, 0x2000000UL);

			eas_unmap(old->context.eas, fpg);
#else
#error
#endif
			old->context.pid = -1;

#if defined(ARM_SHARED_DOMAINS)
#if defined(CONFIG_CELL)
			okl4_unshare_domain(old->context.space_id);
#elif defined(CONFIG_IGUANA)
			eas_unshare_domain(old->context.eas);
#else
#error
#endif
#endif
		}
	}
	//printk("resume: %p : pid %d\n", new_mm, pid);

	new_mm->context.pid = pid;
	pid_owner[pid] = new_mm;

#if defined(ARM_SHARED_DOMAINS)
#if defined(CONFIG_CELL)
	okl4_share_domain(new_mm->context.space_id);
#elif defined(CONFIG_IGUANA)
	r = eas_share_domain(new_mm->context.eas);
	assert(r==0);
#else
#error
#endif
#endif

#if defined(CONFIG_CELL)
	r = okl4_space_set_pid(new_mm->context.space_id, pid);
#elif defined(CONFIG_IGUANA)
	r = eas_modify(new_mm->context.eas, pid);
#else
#error
#endif
	assert(r == 0);

	local_irq_restore(flags);
}
#endif

int init_new_context(struct task_struct *task, struct mm_struct *mm)
{
	int i, found = 0;
	for (i = 0; i < SPACE_CACHE_SIZE; i++)
	{
	    if (space_cache[i].space_id.raw != 0) {
#if defined(CONFIG_IGUANA)
		mm->context.eas = space_cache[i].eas;
		space_cache[i].eas = 0;
#endif
		mm->context.space_id = space_cache[i].space_id;
		space_cache[i].space_id.raw = 0;
#ifdef ARM_PID_RELOC
		mm->context.pid = space_cache[i].pid;
		pid_owner[mm->context.pid] = mm;
		mm->context.largevm = 0;
		mm->context.mmap_free_list = rfl_new();
#endif
		found = 1;
		break;
	    }
	}

	if (!found) {
#if defined(CONFIG_IGUANA)
#ifdef ARM_PID_RELOC
		mm->context.pid = -1;	/* no PID assigned */
		mm->context.eas = eas_create(utcb_area, 0, &mm->context.space_id);
#else
		mm->context.eas = eas_create(utcb_area, &mm->context.space_id);
#endif
#elif defined(CONFIG_CELL)
#ifdef ARM_PID_RELOC
		mm->context.pid = -1;	/* no PID assigned */
#endif
		mm->context.space_id = okl4_create_space(utcb_area, 0);
#endif	/*CONFIG_CELL*/
#ifdef ARM_PID_RELOC
		mm->context.largevm = 0;
		mm->context.mmap_free_list = rfl_new();
#endif
	}

#ifdef ARM_PID_RELOC
 	if (!mm->context.mmap_free_list)
 		return -ENOMEM;
#endif

	//printk("%s: %p mm=%p, eas=%lx\n", __func__, task->thread_info, mm, mm->context.eas);

	if (
#if defined(CONFIG_IGUANA)
		!mm->context.eas
#elif defined(CONFIG_CELL)
		mm->context.space_id.raw == 0
#endif
	   ) {
		return -ENOMEM;
	}

#if !defined(ARM_PID_RELOC) && defined(ARM_SHARED_DOMAINS)
#error UNSUPPORTED
	if (!found)
	{
		int r;
		r = eas_share_domain(mm->context.eas);
		assert(r==0);
	}
#endif
	/*
	 * Architecture hook for architecture specific initialization.
	 *
	 * By the way, the ARM code above should do the same. -gl
	 */
	l4_arch_init_new_context(task, mm);
	return 0;
}

void destroy_context(struct mm_struct *mm)
{
	//printk("%s: mm=%p, eas=%lx\n", __func__, mm, mm->context.eas);
	int i, found = 0;

	for (i = 0; i < SPACE_CACHE_SIZE; i++)
	{
	    if (space_cache[i].space_id.raw == 0)
	    {
#if defined(CONFIG_IGUANA)
		space_cache[i].eas = mm->context.eas;
#endif
		space_cache[i].space_id = mm->context.space_id;
#ifdef ARM_PID_RELOC
		space_cache[i].pid = mm->context.pid;
		/* mark that cache space own this pid */
                if (mm->context.pid != -1) {
			pid_owner[mm->context.pid] = (struct mm_struct*)((unsigned long)&space_cache[i] | 1);
                }
#endif
		found = 1;
		break;
	    }
	}

	if (!found)
	{
		/* Cache is full, really delete the mm context */
#ifdef ARM_PID_RELOC
		if (mm->context.pid != -1)
		{
#if defined(ARM_SHARED_DOMAINS)
#if defined(CONFIG_CELL)
			okl4_unshare_domain(mm->context.space_id);
#elif defined(CONFIG_IGUANA)
			eas_unshare_domain(mm->context.eas);
#else
#error
#endif
#endif
			pid_mask &= ~(1UL << mm->context.pid);
			pid_owner[mm->context.pid] = NULL;

			rfl_destroy(mm->context.mmap_free_list);
			mm->context.largevm = 0;
		}
#endif

#if defined(CONFIG_IGUANA)
		eas_delete(mm->context.eas);
#elif defined(CONFIG_CELL)
		okl4_delete_space(mm->context.space_id);
#endif
	}

#if defined(CONFIG_IGUANA)
	mm->context.eas = 0;
#endif
#ifdef ARM_PID_RELOC
	mm->context.pid = -1;
#endif
	l4_arch_destroy_context(mm);
}

void l4_mm_thread_delete(struct task_struct *tsk, struct mm_struct *mm)
{
	L4_ThreadId_t tid = task_thread_info(tsk)->user_tid;
	//printk("%s: %p mm=%p\n", __func__, task->thread_info, mm);

	task_thread_info(tsk)->user_tid = L4_nilthread;
	if (tid.raw != L4_nilthread.raw)
#if defined(CONFIG_IGUANA)
		thread_delete(tid);
#elif defined(CONFIG_CELL)
		okl4_delete_thread(tid);
#endif
}
