/*********************************************************************
 *                
 * Copyright (C) 2004,  National ICT Australia
 *                
 ********************************************************************/

#include <asm/macros.h>
#include <asm/errno.h>
#include <asm/constants.h>
#include <asm/regdef.h>
#include <l4/mips64/vregs.h>

	.section .exregspage
	.global __wombat_user_sig_fault;
__wombat_user_sig_fault:
	jr	ra

	.global __wombat_user_fork_handler;
__wombat_user_fork_handler:
	/* We can use the t2-t7 registers safely here */
	ld	t2,	(__L4_TCR_USER_DEFINED_HANDLE*4)(k0)		/* Get user defined handle */
	move	v0,	zero		/* Clear result */
	add	t2,	t2,	4	/* Increment IP */
	move	a3,	zero		/* Clear errors */
	jr	t2			/* jump to fork-ip */

	.global __wombat_user_sigentry_int;
__wombat_user_sigentry_int:
//	mov	r0,	#-EINTR

	.global __wombat_user_sigentry_restart;
__wombat_user_sigentry_restart:
//	str	r0,	[sp, #L4_SIGFRAME_R0_OFFSET]

	.global __wombat_user_sigentry;
__wombat_user_sigentry:
//	str	r1,	[sp, #L4_SIGFRAME_R1_OFFSET]
//	str	lr,	[sp, #L4_SIGFRAME_LR_OFFSET]

//	ldr	r0,	[sp, #L4_SIGFRAME_USIG_OFFSET]
//	ldr	lr,	[sp, #L4_SIGFRAME_RETLR_OFFSET]

//	ldr	pc,	[sp, #L4_SIGFRAME_SIGIP_OFFSET]

	.global __wombat_user_rt_sigentry_int;
__wombat_user_rt_sigentry_int:
//	mov	r0,	#-EINTR

	.global __wombat_user_rt_sigentry_restart;
__wombat_user_rt_sigentry_restart:
//	str	r0,	[sp, #L4_SIGRTFRAME_R0_OFFSET]

	.global __wombat_user_rt_sigentry;
__wombat_user_rt_sigentry:
//	str	r1,	[sp, #L4_SIGRTFRAME_R1_OFFSET]
//	str	lr,	[sp, #L4_SIGRTFRAME_LR_OFFSET]

//	ldr	r1,	[sp, #L4_SIGRTFRAME_PINFO_OFFSET]
//	ldr	r2,	[sp, #L4_SIGRTFRAME_PUC_OFFSET]

//	ldr	r0,	[sp, #L4_SIGRTFRAME_USIG_OFFSET]
//	ldr	lr,	[sp, #L4_SIGRTFRAME_RETLR_OFFSET]

//	ldr	pc,	[sp, #L4_SIGRTFRAME_SIGIP_OFFSET]

	.global __wombat_user_sigrestore;
__wombat_user_sigrestore:
//	ldr	r0,	[sp, #L4_RESTORE_R0_OFFSET]
//	ldr	r1,	[sp, #L4_RESTORE_R1_OFFSET]

//	ldr	pc,	[sp, #L4_RESTORE_FRAME_PC_OFFSET]

