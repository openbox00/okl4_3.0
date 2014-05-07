
/****************************************************************
**                                                             **
**    Copyright (c) 1996 - 2001 Caldera International, Inc.    **
**                    All Rights Reserved.                     **
**                                                             **
** This program is free software; you can redistribute it      **
** and/or modify it under the terms of the GNU General Public  **
** License as published by the Free Software Foundation;       **
** either version 2 of the License, or (at your option) any    **
** later version.                                              **
**                                                             **
** This program is distributed in the hope that it will be     **
** useful, but WITHOUT ANY WARRANTY; without even the implied  **
** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR     **
** PURPOSE. See the GNU General Public License for more        **
** details.                                                    **
**                                                             **
** You should have received a copy of the GNU General Public   **
** License along with this program; if not, write to the Free  **
** Software Foundation, Inc., 59 Temple Place, Suite 330,      **
** Boston, MA  02111-1307  USA                                 **
**                                                             **
****************************************************************/
#define _POSIX_SOURCE 1


#include <stdio.h>			/* enable fprintf(), etc. */
#include <stdlib.h>			/* enable exit(), etc. */
#include "suite.h"

source_file *
fillin_c()
{
	static source_file s = { " @(#) fillin.c:1.3 7/26/93 16:55:26",	/* SCCS info */
		__FILE__, __DATE__, __TIME__
	};

	return &s;
}


void
foo(int a,
    int b)

/* 
 * dummy routine which should never be called 
 */
{
	fprintf(stderr, "\nFoo(%d,%d) called.\n", a, b);	/* talk to human */
	exit(-1);			/* die quickly here */
}

void
foo_real(double a,
	 double b)

/* 
 * dummy routine which should never be called 
 */
{
	fprintf(stderr, "\nFoo_real(%g,%g) called.\n", a, b);	/* talk to human */
	exit(-1);			/* die quickly here */
}
