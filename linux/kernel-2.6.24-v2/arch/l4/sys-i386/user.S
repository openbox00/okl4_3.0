/*********************************************************************
 *                
 * Copyright (C) 2004,  National ICT Australia
 *                
 ********************************************************************/

#include <asm-l4/asm-offsets.h>
#include <asm-x86/errno.h>
#include <asm-x86/unistd.h>
#include <l4/arch/vregs_asm.h>

	.section .exregspage
	.global __wombat_user_sig_fault;
__wombat_user_sig_fault:
	ret

	.global __wombat_user_fork_handler;
/*
 * OK - fp state need not be preserved across fork() call
 */
__wombat_user_fork_handler:
	/*
	 * Get the UTCB pointer, offset via %gs.  We need to 
	 * save the previous value of %gs as it is preserved across
	 * the fork() system call.
	 */
	push	%gs
	movw	$L4_UTCB, %ax
	movw	%ax,	%gs	/* save %gs value */
	movl	%gs:0,	%eax	/* Get utcb pointer */
	movl	(__L4_TCR_USER_DEFINED_HANDLE*4)(%eax), %eax	/* Load user defined handle */
	/*
	 * XXX What about eflags?  Do we need to preserve that? -gl
	 */
	add	$2,	%eax	/* handle = fork-ip - 2, so add 2 */
	pop	%gs		/* restore previous %gs value */
	pushl	%eax		/* store on the stack so we can clear eax */
	movl	$0,	%eax
	ret			/* Jump to fork-ip */

	.global __wombat_user_sigentry_int;
__wombat_user_sigentry_int:
	movl	$-EINTR,    %eax

	.global __wombat_user_sigentry_restart;
__wombat_user_sigentry_restart:
	movl	%eax, L4_SIGFRAME_EAX_OFFSET(%esp)

	.global __wombat_user_sigentry;
__wombat_user_sigentry:
	movl	%edi, L4_SIGFRAME_EDI_OFFSET(%esp)
	movl	%esi, L4_SIGFRAME_ESI_OFFSET(%esp)
	movl	%ebp, L4_SIGFRAME_EBP_OFFSET(%esp)
	movl	%ebx, L4_SIGFRAME_EBX_OFFSET(%esp)
	movl	%ecx, L4_SIGFRAME_ECX_OFFSET(%esp)
	movl	%edx, L4_SIGFRAME_EDX_OFFSET(%esp)
	fxsave	L4_SIGFRAME_FPU_OFFSET(%esp)
	fninit

	jmpl	*L4_SIGFRAME_SIGIP_OFFSET(%esp)

/*
 * Yes, this is a discrepency between the user_sigreturn and the rt_sigreturn.
 * the reason is because the rt version offsets the stack pointer by 4 while
 * the vanilla one offset it by 8 in the Linux kernel.  So we have to 
 * change the stack to appease it..
 * 
 * -gl
 */
	.global __wombat_user_sigreturn;
__wombat_user_sigreturn:
	popl %eax		/* XXX does this mean it needs unwind info? */
	movl $__NR_sigreturn, %eax
	int $0x80

	.global __wombat_user_rt_sigentry_int;
__wombat_user_rt_sigentry_int:
	movl	$-EINTR,    %eax

	.global __wombat_user_rt_sigentry_restart;
__wombat_user_rt_sigentry_restart:
	movl	%eax, L4_SIGRTFRAME_EAX_OFFSET(%esp)

	.global __wombat_user_rt_sigentry;
__wombat_user_rt_sigentry:
	movl	%edi, L4_SIGRTFRAME_EDI_OFFSET(%esp)
	movl	%esi, L4_SIGRTFRAME_ESI_OFFSET(%esp)
	movl	%ebp, L4_SIGRTFRAME_EBP_OFFSET(%esp)
	movl	%ebx, L4_SIGRTFRAME_EBX_OFFSET(%esp)
	movl	%ecx, L4_SIGRTFRAME_ECX_OFFSET(%esp)
	movl	%edx, L4_SIGRTFRAME_EDX_OFFSET(%esp)
	fxsave	L4_SIGRTFRAME_FPU_OFFSET(%esp)
	fninit

	jmpl	*L4_SIGRTFRAME_SIGIP_OFFSET(%esp)

	.global __wombat_user_rt_sigreturn;
__wombat_user_rt_sigreturn:
	movl $__NR_rt_sigreturn, %eax
	int $0x80

	.global __wombat_user_sigrestore;
__wombat_user_sigrestore:
	/*
	 * Restore the FPU
	 */
	mov	L4_RESTORE_FPU_PTR_OFFSET(%esp), %eax
	fxrstor	L4_FPUSTATE_FPU_OFFSET(%eax)
	/*
	 * Change the restored stack pointer so that
	 * the restored stack pointer is actually off by 4 bytes.
	 * This is because we use the ret instruction to return,
	 * to make the restoring of the stack and the return to 
	 * previous execution atomic.  In native Linux this is done
	 * in the kernel which gurantees atomicity wrt this 
	 * op.
	 *
	 * Here, we get the stack pointer, sub 4.  Because subl may
	 * change flags so we save and restore eflags for this operation.
	 * The esp offset is off by 4 to compensate for the pushf.  Thanks
	 * to Hal for noticing this problem.
	 *
	 * -gl
	 */
#if 1
	pushf
	subl	$4, (4+L4_RESTORE_ESP_OFFSET)(%esp)
	popf
#endif

	movl	L4_RESTORE_EDI_OFFSET(%esp), %edi
	movl	L4_RESTORE_ESI_OFFSET(%esp), %esi
	movl	L4_RESTORE_EBP_OFFSET(%esp), %ebp
	movl	L4_RESTORE_EAX_OFFSET(%esp), %eax
	movl	L4_RESTORE_EBX_OFFSET(%esp), %ebx
	movl	L4_RESTORE_ECX_OFFSET(%esp), %ecx
	movl	L4_RESTORE_EDX_OFFSET(%esp), %edx
                                                 
	movl	L4_RESTORE_ESP_OFFSET(%esp), %esp

	/*
	 * The restoring of the stack and the instruction pointer
	 * must be atomic, offseting -4(%esp) means that 
	 * the return ip can be subsequently trashed.  We can't
	 * trash the eflags, either, so we do in two steps.  First
	 * we decrement the stack at the very beginning of the 
	 * trampoline by 4 to indicate that the stack is actually
	 * not the stack that we wanted but includes instruction pointer.
	 * Second at the last bit instead of using a jmp, we use
	 * a ret that does tmp = (sp), sp = sp + 4, jmp (tmp).
	 *
	 * -gl
	 */
#if 0
	jmpl	*L4_RESTORE_FRAME_EPC_OFFSET(%esp)
#endif
	ret

