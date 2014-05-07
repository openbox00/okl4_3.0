/*********************************************************************
 *                
 * Copyright (C) 2004,  National ICT Australia
 *                
 ********************************************************************/

#include <asm/macros.h>
#include <asm/errno.h>
#include <asm-l4/asm-offsets.h>
#include <l4/arch/vregs_asm.h>

	.section .exregspage
	.global __wombat_user_sig_fault;
__wombat_user_sig_fault:
	mov	pc, lr

	.global __wombat_user_fork_handler;
__wombat_user_fork_handler:
	mov	r0,	#0xff000000	/* Get KIP pointer localtion */
	ldr	r0,	[r0, #0xff0]	/* Get UTCB pointer */
	ldr	r0,	[r0, #__L4_TCR_USER_DEFINED_HANDLE*4]	/* User defined handle */
	str	r0,	[sp, #-4]!
	mov	r0,	#0		/* Clear r0 as per spec */
	ldr	pc,	[sp], #4 	/* Jump to fork-ip */

	.global __wombat_user_sigentry_int;
__wombat_user_sigentry_int:
	mov	r0,	#-EINTR

	.global __wombat_user_sigentry_restart;
__wombat_user_sigentry_restart:
	str	r0,	[sp, #L4_SIGFRAME_R0_OFFSET]

	.global __wombat_user_sigentry;
__wombat_user_sigentry:
	str	r1,	[sp, #L4_SIGFRAME_R1_OFFSET]
	str	r2,	[sp, #L4_SIGFRAME_R2_OFFSET]
	str	r3,	[sp, #L4_SIGFRAME_R3_OFFSET]
	str	r4,	[sp, #L4_SIGFRAME_R4_OFFSET]
	str	r5,	[sp, #L4_SIGFRAME_R5_OFFSET]
	str	r6,	[sp, #L4_SIGFRAME_R6_OFFSET]
	str	r7,	[sp, #L4_SIGFRAME_R7_OFFSET]
	str	r8,	[sp, #L4_SIGFRAME_R8_OFFSET]
	str	r9,	[sp, #L4_SIGFRAME_R9_OFFSET]
	str	r10,	[sp, #L4_SIGFRAME_R10_OFFSET]
	str	r11,	[sp, #L4_SIGFRAME_R11_OFFSET]
	str	r12,	[sp, #L4_SIGFRAME_R12_OFFSET]
	str	lr,	[sp, #L4_SIGFRAME_LR_OFFSET]

	ldr	r0,	[sp, #L4_SIGFRAME_USIG_OFFSET]
	ldr	lr,	[sp, #L4_SIGFRAME_RETLR_OFFSET]

	ldr	pc,	[sp, #L4_SIGFRAME_SIGIP_OFFSET]

	.global __wombat_user_rt_sigentry_int;
__wombat_user_rt_sigentry_int:
	mov	r0,	#-EINTR

	.global __wombat_user_rt_sigentry_restart;
__wombat_user_rt_sigentry_restart:
	str	r0,	[sp, #L4_SIGRTFRAME_R0_OFFSET]

	.global __wombat_user_rt_sigentry;
__wombat_user_rt_sigentry:
	str	r1,	[sp, #L4_SIGRTFRAME_R1_OFFSET]
	str	r2,	[sp, #L4_SIGRTFRAME_R2_OFFSET]
	str	r3,	[sp, #L4_SIGRTFRAME_R3_OFFSET]
	str	r4,	[sp, #L4_SIGRTFRAME_R4_OFFSET]
	str	r5,	[sp, #L4_SIGRTFRAME_R5_OFFSET]
	str	r6,	[sp, #L4_SIGRTFRAME_R6_OFFSET]
	str	r7,	[sp, #L4_SIGRTFRAME_R7_OFFSET]
	str	r8,	[sp, #L4_SIGRTFRAME_R8_OFFSET]
	str	r9,	[sp, #L4_SIGRTFRAME_R9_OFFSET]
	str	r10,	[sp, #L4_SIGRTFRAME_R10_OFFSET]
	str	r11,	[sp, #L4_SIGRTFRAME_R11_OFFSET]
	str	r12,	[sp, #L4_SIGRTFRAME_R12_OFFSET]
	str	lr,	[sp, #L4_SIGRTFRAME_LR_OFFSET]

	ldr	r1,	[sp, #L4_SIGRTFRAME_PINFO_OFFSET]
	ldr	r2,	[sp, #L4_SIGRTFRAME_PUC_OFFSET]

	ldr	r0,	[sp, #L4_SIGRTFRAME_USIG_OFFSET]
	ldr	lr,	[sp, #L4_SIGRTFRAME_RETLR_OFFSET]

	ldr	pc,	[sp, #L4_SIGRTFRAME_SIGIP_OFFSET]

	.global __wombat_user_sigrestore;
__wombat_user_sigrestore:
	ldr	r0,	[sp, #L4_RESTORE_SP_OFFSET]
	sub	r0,	r0, #4
	str	r0,	[sp, #L4_RESTORE_SP_OFFSET]

	ldr	r0,	[sp, #L4_RESTORE_R0_OFFSET]
	ldr	r1,	[sp, #L4_RESTORE_R1_OFFSET]
	ldr	r2,	[sp, #L4_RESTORE_R2_OFFSET]
	ldr	r3,	[sp, #L4_RESTORE_R3_OFFSET]
	ldr	r4,	[sp, #L4_RESTORE_R4_OFFSET]
	ldr	r5,	[sp, #L4_RESTORE_R5_OFFSET]
	ldr	r6,	[sp, #L4_RESTORE_R6_OFFSET]
	ldr	r7,	[sp, #L4_RESTORE_R7_OFFSET]
	ldr	r8,	[sp, #L4_RESTORE_R8_OFFSET]
	ldr	r9,	[sp, #L4_RESTORE_R9_OFFSET]
	ldr	r10,	[sp, #L4_RESTORE_R10_OFFSET]
	ldr	r11,	[sp, #L4_RESTORE_R11_OFFSET]
	ldr	r12,	[sp, #L4_RESTORE_R12_OFFSET]
	ldr	lr,	[sp, #L4_RESTORE_LR_OFFSET]
	ldr	sp,	[sp, #L4_RESTORE_SP_OFFSET]

	ldr	pc,	[sp], #4

        .balign  0x400
__cmpxchg:
	mov	r3,	#0xff000000
	ldr	r3,	[r3, #0xff0]
	adr	ip,	9f
	str	ip,	[r3, #__L4_TCR_PREEMPT_CALLBACK_IP*4]
	mov	ip,	#32
	strb	ip,	[r3, #__L4_TCR_PREEMPT_FLAGS*4]

	ldr	ip,	[r2]
	teq 	ip,	r0
	streq	r1,	[r2],	#1

	mov	ip,	#0
	strb	ip,	[r3, #__L4_TCR_PREEMPT_FLAGS*4]
	subeq	r2,	r2,	#1
	moveq	r0,	#0
	movne	r0,	#1
	msreq	cpsr_f,	#(0x6 << 28)
	msrne	cpsr_f,	#0

	mov	pc,	lr
9:
	mov	ip,	#0
	strb	ip,	[r3, #__L4_TCR_PREEMPT_FLAGS*4]
	tst 	r2,	#1
	subne	r2,	r2,	#1
	moveq	r0,	#1
	movne	r0,	#0
	msreq	cpsr_f,	#(0x4 << 28)
	msrne	cpsr_f,	#(0x2 << 28)

	mov	pc,	lr
