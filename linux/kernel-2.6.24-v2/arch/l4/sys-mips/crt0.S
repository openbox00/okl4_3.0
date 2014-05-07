/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * $Id: crt0.S,v 1.4 2004/03/05 00:37:51 cvansch Exp $
 *                
 ********************************************************************/


#include <asm/asm.h>
#include <asm/regdef.h>

	.global _stext; 
_stext:
	.global _start; 
	.align 3;
	.ent _start;
_start:

	jal	__L4_Init

	ld	a0,  0(sp)
	jal	__lib_init
	add	sp, sp,	(1*8)

	ld	a0,  0(sp)
	add	sp, sp,	(1*8)
	move	a1, sp
	jal	main

	jal	_Exit	    /* Should never get here */

	.end _start
