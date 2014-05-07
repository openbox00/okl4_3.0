
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/tick.h>
#include <l4.h>
#include <l4/caps.h>

#include <l4/schedule.h>

#include "assert.h"

#include <asm/arch.h>
#include <asm/current.h>
#include <asm/page.h>
#include <asm/tlbflush.h>
#include <asm/uaccess.h>
#include <asm/signal_l4.h>
#include <asm/okl4.h>

#if defined(CONFIG_IGUANA)
#include <iguana/eas.h>
#include <iguana/thread.h>
#elif defined(CONFIG_CELL)
#include <okl4/bitmap.h>
#include <okl4/kspaceid_pool.h>
#include <okl4/env.h>
#endif

extern void NORET_TYPE syscall_loop (void);
static void new_thread_handler(struct thread_info *prev);
static void new_process_handler(struct thread_info *prev);
extern void schedule_tail(struct task_struct *prev);

extern L4_Fpage_t utcb_area;
extern L4_ThreadId_t timer_thread;

#include <l4/ipc.h>
#include <l4/message.h>
#include <l4/thread.h>

#ifdef __i386__
#include INC_SYSTEM(segment.h)
#include INC_SYSTEM(ldt.h)

/* For VDSO stack copying */
#include <asm/elf.h>
extern void SYSENTER_RETURN;

#endif

/*
 * Initialises a new kernel thread
 */
long
kernel_thread(int (*fn)(void *), void *arg, unsigned long flags)
{
	long pid;
	//printk("kernel_thread for: %p\n", fn);
	current_thread_info()->request.u.thread.proc = fn;
	current_thread_info()->request.u.thread.arg = arg;
	current_thread_info()->request.op = OP_KTHREAD;
	current_thread_info()->regs.mode = 1;
	pid = do_fork(CLONE_VM | flags, 0, NULL, 0, NULL, NULL);
	//printk("kernel_thread for: %p is %ld\n", fn, pid);
	return pid;
}


/*
 * Binding for the sys_fork system call
 */
long
sys_fork(struct pt_regs * regs)
{
	current_thread_info()->request.op = OP_FORK;

	return do_fork(SIGCHLD, ARCH_sp(regs), regs, 0, NULL, NULL);
}

/*
 * This is trivial, and on the face of it looks like it
 * could equally well be done in user mode.
 *
 * Not so, for quite unobvious reasons - register pressure.
 * In user mode vfork() cannot have a stack frame, and if
 * done by calling the "clone()" system call directly, you
 * do not have enough call-clobbered registers to hold all
 * the information you need.
 */
long
sys_vfork(struct pt_regs * regs)
{
	current_thread_info()->request.op = OP_FORK;

	return do_fork(CLONE_VFORK | CLONE_VM | SIGCHLD, ARCH_sp(regs), regs, 0, NULL, NULL);
}


/*
 * Binding for the sys_execve system call
 */
long
sys_execve(char *file, char **argv, char **env)
{
	int error;
	char *filename;
	filename = getname((char *) file);
	error = PTR_ERR(filename);
	if (IS_ERR(filename)) goto out;
	error = do_execve(filename, argv, env, NULL);
	putname(filename);
 out:
	return(error);
}

/* This is the execve entry point for use when called
   from init at kernel startup. At this case at the end
   of it we want to go into the syscall_loop
*/
int
__execve(char *file, char **argv, char **env)
{
	int ret;

	ret = sys_execve(file, argv, env);
	if (ret < 0)
		return ret;

	set_usermode_status_true(current_thread_info());

	/* Reset the stack pointer - forget old context */
	arch_change_stack((void*) ((THREAD_SIZE +
				(unsigned long) current_thread_info() -
				sizeof(long)) & ~0x7));

	syscall_loop();
	/* Don't return */
	while (1);
}

/*
 * This is the linux kernel idle thread, which is scheduled when
 * theres nothing else to run.
 * This does a blocking receive IPC on the timer_thread which
 * effectively waits for a timer/interrupt wakeup
 */
void
cpu_idle(void)
{
	L4_ThreadId_t	dummy;

	/* Idle thread has special user_tid so we can identify it */
	current_thread_info()->user_tid = L4_anythread;
	current_thread_info()->user_handle = L4_anythread;

	L4_Accept(L4_NotifyMsgAcceptor);
	L4_Set_NotifyMask(L4_PREEMPT);

	while (1) {
		tick_nohz_stop_sched_tick();
		while (!need_resched()) {
			if (!need_resched())
				L4_Wait(&dummy);
		}
		tick_nohz_restart_sched_tick();
		//printk("!");
		schedule();
	}
}

#if defined(CONFIG_CELL)

struct okl4_bitmap_allocator *spaceid_pool, *threadid_pool, *clistid_pool;

static struct okl4_bitmap_allocator *
init_range_alloc_interval(word_t start, word_t end)
{
	struct okl4_bitmap_allocator *alloc;
	okl4_allocator_attr_t attr;

	okl4_allocator_attr_init(&attr);
	okl4_allocator_attr_setrange(&attr, start, end - start);
	alloc = kmalloc(OKL4_BITMAP_ALLOCATOR_SIZE(end - start), GFP_KERNEL);
	if (alloc == NULL)
		return NULL;

	okl4_bitmap_allocator_init(alloc, &attr);

	return alloc;
}

extern L4_ThreadId_t main_thread;
L4_ClistId_t user_clist;

static int __init
l4_process_init(void)
{
	struct okl4_bitmap_item bitmap_item;
	int r;

	threadid_pool = init_range_alloc_interval(0x10, 512);
	spaceid_pool = okl4_env_get("MAIN_SPACE_ID_POOL");
	clistid_pool = okl4_env_get("MAIN_CLIST_ID_POOL");

	assert(spaceid_pool != NULL);
	assert(threadid_pool != NULL);
	assert(clistid_pool != NULL);

	okl4_bitmap_item_setany(&bitmap_item);
	r = okl4_bitmap_allocator_alloc(clistid_pool, &bitmap_item);
	assert(r == OKL4_OK);
	user_clist = L4_ClistId(bitmap_item.unit);

	/* XXX TODO ARMV6 STUFF? */

	r = L4_CreateClist(user_clist, 1);
	assert(r == 1);

	return 0;
}

L4_ThreadId_t
okl4_create_sys_thread(L4_SpaceId_t space, L4_ThreadId_t pager, 
		L4_ThreadId_t scheduler, void *utcb, word_t unit)
{
	L4_ThreadId_t thrd;
    int r;
	thrd = L4_CapId(TYPE_CAP, unit);
	r = L4_ThreadControl(thrd, space, scheduler, pager, pager, 0, 
	    utcb);
    if (r != 1) {
        L4_KDB_Enter("Failed to create main thread");
    }

	return thrd;
}

L4_ThreadId_t
okl4_create_thread(L4_SpaceId_t space, L4_ThreadId_t pager, 
		L4_ThreadId_t scheduler, void *utcb, L4_ThreadId_t *handle_rv)
{
	struct okl4_bitmap_item bitmap_item;
	L4_ThreadId_t thrd;
	int ret;

	okl4_bitmap_item_setany(&bitmap_item);
	ret = okl4_bitmap_allocator_alloc(threadid_pool, &bitmap_item);

	*handle_rv = L4_nilthread;

	if (ret != OKL4_OK)
		return L4_nilthread;

	thrd = L4_CapId(TYPE_CAP, bitmap_item.unit);
	ret = L4_ThreadControl(thrd, space, scheduler, pager, pager, 0, utcb);
	if (ret != 1) {
		okl4_bitmap_allocator_free(threadid_pool, &bitmap_item);
		L4_KDB_Enter("okl4 create thrd failed");
		printk("okl4 create thrd %lx failed. Error = %d\n", thrd.raw, ret);
		return L4_nilthread;
	}

	L4_StoreMR(0, &handle_rv->raw);

	return thrd;
}

void
okl4_delete_thread(L4_ThreadId_t thrd)
{
	struct okl4_bitmap_item bitmap_item;

	bitmap_item.unit = (uintptr_t)L4_CapIndex(thrd);
	okl4_bitmap_allocator_free(threadid_pool, &bitmap_item);

	if (L4_ThreadControl(thrd, L4_nilspace, L4_nilthread, L4_nilthread, 
	    L4_nilthread, 0, NULL) != 1)
		panic("can't delete thread");
}

L4_SpaceId_t
okl4_create_space(L4_Fpage_t utcb_area, L4_Word_t max_prio)
{
	struct okl4_bitmap_item bitmap_item;
	L4_SpaceId_t space;
	word_t ctrl, resource;
	int ret;

	okl4_bitmap_item_setany(&bitmap_item);
	ret = okl4_bitmap_allocator_alloc(spaceid_pool, &bitmap_item);
	if (ret != OKL4_OK)
		return L4_nilspace;

	space = L4_SpaceId(bitmap_item.unit);
	// even with PID relocation, we don't set PID here
	resource = 0;

	ctrl = L4_SpaceCtrl_new|L4_SpaceCtrl_space_pager|
		L4_SpaceCtrl_prio(max_prio);
	if (L4_SpaceControl(space, ctrl, user_clist, utcb_area, resource, 
				NULL) != 1) {
		return L4_nilspace;
	}

	return space;
}

void
okl4_delete_space(L4_SpaceId_t space)
{
	struct okl4_bitmap_item bitmap_item;

	bitmap_item.unit = (uintptr_t)L4_SpaceNo(space);
	okl4_bitmap_allocator_free(spaceid_pool, &bitmap_item);
	if (L4_SpaceControl(space, L4_SpaceCtrl_delete, user_clist, L4_Nilpage,
				0, NULL) != 1) {
		panic("can't delete okl4 space");
	}
}

#if defined(ARM_PID_RELOC)
int
okl4_space_set_pid(L4_SpaceId_t space, unsigned int pid)
{
	word_t ctrl, resource;
	int ret;

	resource = pid;

	ctrl = L4_SpaceCtrl_resources;
	ret = L4_SpaceControl(space, ctrl, user_clist, utcb_area, resource, NULL);

#if defined(ARM_SHARED_DOMAINS)
	if (ret == 1) {
		L4_Fpage_t src_fpage;

		src_fpage = L4_FpageLog2((uintptr_t)pid * (1<<25), 25);
		L4_Set_Rights(&src_fpage, 7);
		L4_Set_Meta(&src_fpage);    // fault via callback

		L4_LoadMR(1, src_fpage.raw);

		L4_LoadMR(0, space.raw);
		ret = L4_MapControl(linux_space, L4_MapCtrl_MapWindow);
	}
#endif

	return (ret != 1);
}
#endif

#if defined(ARM_SHARED_DOMAINS)
#include <l4/arch/ver/space_resources.h>

void okl4_share_domain(L4_SpaceId_t space)
{
	word_t ctrl;
	int ret;

	ctrl = L4_SpaceCtrl_resources;

	L4_LoadMR(0, space.raw);
	ret = L4_SpaceControl(linux_space, ctrl, user_clist, utcb_area,
			L4_SPACE_RESOURCES_WINDOW_GRANT, NULL);
}

void okl4_unshare_domain(L4_SpaceId_t space)
{
	word_t ctrl;
	int ret;

	ctrl = L4_SpaceCtrl_resources;

	L4_LoadMR(0, space.raw);
	ret = L4_SpaceControl(linux_space, ctrl, user_clist, utcb_area,
			L4_SPACE_RESOURCES_WINDOW_REVOKE, NULL);
}
#endif

#endif
	
/*
 * This called from copy_process() as part of a fork. It creates
 * a new thread. It takes argumets in thread_info->request.op which we
 * setup in the base function requesting the fork. If a new user thread
 * is being created from another, the registers from the old thread
 * are copied to the new thread.
 */
int
copy_thread(int nr, unsigned long clone_flags, unsigned long usp,
	    unsigned long unused,  struct task_struct * p, 
	    struct pt_regs * regs)
{
	int r = 0;
	//printk("%s %d %lx usp=%lx, task=%p, regs=%p\n", __func__, nr, clone_flags, usp, p, regs);

	if ((task_thread_info(p)->request.op != OP_KTHREAD) &&
	    (current_thread_info()->user_tid.raw != L4_nilthread.raw)) {
		extern char __wombat_user_fork_handler[];
		L4_Word_t fork_start;
		void * utcb;

		if (utcb_area.raw == L4_Nilpage.raw)
		    utcb = (void*)-1UL;
		else
		    utcb = (void*) L4_Address(utcb_area);

                fork_start = TASK_SIG_BASE +
			(((L4_Word_t)&__wombat_user_fork_handler) & ~PAGE_MASK);

#if defined(CONFIG_CELL)
		task_thread_info(p)->user_tid = 
			okl4_create_thread(p->mm->context.space_id,
					main_thread, main_thread, utcb, 
					&task_thread_info(p)->user_handle);
#elif defined(CONFIG_IGUANA)
		task_thread_info(p)->user_tid =
			eas_create_thread(p->mm->context.eas,
					  L4_myselfconst, L4_myselfconst, utcb, &task_thread_info(p)->user_handle);
#endif

		if (task_thread_info(p)->user_tid.raw == L4_nilthread.raw)
		    return -ENOMEM;

		/*
		 * This will be disabled in libl4 with NDEBUG set, so it's OK.
		 */
		{
			char name[16];
			snprintf(name, 15, "L_%d", p->pid);
			L4_KDB_SetThreadName(task_thread_info(p)->user_tid, 
			    name);
		}

		r  = L4_Set_Priority(task_thread_info(p)->user_tid, 98);
		assert(r != 0);

		L4_Copy_regs (current_thread_info()->user_tid,
				   task_thread_info(p)->user_tid);

		task_thread_info(p)->request.u.fork.user_sp = usp ? usp : ARCH_sp(regs);
		task_thread_info(p)->request.u.fork.user_ip = ARCH_pc(regs);
		task_thread_info(p)->request.u.fork.user_start = fork_start;

#ifdef __i386_
#ifdef CONFIG_IA32_VDSO_ENABLE
		{
			/* 
			 * When we were called from a vsyscall (eg: sys_clone) 
			 * edi and ebp are pushed onto the stack.  We need to 
			 * copy from the parent's stack to the child's.
			 */
			unsigned long vsyscall_stack[8];

			if (usp && (void *)ARCH_pc(regs) == 
			    VDSO_SYM(&SYSENTER_RETURN)) {
				int size = sizeof(vsyscall_stack);

				if (copy_from_user(vsyscall_stack, 
				    (void *)ARCH_sp(regs), size))
					return -EFAULT;
				if (access_process_vm(p, task_thread_info(p)->request.u.fork.user_sp - size, vsyscall_stack, size, 1) != size)
					return -EFAULT;
				task_thread_info(p)->request.u.fork.user_sp -= size;
			}
		}
#endif	/*CONFIG_IA32_VDSO_ENABLE*/
#endif	/*__i386__*/

	} else {
		task_thread_info(p)->user_tid = L4_nilthread;
	}

	/* Setup kernel ip/sp */
	/* XXX - setup argument to new process handler */
	if (task_thread_info(p)->request.op == OP_KTHREAD) {
		task_thread_info(p)->request.op = OP_RESUME;

		/* Start new kernel thread - at new_thread_handler stub  */
		arch_setup_ipsp(task_thread_info(p), new_thread_handler,
				((THREAD_SIZE +
				 (unsigned long) task_thread_info(p) -
				 sizeof(long))) & ~0x7);
	} else {
		/* Start new user thread - at new_process_handler stub  */
		task_thread_info(p)->request.op = OP_RESUME;

		arch_setup_ipsp(task_thread_info(p), new_process_handler,
				((THREAD_SIZE +
				 (unsigned long) task_thread_info(p)-
				 sizeof(long))) & ~0x7);
	}

	return l4_arch_set_tls(clone_flags & CLONE_SETTLS, p, regs);
}


/*
 * XXX do more here? This is called from execve when the kernel
 * wants to clear all special state of a thread. eg FPU, debug
 * registers etc.
 */
void flush_thread (void) 
{ 
	//printk("flush_thread called\n"); 
} 

/*
 * Called after loading a new executeable, starts the thread
 * with the new IP and SP. May create a new user-thread on
 * demand if the process was a kernel thread.
 */
void
start_thread(struct pt_regs * regs, unsigned long pc, unsigned long sp)
{ 
	L4_ThreadId_t thrd;
	L4_ThreadId_t handle;
	//printk("%s current = %p  pc = %lx, sp = %lx\n", __func__, current, pc, sp);

	if (current_thread_info()->user_tid.raw == L4_nilthread.raw) {
		int r;
		void * utcb;

		if (utcb_area.raw == L4_Nilpage.raw)
		    utcb = (void*)-1UL;
		else
		    utcb = (void*) L4_Address(utcb_area);

		/*
		 * tls_bitmap:
		 *
		 * This is only currently used on the x86 where one
		 * can set multiple supported TLS entries.
		 */
		current_thread_info()->tls_bitmap = 0;
#if defined(CONFIG_CELL)
		thrd = okl4_create_thread(current->mm->context.space_id,
				main_thread, main_thread,
				utcb, &handle);
#elif defined(CONFIG_IGUANA)
                thrd = eas_create_thread( current->mm->context.eas,
				L4_myselfconst, L4_myselfconst,
				utcb, &handle);
#endif

		assert(thrd.raw != L4_nilthread.raw);
		r  = L4_Set_Priority(thrd, 98);
		assert(r != 0);
		/*
		 * This will be disabled in libl4 with NDEBUG set, so it's OK.
		 */
		{
			char name[16];
			snprintf(name, 15, "L_%d", current->pid);
			L4_KDB_SetThreadName(thrd, name);
		}
		current_thread_info()->user_tid = thrd;
		current_thread_info()->user_handle = handle;

	} else {
		thrd = current_thread_info()->user_tid;
	}
	regs = current_regs();

	ARCH_put_pc(regs, pc);
	ARCH_put_sp(regs, sp);

//	syscall_exit(1);
 
	/* zero the flag we use to indicate if 
	 * we're doing a syscall trace
	 */
	current_thread_info()->regs.strace_flag = 0;

	set_need_restart(current_thread_info(), pc, sp, 0);
	L4_AbortIpc_and_stop_Thread(thrd);
	set_user_ipc_cancelled(current_thread_info());
}


/*
 * Called from the Linux scheduler internals when switching
 * to a new thread.
 */
void*
__switch_to(struct task_struct *prev, struct task_struct *next)
{
	struct thread_info *prev_info = task_thread_info(prev);
	struct thread_info *next_info = task_thread_info(next);
#if 0
	printk("Switch from: %d to %d\n", prev->pid, next->pid);
#endif

	prev_info->request.op = OP_RESUME;

	local_save_flags(prev_info->irq_flags);

	if (next_info->request.op == OP_KTHREAD) {
		printk("KTHREAD\n");
		WARN_ON(1);
	} else if (next_info->request.op == OP_RESUME) {
#if defined(CONFIG_DIRECT_VM)
		int r;
		unsigned long resource;

		if (next->mm) {
			resource = next->mm->context.space_id.raw << 16;
		} else {
			resource = linux_space.raw << 16;
		}
		resource |= (1UL << 31);

		r = L4_SpaceControl(linux_space, L4_SpaceCtrl_resources,
				L4_ClistId(0), L4_Nilpage, resource, NULL);
#endif

		/* Current is set here because it is not stack based */
		/* XXX - is the race from here to the switch important? */
		current_tinfo(smp_processor_id()) = (unsigned long)next_info;

		prev_info = arch_switch(prev_info, next_info);
		mb();
	} else {
		printk("OTHER\n");
		WARN_ON(1);
	}

	next_info = current_thread_info();
	local_irq_restore(next_info->irq_flags);

	return prev_info->task;
}

/*
 * Called when creating a new kernel thread. Calls requested
 * function.
 */
static void
new_thread_handler(struct thread_info *prev)
{
	//printk("%s current = %p  prev = %p\n", __func__, current, prev->task);

	schedule_tail(prev->task);

	current_thread_info()->request.u.thread.proc(
			current_thread_info()->request.u.thread.arg);

	do_exit(0);
}


/*
 * L4 ExchangeRegisters wrapper
 */
L4_INLINE void L4_Stop_SetUser (L4_ThreadId_t t, L4_Word_t user)
{
	L4_Word_t dummy;

	(void) L4_ExchangeRegisters (t, L4_ExReg_Halt + L4_ExReg_AbortIPC + L4_ExReg_user,
				 0, 0, 0, user, L4_nilthread,
				 &dummy, &dummy, &dummy, &dummy, &dummy);
}

/*
 * Called when starting a new user-thread. Setup IP/SP and
 * call syscall loop.
 */
static void
new_process_handler(struct thread_info *prev)
{
//	struct pt_regs * regs;
//	printk("%s current = %p  prev = %p\n", __func__, current, prev->task);

	schedule_tail(prev->task);

	set_usermode_status_true(current_thread_info());

//	regs = current_regs();
//	ARCH_put_pc(regs, current_thread_info()->request.u.fork.user_ip);
//	ARCH_put_sp(regs, current_thread_info()->request.u.fork.user_sp);

//	syscall_exit(1);

	L4_Stop_SetUser(current_thread_info()->user_tid,
			current_thread_info()->request.u.fork.user_ip);
	set_need_restart(current_thread_info(),
		current_thread_info()->request.u.fork.user_start,
		current_thread_info()->request.u.fork.user_sp, 0);
	set_user_ipc_cancelled(current_thread_info());

	syscall_loop();
	/* Don't return */
	while (1);
}

void show_regs(struct pt_regs * regs)
{
	printk("show_regs() called\n");
}

#if defined(CONFIG_CELL)
__initcall(l4_process_init);
#endif

