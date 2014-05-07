
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
#define _POSIX_SOURCE 1			/* enable POSIX functions */

#include "suite.h"
source_file *
rand_c()
{
	static source_file s = { " @(#) rand.c:1.2 1/22/96 00:00:00",	/* SCCS info */
		__FILE__, __DATE__, __TIME__
	};

	return &s;
}


#define MULT 1103515245l
#define INCR  12345l

static unsigned long seed = 1l;
static unsigned long seed2 = 1l;

unsigned int
aim_rand()
{
	seed = seed * MULT + INCR;	/* modulus is span of a long */
	return ((unsigned int)((seed >> 16) & 0x7fff));
}

void
aim_srand(unsigned int input)
{
	seed = (unsigned long)input;
}

unsigned int
aim_rand2()
{
	seed2 = seed2 * MULT + INCR;	/* modulus is span of a long */
	return ((unsigned int)((seed2 >> 16) & 0x7fff));
}

void
aim_srand2(unsigned int input)
{
	seed2 = (unsigned long)input;
}
