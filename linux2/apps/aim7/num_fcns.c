
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
#include <math.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>			/* for getlogin(), etc.; RAND_MAX */
#include <string.h>			/* for strcpy(), etc. */
#include <time.h>			/* for ctime() */
#include <sys/types.h>
#include <dirent.h>			/* for read/seek/rewind/closedir */
#include <termios.h>			/* for cfgetiospeed */
#include <unistd.h>			/* for getpgid(), etc. */
#include <sys/utsname.h>		/* for uname */
#include <sys/times.h>			/* for times() */
#include <sys/wait.h>			/* wait() */
#include <limits.h>
#include <pwd.h>			/* for getpwuid, getpwnam, etc. */
#include <grp.h>			/* for getgrid, getgrnam, etc. */
#include <ctype.h>			/* for is*() */
#include "suite.h"

#define MAXSTRINGS (100)		/* number of strings */
#define MAXSORTSIZE (10000)		/* make it large enough to matter */
#define MAXTRIES (100)			/* for newton-raphson */
#define M_SIZE (4)			/* size for matrix in matrix_rtns */
#define A_HEIGHT 100			/* height for matrix in array_rtns */
#define A_WIDTH  101			/* width for matrix in array_rtns */
#define A_SIZE   (A_HEIGHT*A_WIDTH)	/* array size for array_rtns */
#define SIZBUF (8192)			/* buffer size for mem ops */
#define NUMVALS (500)			/* size of vals[] arrays */
#define NUM10VALS (10000)		/* size of vals10[] arrays */
#define ARG (vals10[count10++ % Members(vals10)])	/* get a positive, negative or 0 value from vals10[] for use as arg */
#define POS (ARG + 10.0001)		/* get a positive value for use as arg */
#define NEG (ARG - 10.0001)		/* get a negative value for use as arg */

static int aim_system(char *);		/* our uninterruptable version of system() */
static int num_rtns_1(char *,
		      Result *);	/* numeric routines (log, exp, etc.) */
static int series_1(char *,
		    Result *);		/* series expansion () */
static int newton_raphson(char *,
			  Result *);	/* newton/raphson equation solver */
static int trig_rtns(char *,
		     Result *);		/* trig routines (sin, cos, etc.) */
static int matrix_rtns(char *,
		       Result *);	/* rendering test (1x4) * (4x4) = (4x1) */
static int array_rtns(char *,
		      Result *);	/* linear solutions using gausian elimination */
static int string_rtns_1(char *argv,
			 Result * res);	/* string operations (str{cat, cpy, str,...} etc.) */
static int mem_rtns_1(char *argv,
		      Result * res);	/* *alloc tests */
static int mem_rtns_2(char *argv,
		      Result * res);	/* mem{set, cpy, move}, etc. */
static int sort_rtns_1(char *argv,
		       Result * res);	/* qsort, bsearch, etc. */
static int misc_rtns_1(char *,
		       Result *);	/* miscellaneous routines getpid, etc. */
static int dir_rtns_1(char *,
		      Result *);	/* dir routines read/rewind dir, etc. */
static int shell_rtns_1(char *,
			Result *);	/* customizable, change aim_1.sh */
static int shell_rtns_2(char *,
			Result *);	/* customizable, change aim_2.sh */
static int shell_rtns_3(char *,
			Result *);	/* customizable, change aim_3.sh */

static double vals[NUMVALS];		/* holds floating point values (1..0) */
static double vals10[NUM10VALS];	/* holds floating point values (-10..10) */
int table[MAXSORTSIZE];			/* table to sort of integers */
static int count = 0;			/* counter for vals[] array */
static int count10 = 0;			/* counter for vals10[] array */
double compiler_fake_out1;		/* dummy variable for num_rtns_1()  */
double compiler_fake_out2;		/* and trig_fcns()...since globally */
double compiler_fake_out3;		/* available, compiler can't assume */
double compiler_fake_out4;		/* anything and thus can't opt.     */
double compiler_fake_out5;		/* their assignments out            */

double
next_value()
{
	double temp;

	temp = vals[count++ % Members(vals)];
	return temp;
}

source_file *
num_fcns_c()
{
	static source_file s = { " @(#) num_fcns.c:1.18 3/21/94 11:45:01",	/* SCCS info */
		__FILE__, __DATE__, __TIME__
	};
	int i;
	double t1, t2;

	/*
	 * Find the "smallest non-zero positive real". We use this value every now and
	 * then because it is troublesome to some machines 
	 */
	t1 = t2 = 1.0;			/* start with 1  */
	while (t1 > 0.0) {		/* while not to zero */
		t2 = t1;		/* save old value */
		t1 = t1 / 2.0;		/* cut this value in half */
	}				/* and loop */
	/*
	 * Initialize the array with values for future reference
	 */
	vals[0] = 0.0;			/* start with good values, 0, 1 */
	vals[1] = 1.0;
	vals[2] = t2;			/* then this small value */
	/*
	 * NUMVALS changed to 500 to minimize # of dnorms created 
	 */
	/*
	 * dnorm values started at array element 1026 = 1.1e-308  
	 */
	/*
	 * now we just use t2 once every 500 values.  tests like  
	 */
	/*
	 * series_1 will themselves generate A LOT of dnorm traffic 
	 */
	for (i = 3; i < NUMVALS; i++) {	/* fill in the rest of the array */
		vals[i] = fabs(vals[i - 2] - vals[i - 1]) / 2.0;	/* with running averages to make strange bedfellows */
		/*
		 * since we changed NUMVALS to 500, we should never enter this if case... 
		 */
		if (vals[i] == 0) {	/* start the sequence all over again */
			vals[++i] = 1.0;
			vals[++i] = t2;
		}
	}
	for (i = 0; i < NUM10VALS; i++) {	/* fill in the vals10[] array, for use in num_rtns_1() */
		if (i % 100 == 0)
			vals10[i] = t2;	/* throw in "smallest non-zero positive real" */
		else
			vals10[i] = ((double)rand()) / RAND_MAX * 20. - 10.;	/* a floating point # between -10 and 10, inclusive */
	}
	/*
	 * Initialize sort table
	 */
	for (i = 0; i < MAXSORTSIZE; i++)	/* make it in order (worst case for qsort) */
		table[i] = i;
	/*
	 * register the tests
	 */
	register_test("num_rtns_1", "100", num_rtns_1, 100,
		      "Numeric Functions");
	register_test("new_raph", "200", newton_raphson, 200, "Zeros Found");
	register_test("trig_rtns", "100", trig_rtns, 10000,
		      "Trigonometric Functions");
	register_test("matrix_rtns", "100", matrix_rtns, 100,
		      "Point Transformations");
	register_test("array_rtns", "20", array_rtns, 20,
		      "Linear Systems Solved");
	register_test("string_rtns", "100", string_rtns_1, 100,
		      "String Manipulations");
	register_test("mem_rtns_1", "100", mem_rtns_1, 30000,
		      "Dynamic Memory Operations");
	register_test("mem_rtns_2", "100", mem_rtns_2, 100,
		      "Block Memory Operations");
	register_test("sort_rtns_1", "10", sort_rtns_1, 10, "Sort Operations");
	register_test("misc_rtns_1", "10", misc_rtns_1, 10, "Auxiliary Loops");
	register_test("dir_rtns_1", "100", dir_rtns_1, 10000,
		      "Directory Operations");
	register_test("shell_rtns_1", "9", shell_rtns_1, 1, "Shell Scripts");
	register_test("shell_rtns_2", "9", shell_rtns_2, 1, "Shell Scripts");
	register_test("shell_rtns_3", "9", shell_rtns_3, 1, "Shell Scripts");
	register_test("series_1", "100", series_1, 100, "Series Evaluations");
	/*
	 * return pointer to sccs info
	 */
	return &s;
}

/*
 * Arbitrary function for newton-raphson solution:
 *
 * f(x) = 1000.0 * sin(x) * cos(x)
 * 
 * Therefore the derivative is:
 *
 * f'(x) = 1000.0 * sin(x) * d[cos(x)]/dx + 1000.0 * d[sin(x)]/dx * cos(x) + sin(x) * cos(x) * d[1000.0]/dx
 *
 *       = 1000.0 sin(x) * (-sin(x))    + 1000.0 * (cos(x))     * cos(x)
 *
 *       = 1000.0 * -(sin(x)**2) + 1000.0 * cos(x)**2
 */
static double
f_prime(double arg)
{
	double
	  c = cos(arg),
	  s = sin(arg);

	return -(s * s * 1000.0) + (c * c * 1000.0);
}

static double
f(double arg)
{
	return 1000.0 * sin(arg) * cos(arg);
}

static int
newton_raphson(char *argv,
	       Result * res)
{
	int n, i64, i;
	double p, p0, start, delta;

	COUNT_START;
	/*
	 * Get arguments
	 */
	if (sscanf(argv, "%d", &i64) < 1) {
		fprintf(stderr, "newton_raphson(): needs 1 argument!\n");
		return -1;
	}
	for (n = 0; n < i64; n++) {
		/*
		 * Step 1: i = 1
		 */
		i = 1;
		start = p0 = 0.05 * (double)n;
		/*
		 * Step 2: while i < MAXTRIES do steps 3-6
		 */
		while (i < MAXTRIES) {
			/*
			 * Step 3: p = p0 - f(po) / f'(p0)
			 */
			p = p0 - f(p0) / f_prime(p0);
			/*
			 * Step 4: if |p - p0| < TOL then terminate successfully
			 */
			delta = fabs(p - p0);
			if (delta == 0.0)
				break;
			/*
			 * Step 5: i += 1
			 */
			i += 1;
			/*
			 * Step 6: p0 = p
			 */
			p0 = p;
		}
		/*
		 * Step 7: Method fails -- too many iterations
		 */
		if (i >= MAXTRIES) {
			fprintf(stderr,
				"Unable to solve equation in %d tries. P = %g, P0 = %g, delta = %g\n",
				MAXTRIES, p, p0, delta);
			return -1;
		}
		COUNT_BUMP;
	}
	COUNT_END("new_raph");
	return 0;
}

static int
series_1(char *argv,
	 Result * res)

/*
 * Evaluate the infinite series for sin(x) around 0
 *
 * sin(x) = x/1! - x^3/3! + x^5/5!.....
 * 
 * and compare it to the system's sin routine results.
 *
 * Iterate until absolute convergency (each additional term adds no
 * additional precision to the result).
 */
{
	int
	  n, i64;
	double
	  i, d0,			/* original x for sin(x) */
	  d1,				/* x for sin(x), changed during calc */
	  old, sine, d1_squared, fact;

	COUNT_START;

	/*
	 * Get arguments
	 */
	if (sscanf(argv, "%d", &i64) < 1) {
		fprintf(stderr, "series_1(): needs 1 argument!\n");
		return -1;
	}
	/*
	 * Do the actual loop where the time is spent
	 */
	for (n = 0; n < i64; n++) {
		d0 = d1 = next_value();	/* fetch next value */
		old = 100.0;		/* illegal possible value */
		sine = 0.0;		/* final sine value goes here */
		d1_squared = d1 * d1;	/* calc once outside of loop */
		fact = 1.0;		/* starting factorial value (1!) */
		i = 1.0;		/* series counter for sine: 1, 3, 5, ... */
		while (sine != old) {	/* loop until we've converged totally */
			old = sine;	/* save last value */
			sine += d1 / fact;	/* add next in series */
			fact *= (i + 1) * (i + 2);	/* i factorial for next time around */
			/*
			 * series advances by 2 
			 */
			i += 2;
			d1 *= -(d1_squared);	/* power of d1 for next in series */
			/*
			 * neg to flip sign 
			 */
		}
		if (fabs(sine - sin(d0)) > .0001) {
			fprintf(stderr,
				"series_1(): calculated sine for %g is %g, sin() returns %g.\n",
				d0, sine, sin(d0));
			return -1;
		}
		COUNT_BUMP;
	}
	COUNT_END("series_1");
	return (0);
}

static int
num_rtns_1(char *argv,
	   Result * res)
     /*
      * This routine tests the traditional hard working numeric routines.
      * Ansi C says that function calls cannot be optimized away.....
      * We'll see. Anyway, these functions do the majority of the work
      * in numeric (non-trig) applications. Vendors good at these functions
      * will deliver higher performance for these classes of applications.
      *
      * We try to keep track of parameter values so that no illegal values
      * are fed to these routines.
      * We try to use all results so that a compiler will not omit a call 
      * whose results are never used.
      * We try to keep track of parameter values so that a variety of values
      * are fed to these routines.
      */
{
	int n, i64, itemp1;
	double d1, d2, dtemp1;

	COUNT_START;

	/*
	 * Get arguments
	 */
	if (sscanf(argv, "%d", &i64) < 1) {
		fprintf(stderr, "num_rtns_1(): needs 1 argument!\n");
		return -1;
	}
	/*
	 * Do the actual loop where the time is spent
	 */
	for (n = 0; n < i64; n++) {
		if (debug < 10) {
			compiler_fake_out1 = floor(exp(ARG))
				+ ceil(log10(POS));
			compiler_fake_out2 = modf(log(POS), &dtemp1)
				+ frexp(NEG, &itemp1);	/* don't want ARG, because of "smallest" */
			compiler_fake_out3 = ldexp(ARG, abs(-itemp1))
				+ fabs(fmod(ARG * 5, POS));	/* try to get 1st fmod() arg larger than second */
			compiler_fake_out4 = sqrt(pow(POS, ARG));	/* pow() will always return pos value */
			srand(itemp1);
			compiler_fake_out5 = labs((long)(-rand() + n));
			/*
			 * rand() often returns same #, +n adds variation, '-' gives labs() something to do 
			 */
		} else {
			/*
			 * compiler_fake_out1 = floor(exp(ARG))
			 */
			d1 = ARG;
			printf("exp<%g> is %g\n", d1, exp(d1));
			fflush(stdout);
			printf("floor<%g> is %g\n", exp(d1), floor(exp(d1)));
			fflush(stdout);

			/*
			 * + ceil(log10(POS)); 
			 */
			d1 = POS;
			printf("log10<%g> is %g\n", d1, log10(d1));
			fflush(stdout);
			printf("ceil<%g> is %g\n", log10(d1), ceil(log10(d1)));
			fflush(stdout);

			/*
			 * compiler_fake_out2 = modf(log(POS), &dtemp1)    
			 */
			d1 = POS;
			printf("log<%g> is %g\n", d1, log(d1));
			fflush(stdout);
			printf("modf<%g> is %g\n", log(d1),
			       modf(log(d1), &dtemp1));
			fflush(stdout);

			/*
			 * + frexp(NEG, &itemp1); 
			 */
			d1 = NEG;
			d2 = frexp(d1, &itemp1);
			printf("frexp<%g> is %g itemp is %d\n", d1, d2,
			       itemp1);
			fflush(stdout);

			/*
			 * compiler_fake_out3 = ldexp(ARG, abs(-itemp1)) 
			 */
			d1 = ARG;
			printf("abs<%d> is %d\n", -itemp1, abs(-itemp1));
			fflush(stdout);
			printf("ldexp<%g, %d> is %g\n", d1, abs(-itemp1),
			       ldexp(d1, abs(-itemp1)));
			fflush(stdout);

			/*
			 * + fabs(fmod(ARG, POS)); 
			 */
			d1 = ARG;
			d2 = POS;
			printf("fmod<%g, %g> is %g\n", d1 * 5, d2,
			       fmod(d1 * 5, d2));
			fflush(stdout);
			printf("fabs(fmod<%g, %g>) is %g\n", d1 * 5, d2,
			       fabs(fmod(d1 * 5, d2)));
			fflush(stdout);

			/*
			 * compiler_fake_out4 = sqrt(pow(POS, ARG)); 
			 */
			d1 = POS;
			d2 = ARG;
			printf("pow<%g, %g> is %g\n", d1, d2, pow(d1, d2));
			fflush(stdout);
			printf("sqrt(pow<%g, %g>) is %g\n", d1, d2,
			       sqrt(pow(d1, d2)));
			fflush(stdout);

			srand(itemp1);
			printf("calling srand<%d>\n", itemp1);
			fflush(stdout);

			/*
			 * compiler_fake_out5 = labs((long)(-rand() + n));
			 */
			itemp1 = -rand() + n;
			printf("labs<%ld> is %ld\n", (long)itemp1,
			       labs((long)itemp1));
			fflush(stdout);
		}
		COUNT_BUMP;
	}
	COUNT_END("num_rtns_1");
	return 0;
}

static int
trig_rtns(char *argv,
	  Result * res)
     /*
      * This routine does the trig and similar functions
      *
      * We try to make sure that no illegal values are fed to these routines.
      * We try to use all results so that a compiler will not omit a call 
      * whose results are never used.
      * We try to keep track of parameter values so that a variety of values
      * are fed to these routines.
      *
      * We nest functions on purpose to allow compilers to reuse floating point 
      * registers but not to optimize out any of the calculations or waste time storing
      * to global variables.
      */
{
	int n, i64;

	COUNT_START;
	/*
	 * First, get args
	 */
	if (sscanf(argv, "%d", &i64) < 1) {
		fprintf(stderr, "trig_rtns(): needs 1 argument!\n");
		return -1;
	}
	/*
	 * prepare for the loop
	 */
	i64 *= 100;			/* scale it some */
	/*
	 * Do the actual loop where the time is spent
	 */
	for (n = 0; n < i64; n++) {
		if (debug < 10) {
			compiler_fake_out1 = acos(cos(asin(sin(ARG))));	/* asin{acos} can legally use any output of sin{cos} */
			/*
			 * no bound on sin and cos arg 
			 */
			compiler_fake_out2 = tanh(atan(tan(ARG)));	/* atan can legally use any output of tan; no bound on tanh arg */
			compiler_fake_out3 = atan2(sinh(ARG), cosh(ARG));	/* no bound on sinh & cosh arg; cosh always returns pos # */
		} else {
			double d1, d2;

			d1 = ARG;
			printf("sin(%g) is %g\n", d1, sin(d1));
			fflush(stdout);
			printf("asin(%g) is %g\n", sin(d1), asin(sin(d1)));
			fflush(stdout);
			printf("cos(%g) is %g\n", asin(sin(d1)),
			       cos(asin(sin(d1))));
			fflush(stdout);
			printf("acos(%g) is %g\n", cos(asin(sin(d1))),
			       acos(cos(asin(sin(d1)))));
			fflush(stdout);
			d1 = ARG;
			printf("tan(%g) is %g\n", d1, tan(d1));
			fflush(stdout);
			printf("atan(%g) is %g\n", tan(d1), atan(tan(d1)));
			fflush(stdout);
			printf("tanh(%g) is %g\n", atan(tan(d1)),
			       tanh(atan(tan(d1))));
			fflush(stdout);
			d1 = ARG;
			d2 = ARG;
			printf("sinh(%g) is %g\n", d1, sinh(d1));
			fflush(stdout);
			printf("cosh(%g) is %g\n", d2, cosh(d2));
			fflush(stdout);
			printf("atan2(%g, %g) is %g\n", sinh(d1), cosh(d2),
			       atan2(sinh(d1), cosh(d2)));
			fflush(stdout);
		}
		COUNT_BUMP;
	}
	COUNT_END("trig_rtns");
	return 0;
}

static int
matrix_rtns(char *argv,
	    Result * res)
     /*
      * Rendering routine
      */
{
	int n, i64, i, j, k;
	double prod;
	double v1[M_SIZE], v2[M_SIZE][M_SIZE], v3[M_SIZE];

	COUNT_START;
	/*
	 * Step 1: get args
	 */
	if (sscanf(argv, "%d", &i64) < 1) {
		fprintf(stderr, "matrix_rtns(): needs 1 argument!\n");
		return -1;
	}
	/*
	 * Step 2: do the routine
	 */
	prod = 0.0;			/* start with nothing */
	for (n = 0; n < i64; n++) {
		/*
		 * First, fetch fresh values for matrices
		 */
		for (i = 0; i < M_SIZE; i++)	/* get new data points */
			v1[i] = next_value();
		for (i = 0; i < M_SIZE; i++)	/* fill transformation matrix */
			for (j = 0; j < M_SIZE; j++)
				v2[i][j] = next_value();
		/*
		 * Now do the matrix arithmetic
		 */
		for (i = 0; i < M_SIZE; i++) {	/* realize each of these values */
			v3[i] = 0.0;	/* clear it out */
			for (j = 0; j < M_SIZE; j++)	/* do the dot product */
				v3[i] += v1[j] * v2[i][j];	/* for the results */
		}
		/*
		 * Now scale and convert to real world coordinates
		 */
		i = v3[0] * 1280.0;	/* convert to screen (int) coords */
		j = v3[1] * 1024.0;	/* y value */
		k = v3[2] * 8192.0;	/* zvalue */
		prod += v3[0] * v3[1] * v3[2];	/* sum magnitude (code motion stopper) */
		compiler_fake_out1 = i + j + k + prod;	/* use the calculated values */
		COUNT_BUMP;
	}
	COUNT_END("matrix_rtns");
	return 0;
}

/*
 * The following routines test a standard gausian elimination simultaneous equation
 * solver which is a classic matrix arithmetic problem. It works by converting the
 * array into LU form and then solves until the array is the identity matrix. The
 * rightmost column then contains the solution to the system of equations. A small
 * modification would convert this into a matrix inversion routine at some small increase
 * in runtime.
 */
#define m(x,y) a[x+wide*y]

static void
print_array(double *a,			/* the array to print */

	    int wide,			/* the width of the array */

	    int high)
{					/* the height of the array */
	int ii, jj;			/* loop variables */

	for (ii = 0; ii < high; ii++) {	/* for each row in the array */
		for (jj = 0; jj < wide; jj++)	/* for each column in the array */
			printf("%13g ", m(jj, ii));	/* print the value */
		printf("\n");		/* and then end the line */
	}				/* end of row */
}

static int
solve_array(double *a,			/* input array of wide x high */

	    double *b,			/* result array of size high */

	    int wide,			/* width of array */

	    int high)
{					/* height of array */
	int i, j, k;			/* loop variables */
	double t;			/* temporary (scaling) value */

	/*
	 * Step 1: Make array upper triangular
	 */
	for (i = 0; i < high; i++) {	/* for each row in the array */
		t = m(i, i);		/* pick the scale factor */
		if (t == 0.0) {		/* if it is 0.0, the array is singular */
			printf("Singular:\n");
			print_array(a, wide, high);
			return 0;	/* error here */
		}
		for (j = i; j < wide; j++)	/* scale the row until diagonal point is 1.0 */
			m(j, i) = m(j, i) / t;	/* narrow the loop to reduce load since left of i = 0.0 */

		for (j = i + 1; j < high; j++) {	/* zero out all lower entries in this column */
			t = m(i, j);	/* factor to solve by */
			for (k = 0; k < wide; k++)	/* subtract ith row from jth row t times */
				m(k, j) = m(k, j) - t * m(k, i);	/* to zero out next column */
		}
	}
	/*
	 * Step 2: Make array into identity matrix by eliminating non-diagonal entries in upper triangle.
	 */
	for (i = high - 1; i > 0; i--) {	/* solve for identity matrix */
		for (j = i - 1; j >= 0; j--) {	/* take away one column */
			t = m(i, j);	/* get the factor */
			for (k = 0; k < wide; k++)	/* subtract the t times the i'th equation from the j'th equation */
				m(k, j) = m(k, j) - t * m(k, i);
		}
	}
	/*
	 * Step 3: Get the last column which contains the solution (since the
	 *         array is now the identity matrix).
	 */
	for (i = 0; i < high; i++)
		b[i] = m(wide - 1, i);
	return 1;			/* return success */
}

static int
test(double *a,				/* input array */

     double *b,				/* answers to check */

     int wide,				/* dimensions of array */

     int high)
{
	int i, j;			/* loop variables */
	double t;			/* temporary sum */

	for (i = 0; i < high; i++) {	/* loop through rows */
		t = 0.0;		/* start sum off at zero */
		for (j = 0; j < high; j++)	/* for each term */
			t += m(j, i) * b[j];	/* calculate running sum */
		if (fabs(m(wide - 1, i) - t) > .5) {	/* if sum isn't close to the answer */
			printf("Row %d: Wanted %g, got %g (%g)\n", i,	/* error here */
			       m(wide - 1, i), t, fabs(m(wide - 1, i) - t));
			return 0;	/* return error */
		}
	}
	return 1;			/* else return success */
}

static int
array_rtns(char *argv,
	   Result * res)
     /*
      * Solution to linear system of equations
      */
{
	int n, i64, high, j, k, wide;
	static double a[A_SIZE], a_orig[A_SIZE], val, results[A_HEIGHT];	/* arguments, results */

	COUNT_START;
	memset(a, 0, A_SIZE * sizeof (double));	/* initialize array */
	memset(a_orig, 0, A_SIZE * sizeof (double));	/* initialize array */
	/*
	 * Step 1: get args
	 */
	if (sscanf(argv, "%d", &i64) < 1) {
		fprintf(stderr, "array_rtns(): needs 1 argument!\n");
		return -1;
	}
	/*
	 * Step 2: do the routine
	 */
	val = 1.0;
	for (high = 5, n = 0; n < i64; n++, high += 5) {
		if (high > A_HEIGHT)
			high = 5;	/* wrap around */
		wide = high + 1;
		/*
		 * First, fetch fresh values for matrices
		 */
		for (j = 0; j < high; j++)
			for (k = 0; k < wide; k++) {	/* fetch a bunch of values */
				m(k, j) = val;	/* make it a real one and non-linear */
				val = val * 1.00001 + 1.0;
				if (val > 1e20)
					val -= 1e19;	/* make sure never generate same values */
			}
		memcpy(a_orig, a, A_SIZE * sizeof (double));	/* save the original for test() later */
		/*
		 * Now do the matrix arithmetic
		 */
		if (!solve_array(a, results, wide, high)) {
			fprintf(stderr, "Singular Matrix of size %d\n", high);
			return -1;
		}
		if (!test(a_orig, results, wide, high)) {
			fprintf(stderr,
				"Illegal results from matrix of size %d\n",
				high);
			return -1;
		}
		COUNT_BUMP;
	}
	COUNT_END("array_rtns");
	return 0;
}

static int
string_rtns_1(char *argv,
	      Result * res)
     /*
      * Work common string lengths into and out of string functions
      */
{
	int
	  n, i64, itemp1, itemp2;
	long
	  ltemp1, ltemp2, ltemp3;
	double
	  dtemp1, dtemp2, dtemp3;

	time_t ttemp1;
	char
	 *p, buffer[1024], catbuf[32];	/* make it larger */

	COUNT_START;

	/*
	 * get args
	 */
	if (sscanf(argv, "%d", &i64) < 1) {
		fprintf(stderr, "string_rtns_1(): needs 1 argument!\n");
		return -1;
	}
	/*
	 * Clear buffer and initialize values
	 */
	memset(buffer, '\0', sizeof (buffer));	/* do it */
	itemp1 = 0;			/* clear counter */
	time(&ttemp1);			/* get the current time */
	for (n = 0; n < i64; n++) {
		dtemp1 = (double)itemp1 / 1000.0;	/* get a value */
		sprintf(buffer, "%g", dtemp1);	/* convert to ascii */
		dtemp2 = atof(buffer);	/* convert back to binary */
		dtemp3 = strtod(buffer, &p);	/* convert also */

		ltemp1 = 1000 * dtemp2;	/* make us a 4 digit integer */
		sprintf(buffer, "%ld", ltemp1);	/* convert to ascii */
		ltemp2 = atol(buffer);	/* convert to long */
		itemp1 = atoi(buffer);	/* convert to integer */
		ltemp3 = strtol(buffer, &p, 10);	/* convert to long */

		sprintf(buffer,
			" \t\n\r\b0123456789 abcde fghij klmno pqrst uvwxy z ABCDE FGHIJ KLMNO PQRST UVWXY Z .,;:!@#$%%^&*()_-+=|\\\"'`[]}{");

		for (p = buffer; p != &buffer[sizeof (buffer) - 1]; p++) {	/* will bump into lots of nulls (real world example) */
			if (isalnum(*p))
				itemp1++;	/* bump counter */
			if (isalpha(*p))
				itemp1++;	/* bump counter */
			if (iscntrl(*p))
				itemp1++;	/* bump counter */
			if (isdigit(*p))
				itemp1++;	/* bump counter */
			if (isgraph(*p))
				itemp1++;	/* bump counter */
			if (islower(*p)) {	/* twiddle some chars */
				itemp1++;	/* bump counter */
				*p = toupper(*p);
				*p = tolower(*p);	/* make it lower case */
			}
			if (isprint(*p))
				itemp1++;	/* bump counter */
			if (ispunct(*p))
				itemp1++;	/* bump counter */
			if (isspace(*p))
				itemp1++;	/* bump counter */
			if (isupper(*p)) {	/* twiddle some chars */
				itemp1++;	/* bump counter */
				*p = tolower(*p);	/* make it lower case */
				*p = toupper(*p);
			}
			if (isxdigit(*p))
				itemp1++;	/* bump counter */
		}			/* end of inner loop */
		sprintf(catbuf, "What is your name?");
		itemp2 = strlen(catbuf);
		sprintf(buffer, "%s", catbuf);
		if (strcmp(buffer, catbuf) == 0)
			itemp2++;	/* should always happen */
		while ((sizeof (buffer) - strlen(buffer)) > strlen(catbuf)) {
			strcat(buffer, catbuf);
			if (strcmp(buffer, catbuf) == 0)
				itemp2++;	/* should always happen */
		}
		strcpy(buffer, catbuf);	/* move it over */
		while ((sizeof (buffer) - strlen(buffer)) > strlen(catbuf)) {
			strncat(buffer, catbuf, 10);
			if (strncmp(buffer, catbuf, 10) == 0)
				itemp2++;	/* should always happen */
		}
		p = ctime(&ttemp1);	/* calculate the time */
		ttemp1++;		/* bump it */
		strncpy(buffer, p, strlen(p));	/* copy it to buffer */
		if (strcoll(buffer, catbuf) == 0)
			itemp1++;	/* shouldn't ever happen */
		if (strcspn(buffer, "jfamsond") > (size_t) 4)
			itemp1++;	/* shouldn't happen either */
		if (strpbrk(buffer, " ;\n") == NULL)
			itemp1++;	/* shouldn't happen */
		if (strrchr(buffer, '\n') == NULL)
			itemp1++;	/* should happen every time */
		if (strspn(buffer, "abcdefghijklmnopqrstuvwxyz") > (size_t) 5)
			itemp1++;
		if (strstr(buffer, buffer) != buffer)
			itemp1++;	/* shouldn't happen */
		if (itemp2 > 100) {	/* use itemp2 some */
			itemp2 = 0;	/* clear counter */
			itemp1++;	/* bump other one */
		}			/* so itemp2 can't be eliminated */
		COUNT_BUMP;
	}				/* end of loop */
	COUNT_END("string_rtns");
	return (itemp1 >= 0) ? 0 : 1;	/* return the counter */
}

static int
mem_rtns_1(char *argv,
	   Result * res)
{
	int n, i64, i, j;
	char *p;
	char *array[MAXSTRINGS * 3];	/* allocate alot of strings */
	static int malloc_size[] = {	/* table of sizes, roughly real world representative */
		1, 3, 5, 7, 9,		/* 5 25 */
		11, 33, 65, 129, 1024,	/* 5 1262 */
		1025, 1023, 8192, 4096, 4095,	/* 5 18431 */
		11, 33, 65, 129, 1024,	/* 5 1262 */
		1, 3, 5, 7, 9,		/* 5 25 */
		11, 33, 65, 129, 1024,	/* 5 1262 */
		1025, 1023, 8192, 4096, 4095,	/* 5 18431 */
		11, 33, 65, 129, 1024
	};				/* 5 1262 */
	static char *str = "Hello, this is an average length string 1234";
	int count = 0;

	COUNT_START;
	/*
	 * Get args 
	 */
	if (sscanf(argv, "%d", &i64) < 1) {
		fprintf(stderr, "mem_rtns_1(): needs 1 argument!\n");
		return -1;
	}
	/*
	 * do the loop
	 */
	for (n = 0; n < i64; n++) {
		/*
		 * First, allocate the memory
		 */
		for (j = i = 0; i < MAXSTRINGS; i++) {
			array[j] = malloc(strlen(str) + 1);
			strcpy(array[j], str);
			count += strlen(array[j++]) + 1;	/* add its length to count */
			array[j++] = malloc(malloc_size[i % Members(malloc_size)]);	/* allocate a block */
			count += malloc_size[i % Members(malloc_size)];	/* update counter */
			array[j++] = calloc(malloc_size[(i + 1) % Members(malloc_size)], 1);	/* allocate a zeroed block */
			count += malloc_size[(i + 1) % Members(malloc_size)];	/* update counter */
		}
		/*
		 * Now touch each of these to force them to be located in main memory
		 */
		for (i = 0; i < MAXSTRINGS * 3; i++) {
			p = array[i];	/* point to it */
			count += *p++;	/* update the count */
			if (count < 0)	/* this won't ever happen */
				count += *p++;	/* update the count */
		}
		/*
		 * Now deallocate the memory in a slightly different order
		 * which isn't quite worst case, but more reasonably reflects
		 * the real world.
		 */
		for (i = (3 * MAXSTRINGS) - 3; i >= 0; i -= 3) {
			free(array[i]);	/* free in reverse order */
			COUNT_BUMP;
			free(array[i + 2]);	/* free in reverse order */
			COUNT_BUMP;
			free(array[i + 1]);	/* free in reverse order */
			COUNT_BUMP;
		}
		res->d += count;	/* prevent compiler from optimizing */
	}
	COUNT_END("mem_rtns_1");
	return 0;
}

static int
mem_rtns_2(char *argv,
	   Result * res)
{
	int n, i64, i, off1, off2, size;
	static int op_size[] = {
		1, 3, 5, 7, 9,
		11, 33, 65, 129, 1024,
		1025, 1023, 8192, 4096, 4095,
		11, 33, 65, 129, 1024,
		1, 3, 5, 7, 9,
		11, 33, 65, 129, 1024,
		1025, 1023, 8192, 4096, 4095,
		11, 33, 65, 129, 1024
	};

	char
	 
		buff1[SIZBUF + sizeof (double)],
		buff2[SIZBUF + sizeof (double)], *bp1 = buff1, *bp2 = buff2;

	char *p;

	COUNT_START;

	if (sscanf(argv, "%d", &i64) < 1) {
		fprintf(stderr, "mem_rtns_2(): needs 1 argument!\n");
		return -1;
	}
	for (n = 0; n < i64; n++) {
		/*
		 * Calculate the offsets (assume memory alignments between 0 and sizeof(double)
		 */
		off1 = n % sizeof (double);
		off2 = (n / sizeof (double)) % sizeof (double);
		size = op_size[n % Members(op_size)];
		/*
		 * Do some operations using these offsets
		 */
		memset(bp1 + off1, 'A', size);
		memset(bp2 + off2, 'A', size);
		(bp1 + off1)[n % size] = 'Z';	/* change one to Z */
		p = (char *)memchr(bp1 + off1, 'Z', size);	/* scan it */
		i = memcmp(bp1 + off1, bp2 + off2, size);	/* compare the two */
		if (i > 0)
			*p = 'A';	/* make it normal again */
		memcpy(bp1 + off1, bp2 + off2, size);	/* do the copy */
		memmove(bp2 + off1, bp1 + off2, size);
		COUNT_BUMP;
	}
	COUNT_END("mem_rtns_2");
	return count;
}

/*
 * These routines are used by qsort and bsearch in next function.
 */
static int
compar1(const void *p1,
	const void *p2)
{
	return (*(int *)p1 - *(int *)p2);	/* make it ascending */
}

static int
compar2(const void *p1,
	const void *p2)
{
	return (*(int *)p2 - *(int *)p1);	/* make it descending */
}

static int
sort_rtns_1(char *argv,
	    Result * res)
{
	int n, i64, i, count, help;
	void *p;

	COUNT_START;

	if (sscanf(argv, "%d", &i64) < 1) {
		fprintf(stderr, "sort_rtns_1(): needs 1 argument!\n");
		return -1;
	}

	help = MAXSORTSIZE;
	for (count = n = 0; n < i64; n += 5) {	/* table starts out ascending */
		qsort((char *)table, MAXSORTSIZE, sizeof (table[0]), compar1);	/* worst case is in order... */
		COUNT_BUMP;
		qsort((char *)table, MAXSORTSIZE, sizeof (table[0]), compar2);	/* best case is out of order */
		COUNT_BUMP;
		qsort((char *)table, MAXSORTSIZE, sizeof (table[0]), compar1);
		COUNT_BUMP;
		for (i = 0; i < MAXSORTSIZE * 2; i += 10) {	/* look for lots which aren't there */
			p = bsearch((const void *)&i,	/* which is more "real world" than */
				    (const void *)table, MAXSORTSIZE,	/* only looking for what is there */
				    sizeof (table[0]), compar1);	/* must use compar routine of last qsort */
			if (p == NULL)
				count++;
		}
		COUNT_BUMP;
	}
	COUNT_END("sort_rtns_1");
	return count;
}

static int
shell_rtns_1(char *argv,
	     Result * res)
{
	int n, i64, current_user_load;
	char cmdline[1024];

	COUNT_START;

	if (sscanf(argv, "%d %d", &i64, &current_user_load) < 1) {
		fprintf(stderr, "shell_rtns_1(): needs 2 arguments!\n");
		return -1;
	}

	sprintf(cmdline, "aim_1.sh %d", current_user_load);
	COUNT_BUMP;

	n = aim_system(cmdline);

	COUNT_END("shell_rtns_1");
	return n;
}

static int
shell_rtns_2(char *argv,
	     Result * res)
{
	int n, i64, current_user_load;
	char cmdline[1024];

	COUNT_START;

	if (sscanf(argv, "%d %d", &i64, &current_user_load) < 1) {
		fprintf(stderr, "shell_rtns_2(): needs 2 arguments!\n");
		return -1;
	}

	sprintf(cmdline, "aim_2.sh %d", current_user_load);
	COUNT_BUMP;

	n = aim_system(cmdline);

	COUNT_END("shell_rtns_2");
	return n;
}

static int
shell_rtns_3(char *argv,
	     Result * res)
{
	int n, i64, current_user_load;
	char cmdline[1024];

	COUNT_START;

	if (sscanf(argv, "%d %d", &i64, &current_user_load) < 1) {
		fprintf(stderr, "shell_rtns_3(): needs 2 arguments!\n");
		return -1;
	}

	sprintf(cmdline, "aim_3.sh %d", current_user_load);
	COUNT_BUMP;

	n = aim_system(cmdline);

	COUNT_END("shell_rtns_3");
	return n;
}

static int
dir_rtns_1(char *argv,
	   Result * res)
{
	int n, i64, count;
	DIR *dir;			/* file pointer */
	struct dirent *d;		/* entry pointer */

	COUNT_START;

	if (sscanf(argv, "%d", &i64) < 1) {
		fprintf(stderr, "dir_rtns_1(): needs 1 argument!\n");
		return -1;
	}

	dir = opendir("/bin");		/* open a known directory */
	if (dir == NULL) {
		fprintf(stderr, "dir_rtns_1(): unable to open /bin\n");
		return (-1);		/* die here */
	}
	i64 *= 100;			/* scale it some */
	for (count = n = 0; n < i64; n++) {
		d = readdir(dir);	/* read one more entry */
		if (d == NULL) {
			count++;	/* bump it at reaching end of file */
			rewinddir(dir);	/* and then rewind it */
		}
		COUNT_BUMP;
	}
	(void)closedir(dir);		/* close it down */
	COUNT_END("dir_rtns_1");
	return count;
}

static int
misc_rtns_1(char *argv,
	    Result * res)
{
	int n, i64, status;
	speed_t speed1, speed2;
	struct termios t;
	clock_t clk;
	char *term, *env, *tty;
	char buffer[2048];		/* make it big */
	uid_t uid, euid;
	gid_t gid, egid, groups[NGROUPS_MAX];
	pid_t pid, pgrp, ppid;
	struct tms tms;
	struct utsname name;
	struct passwd *pass1, *pass2;
	struct group *gr2;

	COUNT_START;

	if (sscanf(argv, "%d", &i64) < 1) {
		fprintf(stderr, "misc_rtns_1(): needs 1 argument!\n");
		return -1;
	}

	for (n = 0; n < i64; n++) {
		speed1 = cfgetispeed(&t);
		speed2 = cfgetospeed(&t);
		clk = clock();		/* get current time */
		if ((term = ctermid(NULL)) == NULL) {	/* get filename of terminal */
			fprintf(stderr, "misc_rtns_1(): ctermid() failed\n");
			return (-1);	/* return error */
		}
		if (getcwd(buffer, sizeof (buffer)) == NULL) {	/* get current directory */
			fprintf(stderr, "misc_rtns_1(): getcwd() failed\n");
			return (-1);	/* return error */
		}
		uid = getuid();
		euid = geteuid();
		gid = getgid();
		egid = getegid();
		if ((env = getenv("PATH")) == NULL) {	/* get path entry */
			fprintf(stderr,
				"misc_rtns_1(): getenv('PATH') failed\n");
			return (-1);	/* return error */
		}
		if (getgroups(NGROUPS_MAX, groups) == -1) {	/* get groups */
			fprintf(stderr, "misc_rtns_1(): getgroups() failed\n");
			return (-1);	/* return error */
		}
		pid = getpid();
		pgrp = getpgrp();
		ppid = getppid();
		if ((pass1 = getpwnam("root")) == (struct passwd *)NULL) {
			fprintf(stderr, "misc_rtns_1(): getpwnam() failed\n");
			return (-1);	/* return error */
		}
		if ((pass2 = getpwuid(uid)) == (struct passwd *)NULL) {
			fprintf(stderr, "misc_rtns_1(): getpwuid() failed\n");
			return (-1);	/* return error */
		}
		if ((gr2 = getgrgid(gid)) == (struct group *)NULL) {
			fprintf(stderr, "misc_rtns_1(): getgrgid() failed\n");
			return (-1);	/* return error */
		}
		if ((clk = times(&tms)) == -1) {	/* get the times */
			fprintf(stderr, "misc_rtns_1(): times() failed\n");
			return (-1);	/* return error */
		}
		if (isatty(STDIN_FILENO))
			clk++;		/* should happen sometimes */
		tty = ttyname(STDIN_FILENO);	/* get tty name */
		if ((status = uname(&name)) == -1) {
			fprintf(stderr, "misc_rtns_1(): uname() failed\n");
			return (-1);	/* return error */
		} else if (status == -2)	/* should never happen */
			fprintf(stderr, "bogus format line%d%d%d%d%s%s%s%s%d%d%d%d%d%d%d%d%d%s%s%s%s", speed1, speed2, t.c_iflag, clk, term, env, tty, buffer, uid, euid, gid, egid, groups[0], pid, pgrp, ppid, tms.tms_utime, name.sysname, pass1->pw_name, pass2->pw_name, gr2->gr_name);	/* let compiler think we're using */
		COUNT_BUMP;
	}
	COUNT_END("misc_rtns");
	return 0;
}

/* use fork() so we don't get an interrupt signal during system call */
static int
aim_system(char *cmdline)
{
	int val, result, status;

	val = fork();
	if (val == 0) {			/* we're in the child */
		result = system(cmdline);	/* run the command */
		exit(result);		/* return the value */
	} else {
		while (1) {		/* now wait for done */
			if (wait(&status) > 0) {	/* if one has completed OK */
				if ((status & 0377) == 0177)	/* child proc stopped */
					(void)fprintf(stderr,
						      "\nChild process stopped by signal #%d\n",
						      ((status >> 8) & 0377));
				else if ((status & 0377) != 0) {	/* child term by sig */
					if ((status & 0377) & 0200) {
						(void)fprintf(stderr,
							      "\ncore dumped\n");
						(void)fprintf(stderr,
							      "\nChild terminated by signal #%d\n",
							      (status & 0177));
					}
				} else {	/* child exit()ed */
					if (((status >> 8) & 0377))	/* isolate status */
						(void)fprintf(stderr,
							      "\nChild process called exit(), status = %d\n",
							      ((status >> 8) &
							       0377));
				}
			} else if (errno == ECHILD)
				break;
			else if (errno == EINTR)
				continue;
		}
	}
	return 0;
}
