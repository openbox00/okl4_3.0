
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
#include <stdio.h>			/* enable printf(), etc. */
#include <stdlib.h>			/* enable exit(), etc. */

#include "suite.h"			/* standard includes */

static int add_double(),
  add_float(),
  add_long(),
  add_short(),
  add_int();

#define PI (3.14159)
#define OPS_PER_LOOP 12

source_file *
add_c()
{
	static char args_double[256];
	static char args_float[256];
	static source_file s = { " @(#) add.c:1.9 3/4/94 17:16:58",	/* SCCS info */
		__FILE__, __DATE__, __TIME__
	};

	sprintf(args_double, "%g %g 1500000", PI, -PI);	/* arbitrary use of pi */
	register_test("add_double", args_double, add_double,
		      1500 * OPS_PER_LOOP,
		      "Thousand Double Precision Additions");

	sprintf(args_float, "%g %g 1000000", 3.5, -3.5);	/* 3.5 to minimize round-off error */
	register_test("add_float", args_float, add_float, 1000 * OPS_PER_LOOP,
		      "Thousand Single Precision Additions");

	register_test("add_long", "3 -3 5000000", add_long,
		      5000 * OPS_PER_LOOP, "Thousand Long Integer Additions");
	register_test("add_int", "3 -3 5000000", add_int, 5000 * OPS_PER_LOOP,
		      "Thousand Integer Additions");
	register_test("add_short", "3 -3 2000000", add_short,
		      2000 * OPS_PER_LOOP, "Thousand Short Integer Additions");
	return &s;
}

/*
 *	add_double 
 */
static int
add_double(char *argv,
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
		fprintf(stderr, "add_double(): needs 3 arguments!\n");
		fprintf(stderr, "add_double(): was passed <%s>\n", argv);
		return (-1);
	}
	d1 = td1;			/* use register variables */
	d2 = td2;
	loop_cnt = tloop_cnt;

	d = 0.0;
	/*
	 * Variable Values 
	 */
	/*
	 * d    d1    d2   
	 */
	for (n = loop_cnt; n > 0; n--) {	/*    0    x     -x  - initial value */
		d += d1;		/*    x    x     -x   */
		d1 += d2;		/*    x    0     -x   */
		d1 += d2;		/*    x    -x    -x   */
		d2 += d;		/*    x    -x    0    */
		d2 += d;		/*    x    -x    x    */
		d += d1;		/*    0    -x    x    */
		d += d1;		/*    -x   -x    x    */
		d1 += d2;		/*    -x   0     x    */
		d1 += d2;		/*    -x   x     x    */
		d2 += d;		/*    -x   x     0    */
		d2 += d;		/*    -x   x     -x   */
		d += d1;		/*    0    x     -x   */
		/*
		 * Note that at loop end, d1 = -d2 
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
 *      add_float
 */
static int
add_float(char *argv,
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
		fprintf(stderr, "add_float(): needs 3 arguments!\n");
		fprintf(stderr, "add_float(): was passed <%s>\n", argv);
		return (-1);
	}
	f1 = tf1;			/* use register variables */
	f2 = tf2;
	loop_cnt = tloop_cnt;

	f = 0.0;
	/*
	 * Variable Values 
	 */
	/*
	 * f    f1    f2   
	 */
	for (n = loop_cnt; n > 0; n--) {	/*    0    x     -x  - initial value */
		f += f1;		/*    x    x     -x   */
		f1 += f2;		/*    x    0     -x   */
		f1 += f2;		/*    x    -x    -x   */
		f2 += f;		/*    x    -x    0    */
		f2 += f;		/*    x    -x    x    */
		f += f1;		/*    0    -x    x    */
		f += f1;		/*    -x   -x    x    */
		f1 += f2;		/*    -x   0     x    */
		f1 += f2;		/*    -x   x     x    */
		f2 += f;		/*    -x   x     0    */
		f2 += f;		/*    -x   x     -x   */
		f += f1;		/*    0    x     -x   */
		/*
		 * Note that at loop end, f1 = -f2 
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
 *      add_long
 */
static int
add_long(char *argv,
	 Result * res)
{
	int
	  n,				/* internal loop variable */
	  loop_cnt,			/* internal loop count */
	  tloop_cnt;			/* temporary internal loop count */

	long
	  l1,				/* copy of arg 1 */
	  l2,				/* copy of arg 2 */
	  tl1, tl2,			/* temp copy of args */
	  l;				/* result goes here */

	if (sscanf(argv, "%ld %ld %d", &tl1, &tl2, &tloop_cnt) < 3) {	/* get arguments */
		fprintf(stderr, "add_long(): needs 3 arguments!\n");
		fprintf(stderr, "add_long(): was passed <%s>\n", argv);
		return (-1);
	}
	l1 = tl1;			/* use register variables */
	l2 = tl2;
	loop_cnt = tloop_cnt;

	l = 0;
	/*
	 * Variable Values 
	 */
	/*
	 * l    l1    l2   
	 */
	for (n = loop_cnt; n > 0; n--) {	/*    0    x     -x  - initial value */
		l += l1;		/*    x    x     -x   */
		l1 += l2;		/*    x    0     -x   */
		l1 += l2;		/*    x    -x    -x   */
		l2 += l;		/*    x    -x    0    */
		l2 += l;		/*    x    -x    x    */
		l += l1;		/*    0    -x    x    */
		l += l1;		/*    -x   -x    x    */
		l1 += l2;		/*    -x   0     x    */
		l1 += l2;		/*    -x   x     x    */
		l2 += l;		/*    -x   x     0    */
		l2 += l;		/*    -x   x     -x   */
		l += l1;		/*    0    x     -x   */
		/*
		 * Note that at loop end, l1 = -l2 
		 */
		/*
		 * which is as we started.  Thus, 
		 */
		/*
		 * the values in the loop are stable 
		 */
	}
	res->l = l;
	return (0);
}

/*
 *      add_int
 */
static int
add_int(char *argv,
	Result * res)
{
	int
	  n,				/* internal loop variable */
	  loop_cnt,			/* internal loop count */
	  tloop_cnt;			/* temporary internal loop count */

	int
	  i1,				/* copy of arg 1 */
	  i2,				/* copy of arg 2 */
	  ti1, ti2,			/* temp copy of args */
	  i;				/* result goes here */

	if (sscanf(argv, "%d %d %d", &ti1, &ti2, &tloop_cnt) < 3) {	/* get arguments */
		fprintf(stderr, "add_int(): needs 3 arguments!\n");
		fprintf(stderr, "add_int(): was passed <%s>\n", argv);
		return (-1);
	}
	i1 = ti1;			/* use register variables */
	i2 = ti2;
	loop_cnt = tloop_cnt;

	i = 0;
	/*
	 * Variable Values 
	 */
	/*
	 * i    i1    i2   
	 */
	for (n = loop_cnt; n > 0; n--) {	/*    0    x     -x  - initial value */
		i += i1;		/*    x    x     -x   */
		i1 += i2;		/*    x    0     -x   */
		i1 += i2;		/*    x    -x    -x   */
		i2 += i;		/*    x    -x    0    */
		i2 += i;		/*    x    -x    x    */
		i += i1;		/*    0    -x    x    */
		i += i1;		/*    -x   -x    x    */
		i1 += i2;		/*    -x   0     x    */
		i1 += i2;		/*    -x   x     x    */
		i2 += i;		/*    -x   x     0    */
		i2 += i;		/*    -x   x     -x   */
		i += i1;		/*    0    x     -x   */
		/*
		 * Note that at loop end, i1 = -i2 
		 */
		/*
		 * which is as we started.  Thus, 
		 */
		/*
		 * the values in the loop are stable 
		 */
	}
	res->i = i;
	return (0);
}

/*
 *      add_short
 */
static int
add_short(char *argv,
	  Result * res)
{
	int
	  n,				/* internal loop variable */
	  loop_cnt,			/* internal loop count */
	  tloop_cnt;			/* temporary internal loop count */

	short
	  s1,				/* copy of arg 1 */
	  s2,				/* copy of arg 2 */
	  ts1, ts2,			/* temp copy of args */
	  s;				/* result goes here */

	if (sscanf(argv, "%hd %hd %d", &ts1, &ts2, &tloop_cnt) < 3) {	/* get arguments */
		fprintf(stderr, "add_short(): needs 3 arguments!\n");
		fprintf(stderr, "add_short(): was passed <%s>\n", argv);
		return (-1);
	}
	s1 = ts1;			/* use register variables */
	s2 = ts2;
	loop_cnt = tloop_cnt;

	s = 0;
	/*
	 * Variable Values 
	 */
	/*
	 * s    s1    s2   
	 */
	for (n = loop_cnt; n > 0; n--) {	/*    0    x     -x  - initial value */
		s += s1;		/*    x    x     -x   */
		s1 += s2;		/*    x    0     -x   */
		s1 += s2;		/*    x    -x    -x   */
		s2 += s;		/*    x    -x    0    */
		s2 += s;		/*    x    -x    x    */
		s += s1;		/*    0    -x    x    */
		s += s1;		/*    -x   -x    x    */
		s1 += s2;		/*    -x   0     x    */
		s1 += s2;		/*    -x   x     x    */
		s2 += s;		/*    -x   x     0    */
		s2 += s;		/*    -x   x     -x   */
		s += s1;		/*    0    x     -x   */
		/*
		 * Note that at loop end, s1 = -s2 
		 */
		/*
		 * which is as we started.  Thus, 
		 */
		/*
		 * the values in the loop are stable 
		 */
	}
	res->s = s;
	return (0);
}
