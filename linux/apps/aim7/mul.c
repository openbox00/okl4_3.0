
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
#include <stdio.h>			/* allow sscanf(), fprintf(), etc. */
#include <stdlib.h>			/* allow exit(), etc. */
#include "suite.h"

static int mul_double(),
  mul_float(),
  mul_long(),
  mul_short(),
  mul_int();

#define PI (3.14159)
#define ONE_OVER_PI (1.0/3.14159)
#define OPS_PER_LOOP 12

source_file *
mul_c()
{
	static char args_double[256];
	static char args_float[256];
	static source_file s = { " @(#) mul.c:1.7 3/4/94 17:17:10",	/* SCCS info */
		__FILE__, __DATE__, __TIME__
	};

	sprintf(args_double, "%g %g 1000000", PI, ONE_OVER_PI);	/* arbitrary use of pi */
	register_test("mul_double", args_double, mul_double,
		      1000 * OPS_PER_LOOP,
		      "Thousand Double Precision Multiplies");

	sprintf(args_float, "%g %g 1000000", 4.0, 1.0 / 4.0);	/* using .25 decreases round-off error */
	register_test("mul_float", args_float, mul_float, 1000 * OPS_PER_LOOP,
		      "Thousand Single Precision Multiplies");

	register_test("mul_long", "1 1 20000", mul_long, 20 * OPS_PER_LOOP,
		      "Thousand Long Integer Multiplies");
	register_test("mul_int", "1 1 20000", mul_int, 20 * OPS_PER_LOOP,
		      "Thousand Integer Multiplies");
	register_test("mul_short", "1 1 25000", mul_short, 25 * OPS_PER_LOOP,
		      "Thousand Short Integer Multiplies");

	return &s;
}


/*
 *      mul_double
 */
static int
mul_double(char *argv,
	   Result * res)
{
	int
	  n,				/* internal loop variable */
	  loop_cnt,			/* internal loop count */
	  tloop_cnt;			/* temporary internal loop count */

	double
	  d1,				/* copy of arg 1 */
	  d2,				/* copy of arg 2 */
	  td1, td2,			/* temp copy of args */
	  d;				/* result goes here */

	if (sscanf(argv, "%lg %lg %d", &td1, &td2, &tloop_cnt) < 3) {	/* get arguments */
		fprintf(stderr, "mul_double(): needs 3 arguments!\n");	/* print message */
		return (-1);		/* die */
	}
	d1 = td1;			/* use register variables */
	d2 = td2;
	loop_cnt = tloop_cnt;

	d = 1.0;
	/*
	 * Variable Values 
	 */
	/*
	 * d    d1    d2   
	 */
	for (n = loop_cnt; n > 0; n--) {	/*    1    x     1/x  - initial value */
		d *= d1;		/*    x    x     1/x */
		d1 *= d2;		/*    x    1     1/x */
		d1 *= d2;		/*    x    1/x   1/x */
		d2 *= d;		/*    x    1/x   1   */
		d2 *= d;		/*    x    1/x   x   */
		d *= d1;		/*    1    1/x   x   */
		d *= d1;		/*    1/x  1/x   x   */
		d1 *= d2;		/*    1/x  1     x   */
		d1 *= d2;		/*    1/x  x     x   */
		d2 *= d;		/*    1/x  x     1   */
		d2 *= d;		/*    1/x  x     1/x */
		d *= d1;		/*    1    x     1/x */
		/*
		 * Note that at loop end, d1 = 1/d2 
		 */
		/*
		 * which is as we started.  Thus, 
		 */
		/*
		 * the values in the loop are stable 
		 */
	}
	res->d = d;
	return (0);
}

/*
 *      mul_float
 */
static int
mul_float(char *argv,
	  Result * res)
{
	int
	  n,				/* internal loop variable */
	  loop_cnt,			/* internal loop count */
	  tloop_cnt;			/* temporary internal loop count */

	float
	  f1,				/* copy of arg 1 */
	  f2,				/* copy of arg 2 */
	  tf1, tf2,			/* temp copy of args */
	  f;				/* result goes here */

	if (sscanf(argv, "%g %g %d", &tf1, &tf2, &tloop_cnt) < 3) {	/* get arguments */
		fprintf(stderr, "mul_float(): needs 3 arguments!\n");	/* print message */
		return (-1);		/* die */
	}
	f1 = tf1;			/* use register variables */
	f2 = tf2;
	loop_cnt = tloop_cnt;

	f = 1.0;
	/*
	 * Variable Values 
	 */
	/*
	 * f    f1    f2   
	 */
	for (n = loop_cnt; n > 0; n--) {	/*    1    x     1/x  - initial value */
		f *= f1;		/*    x    x     1/x */
		f1 *= f2;		/*    x    1     1/x */
		f1 *= f2;		/*    x    1/x   1/x */
		f2 *= f;		/*    x    1/x   1   */
		f2 *= f;		/*    x    1/x   x   */
		f *= f1;		/*    1    1/x   x   */
		f *= f1;		/*    1/x  1/x   x   */
		f1 *= f2;		/*    1/x  1     x   */
		f1 *= f2;		/*    1/x  x     x   */
		f2 *= f;		/*    1/x  x     1   */
		f2 *= f;		/*    1/x  x     1/x */
		f *= f1;		/*    1    x     1/x */
		/*
		 * Note that at loop end, f1 = 1/f2 
		 */
		/*
		 * which is as we started.  Thus, 
		 */
		/*
		 * the values in the loop are stable 
		 */
	}
	res->f = f;
	return (0);
}

/*
 *      mul_long 
 */
static int
mul_long(char *argv,
	 Result * res)
{
	int
	  n,				/* inside loop variable */
	  loop_cnt,			/* internal loop count */
	  tloop_cnt;			/* temporary internal loop count */

	long
	  l1, l2,			/* working copies */
	  tl1, tl2,			/* temp copy of args */
	  l;				/* result */

	if (sscanf(argv, "%ld %ld %d", &tl1, &tl2, &tloop_cnt) < 3) {	/* get args */
		fprintf(stderr, "mul_long(): needs 3 arguments!\n");	/* handle errors */
		return (-1);
	}
	l1 = tl1;			/* use register variables */
	l2 = tl2;
	loop_cnt = tloop_cnt;

	l = 12345678;
	/*
	 * Variable Values 
	 */
	/*
	 * l    l1    l2  
	 */
	for (n = loop_cnt; n > 0; n--) {	/*    x    1     1   - initial value */
		l *= l1;		/*    x    1     1   */
		l1 *= l2;		/*    x    1     1   */
		l1 *= l2;		/*    x    1     1   */
		l2 *= l1;		/*    x    1     1   */
		l2 *= l1;		/*    x    1     1   */
		l *= l2;		/*    x    1     1   */
		l *= l1;		/*    x    1     1   */
		l1 *= l2;		/*    x    1     1   */
		l1 *= l2;		/*    x    1     1   */
		l2 *= l1;		/*    x    1     1   */
		l2 *= l1;		/*    x    1     1   */
		l *= l2;		/*    x    1     1   */
		/*
		 * Note that at loop end, all values 
		 */
		/*
		 * as we started.  Thus, the values 
		 */
		/*
		 * in the loop are stable 
		 */
	}
	res->l = l;			/* return value */
	return (0);			/* return success */
}

/*
 *      mul_int 
 */
static int
mul_int(char *argv,
	Result * res)
{
	int
	  n,				/* inside loop variable */
	  loop_cnt,			/* internal loop count */
	  tloop_cnt;			/* temporary internal loop count */

	int
	  i1, i2,			/* working copies */
	  ti1, ti2,			/* temp copy of args */
	  i;				/* result */

	if (sscanf(argv, "%d %d %d", &ti1, &ti2, &tloop_cnt) < 3) {	/* get args */
		fprintf(stderr, "mul_int(): needs 3 arguments!\n");	/* handle errors */
		return (-1);
	}
	i1 = ti1;			/* use register variables */
	i2 = ti2;
	loop_cnt = tloop_cnt;

	i = 12345678;
	/*
	 * Variable Values 
	 */
	/*
	 * i    i1    i2  
	 */
	for (n = loop_cnt; n > 0; n--) {	/*    x    1     1   - initial value */
		i *= i1;		/*    x    1     1   */
		i1 *= i2;		/*    x    1     1   */
		i1 *= i2;		/*    x    1     1   */
		i2 *= i1;		/*    x    1     1   */
		i2 *= i1;		/*    x    1     1   */
		i *= i2;		/*    x    1     1   */
		i *= i1;		/*    x    1     1   */
		i1 *= i2;		/*    x    1     1   */
		i1 *= i2;		/*    x    1     1   */
		i2 *= i1;		/*    x    1     1   */
		i2 *= i1;		/*    x    1     1   */
		i *= i2;		/*    x    1     1   */
		/*
		 * Note that at loop end, all values 
		 */
		/*
		 * as we started.  Thus, the values 
		 */
		/*
		 * in the loop are stable 
		 */
	}
	res->i = i;			/* return value */
	return (0);			/* return success */
}

/*
 *      mul_short 
 */
static int
mul_short(char *argv,
	  Result * res)
{
	int
	  n,				/* inside loop variable */
	  loop_cnt,			/* internal loop count */
	  tloop_cnt;			/* temporary internal loop count */

	short
	  s1, s2,			/* working copies */
	  ts1, ts2,			/* temp copy of args */
	  s;				/* result */

	if (sscanf(argv, "%hd %hd %d", &ts1, &ts2, &tloop_cnt) < 3) {	/* get args */
		fprintf(stderr, "mul_short(): needs 3 arguments!\n");	/* handle errors */
		return (-1);
	}
	s1 = ts1;			/* use register variables */
	s2 = ts2;
	loop_cnt = tloop_cnt;

	s = 1234;
	/*
	 * Variable Values 
	 */
	/*
	 * s    s1    s2  
	 */
	for (n = loop_cnt; n > 0; n--) {	/*    x    1     1   - initial value */
		s *= s1;		/*    x    1     1   */
		s1 *= s2;		/*    x    1     1   */
		s1 *= s2;		/*    x    1     1   */
		s2 *= s1;		/*    x    1     1   */
		s2 *= s1;		/*    x    1     1   */
		s *= s2;		/*    x    1     1   */
		s *= s1;		/*    x    1     1   */
		s1 *= s2;		/*    x    1     1   */
		s1 *= s2;		/*    x    1     1   */
		s2 *= s1;		/*    x    1     1   */
		s2 *= s1;		/*    x    1     1   */
		s *= s2;		/*    x    1     1   */
		/*
		 * Note that at loop end, all values 
		 */
		/*
		 * as we started.  Thus, the values 
		 */
		/*
		 * in the loop are stable 
		 */
	}
	res->s = s;			/* return value */
	return (0);			/* return success */
}
