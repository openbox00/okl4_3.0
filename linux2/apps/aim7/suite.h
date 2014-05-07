
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
#define suite_h " @(#) suite.h:1.9 1/22/96 00:00:00"

/*
 ** suite.h
 **
 **	Common include file for the benchmark.
 **
 */
#define TRUE (1)			/* define these for everyone */
#define FALSE (0)

#define EXTERN		extern
#ifdef COUNT
#define COUNT_START static int aim_iteration_test_count = 0, caim_iteration_test_count = 0;
#define COUNT_ZERO aim_iteration_test_count = 0; caim_iteration_test_count = 0
#define COUNT_BUMP  { aim_iteration_test_count++; }
#define COUNT_END(a) if (caim_iteration_test_count++ == 0) printf("Count = %d for test %s in file %s at line %d\n", aim_iteration_test_count, a, __FILE__, __LINE__);
#else
#define COUNT_START
#define COUNT_BUMP
#define COUNT_ZERO
#define COUNT_END(a)
#endif

#define WORKLD		100
#define WORKFILE	"workfile"
#define CONFIGFILE	"config"
#define STROKES		50		/* baud rate per user typing */
#define MAXITR		10		/* max number of iteration */
#define MAXDRIVES	255		/* max number of HD drives to test */
#define RTMSEC(x)	rtmsec(x)
#define CHILDHZ(x)	((long) times(&t[0]),t[0].tms_cstime+t[0].tms_cutime)
#define LIST		100		/* results array size */
#define LOADNUM		100		/* number of procs to do per user */
#define CONTINUE	1
#define STRLEN		80
#define Members(x)	(sizeof(x)/sizeof(x[0]))	/* number items in array */

/*
 * NBUFSIZE is the smallest possible test file size (1k==1024 bytes).
 * The filesize is NBUFSIZE * disk_iteration_count (normally 1 megabyte).
 * disk_iteration_count is set by the user at the input prompt
 */
#define NBUFSIZE        (2*512)		/* size of disk read/write buffer */

void killall();				/* for signal handling */
void letgo();				/*  */
void dead_kid();			/* for signal handling */
void math_err();			/*  */
int vmctl();				/* rec */
int ttyctl();				/* rec */
int tp_ctl();				/* rec */
void foo();				/* rec */

/*----------------------------------------------
 ** Declared GLOBALs for use in other files
 **----------------------------------------------
 */
EXTERN int *p_i1;
EXTERN long *p_fcount;
EXTERN long debug;

typedef struct {
	char c;
	short s;
	int i;
	long l;
	float f;
	double d;
} Result;

double next_value();			/* next value from floating point array */
long rtmsec(int);			/* return real time in ms */

unsigned int aim_rand(),
  aim_rand2();

void aim_srand(),
  aim_srand2();

#define WAITING_FOR_DISK_COUNT -99	/* marker value */
void register_test(char *name,		/* register test to be run */

		   char *args,		/* name, args */

		   int (*f) (),		/* pointer to function */

		   int factor,		/* factor (or -1 for uncalibrated) */

		   char *units);	/* units for factor */

typedef struct CARGS {
	char *name;
	char *args;
	int (*f) ();
	int factor;			/* 5/27/93 -- REC used for Suite IX's operation count */
	char *units;			/* 5/28/93 -- REC used for Suite IX's operation count */
	int run;
} Cargs;


/*
 ** Defines for disk tests
 */
#define TMPFILE1 "tmpa.common"
#define TMPFILE2 "tmpb.XXXXXXXXXX"
extern int disk_iteration_count;	/* specified by user, test file size in kbytes */

#undef EXTERN

typedef struct {			/* useful for declarations */
	char *sccs;			/* contains SCCS info */
	char *filename;			/* filename */
	char *date;			/* date of compilation */
	char *time;			/* time of compilation */
} source_file;				/* make it a typedef */
