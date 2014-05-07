/* 
 * Copyright (C) 2002 Jeff Dike (jdike@karaya.com)
 * Licensed under the GPL
 */

#ifndef __L4_THREAD_INFO_H
#define __L4_THREAD_INFO_H

#ifndef __ASSEMBLY__
#include "l4.h"
#include <asm/processor.h>
#include <asm/percpu.h>
#include <asm/macros.h>
#include <asm/regs.h>
#include <linux/smp.h>

#include INC_SYSTEM2(context.h)

#define PREEMPT_ACTIVE             0x10000000

struct thread_info {
	L4_ThreadId_t           user_tid;	/* user process L4-thread Id */
	L4_ThreadId_t           user_handle;	/* handle to check incoming IPC tid with read tid */

	struct task_struct	*task;		/* main task structure */
	struct exec_domain	*exec_domain;	/* execution domain */

	unsigned long		flags;		/* low level flags */
	unsigned long		irq_flags;	/* irq flags over context switch */
	__u32			cpu;		/* current CPU */
	__s32			preempt_count;  /* 0 => preemptable, <0 => BUG */

	struct restart_block    restart_block;

	struct arch_kernel_context	context;/* architecture independent kernel context */
	struct pt_regs		regs;		/* user process saved registers */
	L4_MsgTag_t		tag;		/* user process L4 tag */
	/*
	 * The tls bitmap is only used on the i386 architecture
	 * at this point in theory could be used to implement 
 	 * multiple slots for the thread local storage area.
	 *
	 * -gl
	 */
	unsigned long		tls_bitmap;
	unsigned long		tp_value;

#define OP_NONE		0
#define OP_FORK		1
#define OP_KTHREAD	2
#define OP_DELETE	3
#define OP_RESUME	4
	struct {
		long op;
		union {
			struct {
				L4_Word_t user_ip;
				L4_Word_t user_sp;
				L4_Word_t user_start;
			} fork, exec;
			struct {
				int (*proc)(void *);
				void *arg;
			} thread;
			struct {
				void (*proc)(void *);
				void *arg;
			} cb;
		} u;
	} request;
	struct {
		L4_Word_t user_ip;
		L4_Word_t user_sp;
		L4_Word_t user_flags;
	} restart;
};


#define INIT_THREAD_INFO(tsk)			\
{						\
	task:		&tsk,			\
	exec_domain:	&default_exec_domain,	\
	flags:		_TIF_USER_MODE,		\
	cpu:		0,			\
	preempt_count:	1,			\
	restart_block:  {			\
		fn:  do_no_restart_syscall,	\
	},					\
	regs: {					\
		mode:	1,			\
	},					\
}

#define init_thread_info	(init_thread_union.thread_info)
#define init_stack		(init_thread_union.stack)

extern DEFINE_PER_CPU(unsigned long, _l4_current_tinfo);
#define current_tinfo(cpu) (per_cpu(_l4_current_tinfo, cpu))

static inline struct thread_info *current_thread_info(void)
{
	return (struct thread_info *) current_tinfo(smp_processor_id());
}

static inline struct pt_regs *current_regs(void)
{
	return &current_thread_info()->regs;
}

/* thread information allocation */
#define THREAD_SIZE (4*PAGE_SIZE)
#define alloc_thread_info(tsk) ((struct thread_info *) \
	__get_free_pages(GFP_KERNEL,2))
#define free_thread_info(ti) free_pages((unsigned long) (ti), 2)
#define get_thread_info(ti) get_task_struct((ti)->task)
#define put_thread_info(ti) put_task_struct((ti)->task)

#endif

/* Thread information flags */

#define TIF_SYSCALL_TRACE	0	/* syscall trace active */
#define TIF_SINGLESTEP		4	/* single step */
#define TIF_SIGPENDING		1	/* signal pending */
#define TIF_NEED_RESCHED	2	/* rescheduling necessary */
#define TIF_POLLING_NRFLAG      3       /* true if poll_idle() is polling 
					 * TIF_NEED_RESCHED 
					 */
#define TIF_USER_OR_KERNEL	4	/* Should we access user or kernel memory
					 * (USER_DS/KERNEL_DS) */
#define TIF_DONT_REPLY_USER	8	/* Did we cancel the user's IPC? */
#define TIF_USER_MODE		9	/* This is set when entering user mode
					 * and cleared when handling a kernel
					 * fault (eg vmalloc'ed page) */
#define TIF_MEMDIE		10	/* OOM killer killed process */
#define	TIF_USER_RESTART	11	/* need to restart thread - new ip/sp */

#define _TIF_SYSCALL_TRACE	(1 << TIF_SYSCALL_TRACE)
#define _TIF_SIGPENDING		(1 << TIF_SIGPENDING)
#define _TIF_NEED_RESCHED	(1 << TIF_NEED_RESCHED)
#define _TIF_POLLING_NRFLAG	(1 << TIF_POLLING_NRFLAG)

#define _TIF_USER_OR_KERNEL	(1 << TIF_USER_OR_KERNEL)
#define _TIF_DONT_REPLY_USER	(1 << TIF_DONT_REPLY_USER)
#define _TIF_USER_MODE		(1 << TIF_USER_MODE)

#define _TIF_USER_RESTART	(1 << TIF_USER_RESTART)

#define user_mode_status(p)	(p->flags & _TIF_USER_MODE)
	
static inline void
set_usermode_status(struct thread_info *p, unsigned long mode)
{
	if (mode)
		set_bit(TIF_USER_MODE, &p->flags);
	else
		clear_bit(TIF_USER_MODE, &p->flags);
}

#define set_usermode_status_true(p)		\
		set_bit(TIF_USER_MODE, &p->flags)
#define set_usermode_status_false(p)	\
		clear_bit(TIF_USER_MODE, &p->flags)

#define set_user_ipc_cancelled(p)	\
	    {	\
		set_bit(TIF_DONT_REPLY_USER, &p->flags);	\
	    }
		
#define set_user_ipc_received(p)		\
		clear_bit(TIF_DONT_REPLY_USER, &p->flags)
#define reply_user_ipc(p)		\
		likely(!(p->flags & _TIF_DONT_REPLY_USER))

#define set_need_restart(p, ip, sp, uflags)	\
{						\
	set_bit(TIF_USER_RESTART, &p->flags);	\
	p->restart.user_ip = ip;		\
	p->restart.user_sp = sp;		\
	p->restart.user_flags = uflags;		\
}

#define clear_need_restart(p)			\
	clear_bit(TIF_USER_RESTART, &p->flags)

#define user_need_restart(p)			\
	unlikely(p->flags & _TIF_USER_RESTART)

int l4_arch_set_tls(int, struct task_struct *p, struct pt_regs *);

#endif

/*
 * Overrides for Emacs so that we follow Linus's tabbing style.
 * Emacs will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-file-style: "linux"
 * End:
 */
