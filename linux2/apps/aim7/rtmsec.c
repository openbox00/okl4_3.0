
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

#include <sys/types.h>
#include <sys/times.h>
#include <time.h>
#include <stdio.h>			/* enable printf(), etc. */
#include <unistd.h>			/* enable sysconf(), etc. */
#include <sys/param.h>			/* get HZ */
#include "suite.h"
#include "testerr.h"

static double rate;
source_file *
rtmsec_c()
{
	static source_file s = { " @(#) rtmsec.c:1.4 3/4/94 17:26:40",	/* SCCS info */
		__FILE__, __DATE__, __TIME__
	};
	long hertz;			/* HZ */

#ifndef HZ
	hertz = sysconf(_SC_CLK_TCK);
	if (hertz == -1) {
		fprintf(stderr, "Cannot get HZ\n");
		exit(1);
	}
#else
	hertz = HZ;
#endif

	rate = 1000.0 / (double)hertz;

	return &s;
}

/*
 * long rmtsec(int)
 *
 * Description
 *	Returns real time in milliseconds.
 *
 * Arguments
 *	TRUE (1) - resets its internal 'epoch' marker
 *	FALSE(0) - returns time since last reset
 *
 * Modification
 *	6/12/89 Added documentation, minor fixes; Tin Le
 */
long
rtmsec(int reset)
{
	/*
	 * rtmsec - the timing routines used by multitask 
	 */
	struct tms ts;
	static clock_t epoch = 0l;
	clock_t delta, retval;
	double temp;

	/*
	 *  Milliseconds returned are now since first call.
	 */
	delta = times(&ts);		/* get the time *NOW* */
	if ((epoch == 0l) || (reset == TRUE)) {
		epoch = delta;
		retval = 0;
	} else {
		temp = rate * ((double)delta - (double)epoch);
		retval = (long)temp;
	}
	return retval;
}
