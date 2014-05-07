/*
 * Fault handler for memcpy_user_armshared.c
 */

#if defined(ARM_SHARED_DOMAINS) || defined(CONFIG_DIRECT_VM)

#include <linux/linkage.h>
#include <asm/macros.h>
#include INC_SYSTEM(assembler.h)


#define	__L4_TCR_PREEMPT_CALLBACK_IP	(9)
#define	__L4_TCR_PREEMPTED_IP			(10)
#define EFAULT	(14)

	.global fault_sp

	.data
fault_sp:
	.space 4

	.text


/*
 * Prototype: int run_with_fault_handler(char *arg1, char *arg2, unsigned long arg3, 
 *											void (*func), int type) 
 */
ENTRY(run_with_fault_handler)
	stmdb	sp,	{r0-r14}
	sub		sp,		sp,	#60
	mov		r9,		#0xff000000
	ldr		r9,		[r9, #0xff0]
	adr		r10,	__handler_fault	
	ldr		r4,		=fault_sp
	str		sp,		[r4]
	str		r10,	[r9, #(__L4_TCR_PREEMPT_CALLBACK_IP*4)]		

	mov		lr,		pc				
	mov		pc,		r3
	str		r0,		[sp, #0]		

	ldmia	sp,		{r0-r14}		
	b	__handler_ok					
__handler_fault:						
	stmdb	sp,		{r0-r14, pc}	
	sub		sp,		sp,	#68			
	mov		r9,		#0xff000000		
	ldr		r9,		[r9, #0xff0]	
	mrs		r8,		cpsr			
	ldr		r10,	[r9, #(__L4_TCR_PREEMPTED_IP*4)]		
	str		r8,		[sp, #0]		
	str		r10,	[sp, #64]		

	ldr		r0,		=fault_sp
	ldr		r0,		[r0]
	ldr		r0,		[r0, #60]
	mov		r4,		sp
	bic		sp,		#7
	bl		fault_handler	
	mov		sp,		r4
	cmp		r0,		#(-EFAULT)
	ldreq	r1,		=fault_sp		
	ldreq	sp,		[r1]			
	ldmeqia	sp,		{r0-r14}		
	beq		__handler_fail			

	ldr		r8,		[sp, #0]		
	add		sp,		sp,	#4			
	msr		cpsr,	r8				
	ldmia	sp,		{r0-r14, pc}	
__handler_fail:						
	mov		r0,		#(-EFAULT)
__handler_ok:						
	mov		pc,		lr

#endif
