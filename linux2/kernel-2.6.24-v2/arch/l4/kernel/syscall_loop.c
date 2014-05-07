#include "l4.h"
#include "assert.h"



#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/kernel.h>
#include <linux/bootmem.h>
#include <linux/fs.h>
#include <linux/root_dev.h>
#include <linux/initrd.h>
#include <linux/highmem.h>
#include <linux/file.h>
#include <linux/module.h>

#include <asm/page.h>
#include <asm/uaccess.h>
#include <asm/syscalls.h>
#include <asm/signal.h>
#include <asm/signal_l4.h>
#include <asm/tlbflush.h>
#include <asm/setup.h>
#include <asm/okl4.h>

#if defined(CONFIG_IGUANA)
#include <iguana/thread.h>
#include <iguana/cap.h>
#include <iguana/memsection.h>
#include <iguana/tls.h>
#include <iguana/object.h>
#endif

#include <linux/mman.h>

#include INC_SYSTEM2(exception.h)
#include INC_SYSTEM2(syscalls_inline.h)

extern L4_ThreadId_t timer_thread;
extern L4_ThreadId_t timer_handle;
extern L4_ThreadId_t main_thread;

extern unsigned long get_instr(unsigned long addr);
extern unsigned long set_instr(unsigned long addr, unsigned long value);

/*
 * This routine handles page faults.  It determines the address,
 * and the problem, and then passes it off to one of the appropriate
 * routines.
 *
 * error_code:
 *	bit 0 == 0 means no page found, 1 means protection fault
 *	bit 1 == 0 means read, 1 means write
 *	bit 2 == 0 means kernel, 1 means user-mode
 *
 * adjusted for l4/linux. do_pagefault changed to l4_do_pagefault
 * which simply:
 *	- calls handle_m_fault if there is a valid vma,
 *	- forces a signal if the page fault was raised by a user,
 *	  calls generate_fake_interrupt to force the user into the
 *	  kernel (it still is in an ipc waiting for the reply to its
 *	  page fault) and returns (-1) to signal the error condition.
 *	- returns (-1) if the pagefault happens within the kernel
 *	  context and leaves the rest to the calling function (should
 *	  be a uacess function which in turn will return an EFAULT to
 *	  its calling function)
 *
 */
/* XXX Note, this fuction does not currently handle faults from
 * vmalloc/vmaped'd memory. That should probably be in a separate
 * function anyway.
 */
int
l4_do_page_fault(unsigned long address, long access, struct pt_regs *regs)
{
	struct vm_area_struct * vma;
	struct mm_struct *mm = current->mm;
	int fault, si_code = SEGV_MAPERR;
	siginfo_t info;

	/* If we're in an interrupt context, or have no user context,
	   we must not take the fault.  */
	if (!mm) /* || in_interrupt()) */
		goto bad_area_nosemaphore;

	down_read(&mm->mmap_sem);
	vma = find_vma(mm, address);
	if (!vma)
		goto bad_area;
	if (vma->vm_start <= address)
		goto good_area;
	if (!(vma->vm_flags & VM_GROWSDOWN))
		goto bad_area;
	if (expand_stack(vma, address))
		goto bad_area;

	/* Ok, we have a good vm_area for this memory access, so
	   we can handle it.  */
 good_area:
	si_code = SEGV_ACCERR;
	if (/* LOAD */ access & 0x4) {
		/* Allow reads even for write/execute-only mappings */
		if (!(vma->vm_flags & (VM_READ | VM_WRITE | VM_EXEC)))
			goto bad_area;
	} else if (/* FETCH */ access & 0x1) {
		if (!(vma->vm_flags & VM_EXEC))
			goto bad_area;
	} else {
		if (!(vma->vm_flags & VM_WRITE))
			goto bad_area;
	}

 survive:
	/* If for any reason at all we couldn't handle the fault,
	   make sure we exit gracefully rather than endlessly redo
	   the fault.  */

	fault = handle_mm_fault(mm, vma, address, access & 0x2);
	up_read(&mm->mmap_sem);

	switch (fault) {
		case VM_FAULT_MINOR:
			current->min_flt++;
			break;
 		case VM_FAULT_MAJOR:
			current->maj_flt++;
			break;
		case VM_FAULT_SIGBUS:
			goto do_sigbus;
		case VM_FAULT_OOM:
			goto out_of_memory;
#if 0
	/*
	 * Well, it's a good idea to have this here, but apparently
	 * handle_mm_fault() can return all sorts of weird stuff, which
	 * makes it unsuitable to put BUG() here. 	-gl
	 */
	      default:
		BUG();
#endif
	}
	return 0;

	/* Something tried to access memory that isn't in our memory map.
	   Fix it, but check if it's kernel or user first.  */
bad_area:
	up_read(&mm->mmap_sem);
	/* Check if it is at TASK_SIG_BASE */
#ifdef CONFIG_ARCH_ARM
	/*
	 * Binary patching for NPTL
	 *
	 * XXX ??? Better place this thing?
	 */
	if (user_mode(regs) && ((address & PAGE_MASK) == 0xffff0000)) {
#if 0
		printk("Fault at address 0x%lx pc = 0x%lx, "
		    "need rewrite\n", address, L4_MsgWord(&current_regs()->msg, 1));
#endif
		if (address == 0xffff0fe0) {
			L4_Msg_t msg;
			unsigned long pc = L4_MsgWord(&current_regs()->msg, 1);
			unsigned long lr, fpc;
			unsigned long instr, r;
			long offs;

			if (pc != 0xffff0fe0)
				goto bad_area_nosemaphore;

			L4_Copy_regs_to_mrs(task_thread_info(current)->user_tid);
			L4_StoreMRs(0, 16, &msg.msg[0]);
			lr = msg.msg[14];
			fpc = lr - 4;

			L4_CacheFlushAll();
			instr = get_instr(fpc);
			if (instr == -1UL) 
				goto bad_area_nosemaphore;

			if ((instr & 0x0f000000) == 0x0b000000) {
				offs = instr << 8;
				offs = offs >> 6;	/* ASR */

				fpc = (fpc + 8) + offs;
				instr = get_instr(fpc);
				if (instr == -1UL)
					goto bad_area_nosemaphore;

				if ((instr & 0xffffffff) == 0xe3e00a0f) {
					/* mvn r0, 0xf000 */

					/*
					 * Rewrite to load the 
					 * kernel_reserved[0] from the
				 	 * utcb.
					 *
					 * This requires L4 to cooperate
					 * with the ExReg() syscall.
					 */
					/* mov r0, #0xff000000 */
					r = set_instr(fpc, 0xe3a004ff);
					if (r == -1UL)
						goto bad_area_nosemaphore;
					fpc += 4;

					/* ldr r0, [r0, #0xff0] */
					r = set_instr(fpc, 0xe5900ff0);
					if (r == -1UL)
						goto bad_area_nosemaphore;
					fpc += 4;

					/* ldr r0, [r0, #56] */
					r = set_instr(fpc, 0xe5900038);
					if (r == -1UL)
						goto bad_area_nosemaphore;
					fpc += 4;

					/* mov pc, lr */
					r = set_instr(fpc, 0xe1a0f00e);
					if (r == -1UL) 
						goto bad_area_nosemaphore;
					L4_CacheFlushAll();

					msg.msg[0] = current_thread_info()->tp_value;
					msg.msg[15] = lr;
					L4_LoadMRs(0, 16, &msg.msg[0]);
					L4_Copy_mrs_to_regs(
					    task_thread_info(current)->user_tid);
					L4_MsgPutWord(&current_regs()->msg, 1,
					    lr);
					return 0;
				}
			} else if (instr == 0xe240f01f) {
				/*
				 * sub pc, r0, #31 
				 */
				fpc = fpc - 8;
				instr = get_instr(fpc);
				if (instr == -1UL)
					goto bad_area_nosemaphore;
				/* mvn r0, 0xf000 */
				if (instr != 0xe3e00a0f)
					goto bad_area_nosemaphore;
				instr = get_instr(fpc + 4);
				if (instr == -1UL)
					goto bad_area_nosemaphore;
				/* mv lr, pc */
				if (instr != 0xe1a0e00f)
					goto bad_area_nosemaphore;

				/* mov r0, #0xff000000 */
				r = set_instr(fpc, 0xe3a004ff);
				if (r == -1UL)
					goto bad_area_nosemaphore;
				/* ldr, r0, [r0, #0xff0] */
				r = set_instr(fpc + 4, 0xe5900ff0);
				if (r == -1UL)
					goto bad_area_nosemaphore;
				/* ldr  r0, [r0, #56] */
				r = set_instr(fpc + 8, 0xe5900038);
				if (r == -1UL)
					goto bad_area_nosemaphore;
				L4_CacheFlushAll();

				msg.msg[15] = fpc;
				L4_LoadMRs(0, 16, &msg.msg[0]);
				L4_Copy_mrs_to_regs(
				     task_thread_info(current)->user_tid);
				L4_MsgPutWord(&current_regs()->msg, 1, fpc);
				return 0;
			}
		}
		if (address == 0xffff0fc0) {
			L4_Msg_t msg;
			unsigned long pc = L4_MsgWord(&current_regs()->msg, 1);
			unsigned long lr, fpc;
			unsigned long instr, r;

			if (pc != 0xffff0fc0)
				goto bad_area_nosemaphore;

			L4_Copy_regs_to_mrs(task_thread_info(current)->user_tid);
			L4_StoreMRs(0, 16, &msg.msg[0]);
			lr = msg.msg[14];
			fpc = lr - 4;

			L4_CacheFlushAll();
			instr = get_instr(fpc);
			if (instr == -1UL)
				goto bad_area_nosemaphore;
			/* sub pc, r3, #63 */
			if (instr != 0xe243f03f)
				goto bad_area_nosemaphore;
			fpc = fpc - 8;
			instr = get_instr(fpc);
			if (instr == -1UL)
				goto bad_area_nosemaphore;
			/* mvn r3, 0xf000 */
			if (instr != 0xe3e03a0f)
				goto bad_area_nosemaphore;
			/* mv lr, pc */
			instr = get_instr(fpc + 4);
			if (instr == -1UL)
				goto bad_area_nosemaphore;
			if (instr != 0xe1a0e00f)
				goto bad_area_nosemaphore;
#ifdef ARM_PID_RELOC
			/* mov r3, #0x1fc0000 */
			r = set_instr(fpc, 0xe3a0377f);
			if (r == -1UL)
				goto bad_area_nosemaphore;
			/* mov lr, pc */
			r = set_instr(fpc + 4, 0xe1a0e00f);
			if (r == -1UL)
				goto bad_area_nosemaphore;
			/* orr pc, r3, #0x3a400 */
			r = set_instr(fpc + 8, 0xe383fbe9);
			if (r == -1UL)
				goto bad_area_nosemaphore;
#else
			/* mov r3, #0x98000000 */
			r = set_instr(fpc, 0xe3a03326);
			if (r == -1UL)
				goto bad_area_nosemaphore;
			/* mov lr, pc */
			r = set_instr(fpc + 4, 0xe1a0e00f);
			if (r == -1UL)
				goto bad_area_nosemaphore;
			/* orr pc, r3, #0x400 */
			r = set_instr(fpc + 8, 0xe383fb01);
			if (r == -1UL)
				goto bad_area_nosemaphore;
#endif
			L4_CacheFlushAll();

			msg.msg[15] = fpc;
			L4_LoadMRs(0, 16, &msg.msg[0]);
			L4_Copy_mrs_to_regs(task_thread_info(current)->user_tid);
			L4_MsgPutWord(&current_regs()->msg, 1, fpc);
			return 0;
		}
	}
#endif
	if (user_mode(regs) && ((address & PAGE_MASK) == TASK_SIG_BASE) &&
			(access & 0x1/* Execute */)) {
		L4_Fpage_t fpage;
		extern char __user_exregs_page[];
		extern void __wombat_user_sig_fault(void);

		/* Map the exregs page for fork/signal handling fixup into
		 * user address space */
		fpage = (L4_Fpage_t) (L4_FpageLog2(
			    (L4_Word_t)&__user_exregs_page, PAGE_SHIFT).raw + 0x1);

#ifdef ARM_PID_RELOC
		if (address < 0x2000000UL) {
			/* PID relocate */
			L4_Word_t pid = (unsigned long)current->mm->context.pid << 25;
			address += pid;
		}
#endif
#if defined(CONFIG_CELL)
		okl4_map_page(&mm->context, address, 
			(L4_Word_t)&__user_exregs_page, 
			L4_Readable|L4_eXecutable, L4_DefaultMemory);
#elif defined(CONFIG_IGUANA)
		l4_map_page(&mm->context, fpage, address, L4_DefaultMemory);
#endif

		return 0;
	}

bad_area_nosemaphore:
	if (user_mode(regs))
		goto do_sigsegv;
	return -1;

/* We only enter here if from kernel access to user memory */
no_context:
	return -1;

	/* We ran out of memory, or some other thing happened to us that
	   made us unable to handle the page fault gracefully.  */
 out_of_memory:
	printk("OUT OF MEMORY!\n");
	if (current->pid == 1) {
		yield();
		down_read(&mm->mmap_sem);
		goto survive;
	}
	printk(KERN_ALERT "VM: killing process %s(%d)\n",
	       current->comm, current->pid);
	if (!user_mode(regs))
		goto no_context;
	do_exit(SIGKILL);

 do_sigbus:
	/* Send a sigbus, regardless of whether we were in kernel
	   or user mode.  */
	info.si_signo = SIGBUS;
	info.si_errno = 0;
	info.si_code = BUS_ADRERR;
	info.si_addr = (void *) address;
	force_sig_info(SIGBUS, &info, current);
	if (!user_mode(regs))
		goto no_context;
	return -1;

 do_sigsegv:
#if 0
	printk("SIGSEGV(%d,%lx): @ %p  ip=%p\n", current->pid,
			task_thread_info(current)->user_tid.raw,
			(void*)address,
			(void*)L4_MsgWord (&current_regs()->msg, 1));
	L4_KDB_Enter("SIGSEGV");
#endif
	info.si_signo = SIGSEGV;
	info.si_errno = 0;
	info.si_code = si_code;
	info.si_addr = (void *) address;
	force_sig_info(SIGSEGV, &info, current);
	return -1;
}

#define L4_PAGEFAULT		((-2ul<<20)>>20)
#define L4_PREEMPTION		((-3ul<<20)>>20)
#define L4_ARCH_EXCEPTION	((-5ul<<20)>>20)

/*
 * Main system call loop which receives IPC messages from user thread
 * and dispatches system calls / pagefault handling / exception handling.
 */
void NORET_TYPE
syscall_loop (void)
{
	L4_Word_t syscall = 0;
	struct thread_info * curinfo = current_thread_info();

	L4_Accept(L4_NotifyMsgAcceptor);
	L4_Set_NotifyMask(L4_PREEMPT);

	goto return_to_user;

	for(;;) {
		isr_irq_enable();   // don't handle interrupt here
		set_user_ipc_received(curinfo);

	restart_syscall:
#if 0
		printk ("syscall_loop: got msg from %p %lx, (0x%p)\n", 
			(void *) curinfo->user_tid.raw,
			(long) L4_Label (curinfo->tag),
			//(void *) L4_MsgWord (&curinfo->regs->msg, 0));
			(void *) L4_MsgWord (&curinfo->regs.msg, 0));
#endif
		/*
		 * Dispatch IPC according to protocol.
		 */
		syscall = 0;
		switch (L4_Label(curinfo->tag) >> 4) {
		case L4_PAGEFAULT:
			{
				/* A pagefault occured. Dispatch to the pager */
				L4_Word_t addr = L4_MsgWord (&curinfo->regs.msg, 0);
#ifdef ARM_PID_RELOC
				L4_Word_t offset = (unsigned long)current->mm->context.pid << 25;

				if (addr >= offset && 
				    (addr < (offset+0x2000000))) {
					addr -= offset;
				}
#endif
				/* Page table lookup */
				l4_do_page_fault(addr, L4_Label(curinfo->tag) & 0xf, &curinfo->regs);
			}
			break;
		/* Trampoline */
		/* Direct syscall */
		case L4_ARCH_EXCEPTION:
			if (L4_ARCH_IS_SYSCALL(curinfo->tag)) {
				L4_Word_t abi, result;
                                long sys_num;
                                
#ifdef __i386__
#ifdef CONFIG_IA32_VDSO_ENABLE
				/* 
				 * For ia32, determine if the syscall was a 
				 * fast (sysenter) syscall.
                                 * If so, obtain the extra args from the 
				 * stack.
				 */
				if (L4_I386_IS_SYSCALL_FAST(curinfo->tag))
					l4_i386_get_fast_syscall_args(
					    &curinfo->regs);
#endif	/*CONFIG_IA32_VDSO_ENABLE*/
#endif	/*__i386__*/
				sys_num = l4_arch_lookup_syscall(&curinfo->regs, &abi);

				if (sys_num > 0) {
					assert(l4_syscall_table[sys_num].fn);

					/*
					 * Set the syscall variable to 
				 	 * the system number.  This 
					 * is OK because all OK Linux
					 * system call numbers are positive,
					 * then we can tell not only where
					 * we came from but also which 
					 * system call was invoked.
					 */
					syscall = sys_num;
					syscall_entry(curinfo);

					result = l4_arch_abi_call(&curinfo->regs, sys_num, abi);
#if 0
					printk("Got a syscall: (%3ld, %2ld) from %3d (%s)  -> %ld\n",
						sys_num, abi, current->pid, current->comm, (signed long)result);
#endif
				} else {
					printk("%s:%d:%lx unknown syscall\n",
							current->comm, current->pid,
							curinfo->user_tid.raw);
					force_sig(SIGILL, current);
				}
			} else if (L4_ARCH_IS_EXCEPTION(curinfo->tag)) {
				l4_arch_handle_exception(&curinfo->regs);
			} else {
				printk("%s:%d:%lx unknown exception message received: ",
						current->comm, current->pid,
						curinfo->user_tid.raw);
				printk("  tag = %lx\n", curinfo->tag.raw);
				force_sig(SIGILL, current);
			}
			break;
		default:
			printk ("%s:%d:%lx unknown ipc request, "
				"(%p, %p, %p)\n",
				current->comm, current->pid,
				curinfo->user_tid.raw,
				(void *) curinfo->tag.raw,
				(void *) L4_MsgWord (&curinfo->regs.msg, 0),
				(void *) L4_MsgWord (&curinfo->regs.msg, 1));

			force_sig(SIGILL, current);
			break;
		}
handle_signal:
		l4_work_pending(curinfo, syscall, &curinfo->regs);

		if (syscall && l4_arch_restart_syscall(&curinfo->regs)) {
#if 0
			printk("%s:%d:%lx restart syscall\n",
					current->comm, current->pid,
					curinfo->user_tid.raw);
#endif
			goto restart_syscall;
		}

return_to_user:
		syscall_exit(curinfo, syscall);

		local_irq_enable();

		/* Code which replies/accepts  IPCs (syscalls) from user processes */
		{
			L4_MsgTag_t tag;
			L4_ThreadId_t from;

			if (likely(reply_user_ipc(curinfo))) {
				curinfo->regs.mode = 0;
				L4_MsgLoad(&curinfo->regs.msg);
				/* Reply to caller and wait for next IPC */
				tag = L4_ReplyWait(curinfo->user_tid,
						&from);
			} else {
retry:
				if (user_need_restart(curinfo)) {
					L4_Start_SpIpFlags(curinfo->user_tid,
							curinfo->restart.user_sp,
							curinfo->restart.user_ip,
							curinfo->restart.user_flags);
					clear_need_restart(curinfo);
				}
				curinfo->regs.mode = 0;
				/* Open Wait */
				tag = L4_Wait (&from);
			}

			if (likely(from.raw == curinfo->user_handle.raw))
			{
				curinfo->regs.mode = 1;
				curinfo->tag = tag;
			}
			else if (from.raw == L4_nilthread.raw)
			{
				curinfo->regs.mode = 1;
				/* Preemption */
				l4_work_pending_preempt(&curinfo->regs);    /* Handle any signals / reschedules */
				goto retry;
			}
			else
			{
				/* XXX
				 * Here we have to lookup the linux thread from
				 * the L4 thread_id, which may be expensive.
				 * Probably someone did a ThreadSwitch to this thread.
				 * We cannot kill it because an unpriviledged linux
				 * thread could use this to kill root processes.
				 */
				printk("illegal ipc from %lx, expected %lx\n", from.raw,
						curinfo->user_handle.raw);
				L4_KDB_Enter("ill");
				goto retry;
			}
		}

		if (unlikely(L4_IpcFailed(curinfo->tag))) {
			L4_Word_t ec = L4_ErrorCode();

			printk("reply_wait: (%p) IPC error (%p) %lx\n",
				(void *) main_thread.raw, 
				(void *) curinfo->user_tid.raw, ec);

			force_sig(SIGILL, current);
			syscall = 0;
			goto handle_signal;
		}

		L4_MsgStore (curinfo->tag, &curinfo->regs.msg); /* Get the tag */
	}
	BUG();
}
