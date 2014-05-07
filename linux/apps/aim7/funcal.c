
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

#include <stdio.h>			/* allow fprintf() */
#define FUNCAL				/* enable special portions */
#include "suite.h"			/* general includesn */
#include "funcal.h"			/* handle declarations */
source_file *
funcal_c()
{
	static source_file s = { " @(#) funcal.c:1.2 7/26/93 16:48:55",	/* SCCS info */
		__FILE__, __DATE__, __TIME__
	};

	return &s;
}


int
fcal0()
{
	return ((*p_fcount += *p_i1));
}

int
fcal1(register int n)
{
	return ((*p_fcount += n));
}

int
fcal2(register int n,
      register int i)
{
	return ((*p_fcount += n + i));
}

int
fcal15(register int i1,
       register int i2,
       register int i3,
       register int i4,
       register int i5,
       register int i6,
       register int i7,
       register int i8,
       register int i9,
       register int i10,
       register int i11,
       register int i12,
       register int i13,
       register int i14,
       register int i15)
{
	return ((*p_fcount += i8 + i15));
}

/* special case to foil inlining optimizers */
int
fcalfake()
{
	fprintf(stderr, "\nfun_cal: You should not see this message.\n");
	return (-1);
/*NOTREACHED*/}
