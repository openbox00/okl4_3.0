/*********************************************************************
 *                
 * Copyright (C) 1999, 2000, 2001,  Karlsruhe University
 *                
 * File path:     sigma0/crt0-arm.S
 * Description:   Startup code for ARM family processors.
 *                
 * @LICENSE@
 *                
 * $Id: crt0.S,v 1.4 2004/03/04 06:27:24 cvansch Exp $
 *                
 ********************************************************************/

	.text
	.global _stext;
_stext:
	.global _start;
	.align;
_start:
#if defined(CONFIG_IGUANA)
	/* Pop off the iguana program stack frame. */
	ldr	r0, [sp], #4 /* pointer to object env */
	ldr	r1, [sp], #4 /* argc */
	ldr	r2, [sp], #4 /* argv */
#endif
	b	__sys_entry

	.end _start
