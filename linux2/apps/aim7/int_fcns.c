
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
#define _POSIX_SOURCE 1			/* turn on POSIX functions */

/* #define COUNTING */
#include <stdio.h>			/* declare sscanf(), etc. */
#include <stdlib.h>			/* for malloc(), etc. */
#include <string.h>			/* for memset(), etc. */
#include "suite.h"

#define PRIME (1)
#define NONPRIME (0)
#define TOTAL_PRIMES (78500)		/* total # primes from 0 to 1000000 */

static int sieve();

source_file *
int_fcns_c()
{
	static source_file s = { " @(#) int_fcns.c:1.6 8/13/93 11:07:18",	/* SCCS info */
		__FILE__, __DATE__, __TIME__
	};

	register_test("sieve", "1000000 5", sieve, 5, "Integer Sieves");

	return &s;
}

static int
sieve(char *argv,
      Result * res)
{
	int
	  iter,				/* number of times to repeat the test */
	  n,				/* outside loop count */
	  i,				/* internal loop variable */
	  prime_count,			/* count primes when done */
	  max, i32;			/* internal loop count */

	char
	 *left,				/* points to next factor */
	 *right,			/* points to next non-prime */
	 *table;			/* holds data to be manipulated */


	COUNT_START;
	/*
	 * Step 1: Get arguments
	 */
	if (sscanf(argv, "%d %d", &max, &i32) < 2) {	/* get arguments */
		fprintf(stderr, "sieve(): needs 2 arguments!\n");	/* print message */
		return (-1);		/* die */
	}
	/*
	 * Step 2: Allocate space
	 */
	table = (char *)malloc(max);	/* allocate lots of space */
	if (table == NULL) {
		fprintf(stderr,
			"sieve(): Unable to allocate %d bytes of memory.\n",
			max);
		return (-1);
	}
	/*
	 * Step 3: Initialize and run sieve in loop.
	 */
	for (iter = 0; iter < i32; iter++) {
		memset((void *)table, PRIME, (size_t) max);	/* init all to PRIME */
		for (n = 2; n < max; n++) {	/* ignore 0 & 1, known */
			left = &table[n];	/* point to next factor */
			if (*left != PRIME)
				continue;	/* if it isn't prime, skip it */
			right = left + n;	/* point to the table */
			for (i = n + n; i < max; i += n) {	/* look for all multiples of *left */
				*right = NONPRIME;	/* mark this one as not prime */
				right += n;	/* move to next pointer */
			}		/* and loop */
		}			/* next time through */
	}				/* end of test */
	prime_count = 0;
	for (n = 0; n < max; n++)	/* check answer */
		if (table[n] == PRIME)
			prime_count++;
	if (prime_count != TOTAL_PRIMES) {
		fprintf(stderr, "sieve(): Problem calculating primes.\n");
		return (-1);
	}
	free(table);			/* return the memory */
	return 0;
}
