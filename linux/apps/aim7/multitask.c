
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

char *version = "1.1";			/* establish version */
char *release_date = "January 22, 1996";	/* and release */
char *benchmark_name = "AIM Multiuser Benchmark - Suite VII";	/* and name */

#define SS_7 "suite7.ss"		/* benchmark spreadsheet output file */

#include <stdio.h>			/* enable printf(), etc. */
#include <stdlib.h>			/* enable atol(), etc. */
#include <signal.h>			/* enables signal handling */
#include <time.h>			/* enables ctime(), etc. */
#include <errno.h>			/* enable errno, etc.  */
#include <unistd.h>			/* getpid(), etc. */
#ifndef NO_ULIMIT
#include <ulimit.h>			/* enable ulimit(), etc. */
#endif
#include <string.h>			/* enables strcat(), etc. */
#include <sys/types.h>			/* used for setpgid(), etc. */
#include <sys/stat.h>			/* used for creat(), etc. */
#include <fcntl.h>			/* used for creat(), etc. */
#include <sys/times.h>			/* enables times(), etc. */
#include <sys/wait.h>			/* enable wait(), etc. */
#include <sys/param.h>			/* get HZ, etc. */
#include <math.h>

#include "testerr.h"
#include "suite.h"
#include "files.h"

#define  SLEEP 10			/* #seconds to sleep before send start signal to kids */
#define MAXCMDARGS (100)
Cargs cmdargs[MAXCMDARGS];		/* filled by register_test() */
Result results[LOADNUM + MAXCMDARGS];	/* holds return values from test functions */

								 /*
								  * all tests run at least once, sometimes pushes us over LOADNUM 
								  */
int num_cmdargs = 0;

source_file *
benchmark_c()
{
	static source_file s = { " @(#) multitask.c:1.40 3/21/94 11:43:55",	/* SCCS info */
		__FILE__, __DATE__, __TIME__
	};

	return &s;
}

#define NOCROSSOVER 0
#define CROSSOVER   1
#define INFINITY 999999999
#define ADAPT_START     8		/* start adaptive timer after 8 datapoints */
#define THRESHOLD_MIN   1.20
#define THRESHOLD_MAX   1.30
#define JUST_TEMP_FILES	0
#define ALL_FILES	1
#define MAXDPTS		1000

double
  a_tn1, a_tn, tasks_per_minute,	/* JPM = Jobs Per Minute */
  tasks_per_minute_per_user;		/* JPM = Jobs Per Minute per user */

struct tms t[2];			/* for times calls */
struct tms start_times, end_times;	/* for logfile times calls */

pid_t parent_pid;			/* parent's PID */

int
 *p_i1, disk_iteration_count;		/* specified by user, test file size in kbytes */

static int
  a_cnt,				/* count data points for adaptive timer */
  gun,					/* start all processes running */
  work,					/* work load for processes */
  numdir,				/* number of disk directories */
  kids,					/* number of users forked */
  kids_forked,				/* keeps track of forking children */
  maxusers[MAXITR],			/* max users at once */
  minusers[MAXITR],			/* min number of users */
  incr[MAXITR],				/* skip incr number each run */
  termination[MAXITR],			/* CROSSOVER or NOCROSSOVER */
  filesize,				/* test file size */
  poolsize,				/* size of a disk pool for common use */
  iters, no_logfile,			/* don't print out logfile */
  aim_fork(),				/* our new fork function */

  disk_create_all_files(),		/* functions */

  adjust_adaptive_timer_increment();

char
  ss_file[NBUFSIZE],			/* output filename */
  datapt_files[MAXITR][256],		/* files holding datapoints */
  file_buffer[NBUFSIZE],		/* buffer for file creation */
  fn1[STRLEN],				/* STRLEN = 80 */
  fn1arr[MAXDRIVES][STRLEN],		/* list of tmpa.common files created */
  fn2arr[MAXDRIVES][STRLEN],		/* list of fakeh directories created */
  dkarr[MAXDRIVES][STRLEN],		/* dirs for disk thrasher from config file */
  mname[256],				/* machine name */
  current_test[256],			/* keep track of current test for signal_handlers */
  mconfig[256],				/* machine configuration */
 *ldate,				/* date from ctime */
  sdate[25];				/* date again copied from ctime */

time_t vtime;

long
  rmtsec(),
  hertz,				/* HZ */

 *p_fcount,
  debug,
  use_file,
  use_Newton,
  t1,
  t2,
  now,
  rt1,
  rt2,					/* time variables */

  period;				/* calibration timer */

struct _tasks {				/* command / weight array */
	int hits;			/* iterations */
	char cmd[50];			/* name */
} tasks[WORKLD];			/* holds 'workfile' contents */

struct _datapoints {
	int cnt;			/* total # points */
	int points[MAXDPTS];
} datapoints[MAXITR];			/* holds datapt_files[] contents */

void
  begin_timing(),			/* before each test */

  end_timing(),				/* after each test */

  initialize_global_variables(),	/* initialize variables */

  disk_unlink_all_test_files(),
  output_suite_header(),		/* print output */

  output_suite_out(),
  dump_results(),
  runtap(),
  death_of_child(),
  kill_all_child_processes(),
  flag_running_processes(),		/* watches kids fork */

  start_child_processes(),
  cleanup(),				/* return system resources */

  math_signal_handler(),
  get_misc_information(),
  read_datapoints(),			/* get load values from file if specified by user */

  clear_ipc();

FILE *logfile = NULL;			/* holds timing information */

int
main(int argc,
     char **argv)
{
	FILE * fp;			/* file pointer */

	source_file *(*s) ();		/* pointer to function */

	int
	  i, n,				/* counters */
	  dpts, p, adapt, verbose,	/* print details at startup */
	  runnum,			/* run number */
	  inc;				/* # users to inc per iteration */

	void (*sigvalu) ();		/* signal handler pointer */

	char
	  buf[132];			/* line buffer for reading */

	/*
	 * Step 1: handle signals
	 */
	parent_pid = getpid();		/* get my pid */
	sigvalu = signal(SIGINT, SIG_IGN);	/* get status of SIGINT */
	if (sigvalu == SIG_IGN)		/* if ignoring interrupts */
		(void)setpgid(0, parent_pid);	/* change process group */
	else				/* else */
		(void)signal(SIGINT, sigvalu);	/* restore handler */
	(void)signal(SIGINT, SIG_IGN);	/* catch signal and ignore */
	(void)signal(SIGTERM, kill_all_child_processes);	/* child process problem */
#ifndef NO_ULIMIT
	(void)ulimit(2, 1L << 20);	/* 1 meg blocks for max file size */
#endif
	/*
	 * Step 2: parse command line options
	 */
	adapt = 1;			/* turn on adapt timer (DEFAULT) */
	verbose = 0;			/* turn off verbose (DEFAULT) */
	no_logfile = 0;			/* print out the logfile (DEFAULT) */
	debug = 0;			/* turn off debug (DEFAULT) */
	use_file = 0;			/* don't expect datapoint file (DEFAULT) */
	use_Newton = 0;			/* don't use Newton's Method (DEFAULT) */
	while (--argc > 0) {		/* scan the command line */
		if ((*++argv)[0] == '-') {	/* look at first char of next */
			switch ((*argv)[1]) {	/* look at second */
			case 'd':
				debug = atol(&(*argv)[2]);	/* debugging always on */
				if (!debug)
					debug = 1;	/* default to level 1 */
				break;	/* skip */

			case 'v':
				verbose = 1;	/* print details at startup */
				break;	/* skip */

			case 'n':	/* -nl */
				if ((*argv)[2] == 'l')
					no_logfile = 1;	/* don't print out the logfile */
				break;	/* skip */

			case 't':
				adapt = 0;	/* don't use adaptive timer */
				break;	/* skip */

			case 'f':
				use_file = 1;	/* prompt for datapoint file */
				break;	/* skip */

			case 'N':
				use_Newton = 1;	/* use Newton's Method to converge on crossover */
				break;	/* skip */

			default:	/* other options */
				(void)fprintf(stderr, "Unknown option --> %s\n", *argv);	/* print it */
				(void)fprintf(stderr, "Usage: %s [-t] [-v] [-dn] [-f] [-N]\n", *argv);	/* and more */
				(void)fprintf(stderr, "  -t     turn off adaptive timer\n");	/* and more */
				(void)fprintf(stderr, "  -v     verbose\n");	/* and more */
				(void)fprintf(stderr, "  -dn    turn on debug level n\n");	/* and more */
				(void)fprintf(stderr,
					      "  -f     prompt for file of datapoints\n");
				(void)fprintf(stderr,
					      "  -nl    do not print out the logfile\n");
				(void)fprintf(stderr,
					      "  -N     use Newton's Method to converge on crossover point\n\n");
				exit(1);	/* drop dead here */
				break;	/* make lint happy */
			}		/* end of switch */
		}			/* end of if */
	}				/* end of while */

	if (adapt) {			/* talk to human about adaptive */
		if (use_file) {
			(void)printf
				("\nThe -f option supercedes the adaptive timer.\n\n");
		} else {
			(void)printf
				("\nYou have chosen to use the adaptive timer.\n\n");
			(void)printf
				("You need to provide the initial increment for the operation load\n");
			(void)printf
				("so that the adaptive timer logic has a starting point to base\n");
			(void)printf("its calculations.\n");
			(void)printf
				("Use \"multitask -t\" to run without the adaptive timer.\n\n");
		}
	} else {
		if (use_file) {
			(void)printf("\n-f supercedes -t flag\n\n");
		}
		if (use_Newton) {
			(void)printf("\n-t supercedes -N flag\n\n");
		}
	}
	/*
	 * Step 3: Initialize global variables
	 */
	initialize_global_variables();	/* do the initialization */
	
	(void)printf("\n%s v%s, %s\n", benchmark_name, version, release_date);
	(void)printf
		("Copyright (c) 1996 - 2001 Caldera International, Inc.\n");
	(void)printf("All Rights Reserved.\n\n");
#ifdef COUNT
	(void)printf
		("This version compiled with counting option -- all results invalid.\n");
#endif
	if (verbose)
		(void)printf("This version compiled at %s on %s\n", __TIME__,
			     __DATE__);
	if (!no_logfile) {
		logfile = fopen("logfile.suite7", "a");	/* create log file */
		if (logfile == NULL) {
			(void)fprintf(stderr, "\nUnable to open log file.\n");
			exit(1);
		}
		if (verbose)
			(void)printf("Logging output to file `logfile'.\n");
	}
	get_misc_information();		/* ask user lots of questions */

	/*
	 * Call initialization routines for each separate file
	 */
	register_test("NOCMD", "NOARGS", (int (*)())0, 0, "You shouldn't see this");	/* register first one */

	if (verbose) {
		printf("\nFile\t\tDate\t\tTime\t\tSCCS\n");
		printf("---------------------------------------------------------\n");
	}
	for (i = 0; i < Members(source_files); i++) {
		source_file *p;

		s = source_files[i];
		p = s();		/* run it */
		if (verbose)
			printf("%-15s\t%s\t%s\t%s\n", p->filename, p->date,
			       p->time, p->sccs);
	}
	if (debug) {
		for (i = 0; i < num_cmdargs; i++) {
			printf("%s\t%s\n", cmdargs[i].name, cmdargs[i].args);
		}
	}
	/*
	 * num_cmdargs has now been set to # of "real" tests registered plus 1 for "NOCMD" 
	 */
	if (verbose) {
		printf("---------------------------------------------------------\n");
		printf("\nA total of %d discrete tests were registered.\n\n",
		       num_cmdargs);
	}

	/*
	 * Step 4: read in stuff
	 */

	if (verbose)
		(void)printf("\nNow reading parameters from file %s.", WORKFILE);	/* tell what we're doing */
	if ((fp = fopen(WORKFILE, "r")) == NULL) {	/* open work file and read in tasks */
		(void)fprintf(stderr,	/* handle error */
			      "Can't find file \"%s\" in current directory.\n",
			      WORKFILE);
		perror(argv[0]);	/* print message */
		exit(1);		/* die here */
	}				/* end of error processing */
	while (!feof(fp) && !ferror(fp)) {	/* while we're not done */
		Cargs *found, *find_arguments();
		char label[32], dim;
		int size;

		if (work >= WORKLD) {
			(void)fprintf(stderr, "Workfile too large, limited to %d entries.\n", WORKLD);	/* print mesg */
			exit(1);	/* and die */
		}
		if (fgets(buf, sizeof (buf) - 1, fp) != NULL) {	/* read in a line */
			if (*buf == '#')
				continue;	/* comment line */
			n = sscanf(buf, "%d %s", &(tasks[work].hits), tasks[work].cmd);	/* parse out hits, command */
			if ((n < 2) && (strlen(buf) > (size_t) 0)) {	/* if not empty line and bad read */
				n = sscanf(buf, "%s %d%c", label, &size, &dim);	/* format: FILESIZE: 10K */
				if ((n < 3) && (strlen(buf) > (size_t) 0)) {	/* if not empty line and bad read */
					(void)fprintf(stderr, "Error in file '%s' (line #%d)=[%s]\n",	/* error here */
						      WORKFILE, work, buf);	/* dump all info */
					(void)fprintf(stderr,
						      "Specify either a weighted test line:\n");
					(void)fprintf(stderr,
						      "<weight> <test name>\n");
					(void)fprintf(stderr,
						      "Or the file or pool size in kbytes or megabytes:\n");
					(void)fprintf(stderr,
						      "FILESIZE: <integer><KkMm>\n");
					(void)fprintf(stderr,
						      "POOLSIZE: <integer><KkMm>\n");
					exit(1);	/* die */
				}
				if ((dim == 'M') || (dim == 'm'))
					size *= NBUFSIZE;	/* how many 1k buffers */
				else if ((dim != 'K') && (dim != 'k')) {
					(void)fprintf(stderr, "Error in file '%s' (line #%d)=[%s]\n",	/* error here */
						      WORKFILE, work, buf);	/* dump all info */
					(void)fprintf(stderr,
						      "Specify the file or pool size in kbytes or megabytes:\n");
					(void)fprintf(stderr,
						      "FILESIZE: <integer><KkMm>\n");
					(void)fprintf(stderr,
						      "POOLSIZE: <integer><KkMm>\n");
					exit(1);	/* die */
				}
				if ((*label == 'F') || (*label == 'f'))
					filesize = size;
				else if ((*label == 'P') || (*label == 'p'))
					poolsize = size;
				else {
					(void)fprintf(stderr, "Error in file '%s' (line #%d)=[%s]\n",	/* error here */
						      WORKFILE, work, buf);	/* dump all info */
					(void)fprintf(stderr,
						      "Specify the file or pool size in kbytes or megabytes:\n");
					(void)fprintf(stderr,
						      "FILESIZE: <integer><KkMm>\n");
					(void)fprintf(stderr,
						      "POOLSIZE: <integer><KkMm>\n");
					exit(1);	/* die */
				}
				continue;	/* don't update 'work' variable */
			}		/* end of error processing */
		} /* end of read ok */
		else if (!feof(fp)) {	/* if not ok read and not eof */
			if (work) {	/* first line? */
				perror(argv[0]);	/* if not, */
				(void)fprintf(stderr, "Error reading file '%s' ", WORKFILE);	/* print message */
			} else		/* else its an empty file */
				(void)fprintf(stderr, "File '%s' is empty\n", WORKFILE);	/* print mesg */
			exit(1);	/* and die */
		} /* end of !feof() */
		else
			continue;
		found = find_arguments(tasks[work].cmd);
		if (strcmp(found->name, "NOCMD") == 0) {	/* flag test we haven't registered */
			(void)fprintf(stderr, "Test '%s' has not been registered.\n", tasks[work].cmd);	/* print mesg */
			exit(1);	/* and die */
		}
		work++;			/* add one more line */
	}				/* and loop until done */
	(void)fclose(fp);		/* close the file (we're done w/ it) */
	if ((filesize == 0) && (poolsize == 0)) {
		(void)fprintf(stderr,
			      "\nWorkfile must contain the file and/or pool size in kbytes or megabytes:\n");
		(void)fprintf(stderr, "FILESIZE: <integer><KkMm>\n");
		(void)fprintf(stderr, "POOLSIZE: <integer><KkMm>\n");
		exit(1);		/* and die */
	}
	for (i = 0, tasks[work].hits = 0; i < work; i++)
		tasks[work].hits += tasks[i].hits;	/* accumulate total */
	if (verbose) {
		(void)fprintf(stderr, "\n\nWorklist: \n");	/* print out weighted task list */
		for (i = 0; i < work; i++)	/* loop through list */
			(void)fprintf(stderr, "%8d %-20s\n",	/* print it out */
				      tasks[i].hits, tasks[i].cmd);	/* print values */
		(void)fprintf(stderr, "\nTotal number entries is %d\n", work);
		fprintf(stderr, "FILESIZE is %d POOLSIZE is %d\n", filesize,
			poolsize);
	}
	/*
	 * Step 5: start testing
	 */
	(void)printf("\n%s Run Beginning\n\n", benchmark_name);	/* talk to human */
	fflush(stdout);			/* talk to human */
	/*
	 * Step 6: Start everything running.
	 */
	for (p = 0; p < iters; p++) {	/* for each iteration */
		time(&vtime);		/* get the time */
		ldate = ctime(&vtime);	/* convert to string */
		sscanf(ldate, "%*s %21c", sdate);	/* get nov 19 12:23:56  1234 */
		sdate[20] = '\0';	/* NULL terminate it */
		output_suite_header(mname, sdate);	/* create suite.ss */
		if (!use_file)		/* not getting datapoints from file */
			inc = incr[p];	/* get increment */
		else
			inc = 0;	/* unused if get datapoints from file */
		dpts = 0;		/* keep track of datapoints from file, if specified */
		a_cnt = 0;		/* datapoint counter for this iteration */
		(void)printf
			("Tasks    jobs/min  jti  jobs/min/task      real       cpu\n");
		/*
		 * if disk file size doesn't change w/ userload, create disk files to be 
		 * used by disk tests ; otherwise, must do below on a per-userload basis 
		 */
		if (poolsize == 0) {
			disk_iteration_count = filesize;	/* set size global, poolsize 0 here */
			if (disk_create_all_files() == -1) {	/* do the creation */
				perror("disk_create_all_files()");	/* error goes here */
				(void)fprintf(stderr,
					      "disk work file creation failed\n");
				kill_all_child_processes(0);	/* kill everything */
			}
		}
		for (runnum = minusers[p]; runnum <= maxusers[p];) {	/* loop through users  */
			time_t my_time;

			/*
			 * if poolsize non-zero, disk filesize changes with userload, must re-create disk files
			 */
			if (poolsize != 0) {	/* for disk tests each time thru loop */
				/*
				 * filesize & poolsize are the "number of NBUFSIZE buffers" needed to 
				 * get the disk file sizes specified at the user prompts.
				 * we truncate instead of round so never go over poolsize
				 */
				disk_iteration_count = filesize + poolsize / runnum;	/* set size global */
				if (disk_create_all_files() == -1) {	/* do the creation */
					perror("disk_create_all_files()");	/* error goes here */
					(void)fprintf(stderr,
						      "disk work file creation failed\n");
					kill_all_child_processes(0);	/* kill everything */
				}
			}
			/*
			 * Next task load 
			 */
			(void)printf("%5d", runnum);	/*  */
			fflush(stdout);	/* talk to human */
			system("sync;sync;sync");	/* clean out the cache, boosts performance */
			runtap(runnum, LOADNUM);	/* run it */
			time(&my_time);	/* get the time */
			(void)printf("   %s", ctime(&my_time));	/* print the time */
			/*
			 * Are we done? 
			 */
			if (runnum == maxusers[p])
				break;	/* reached max users */
			if ((termination[p] == CROSSOVER) && (tasks_per_minute_per_user <= 1.0)) {	/* reached crossover */
				printf("Crossover achieved.\n");
				break;
			} else if (use_file) {
				dpts++;
				if (dpts >= datapoints[p].cnt)
					break;	/* gone thru whole list of datapoints */
			}
			/*
			 * Set up to go again 
			 */
			if (use_file) {
				runnum = datapoints[p].points[dpts];
			} else {
				if (adapt)	/* if adaptive timer */
					inc = adjust_adaptive_timer_increment(inc, p, runnum);	/* calculate next level */
				runnum += inc;	/* add the increment */
			}
			if (runnum > maxusers[p])	/* if too many, stop */
				runnum = maxusers[p];	/* else do the top */
			/*
			 * if poolsize non-zero, must unlink disk files for disk tests each time thru loop 
			 */
			if (poolsize != 0)
				disk_unlink_all_test_files(JUST_TEMP_FILES);
		}			/* end user loop */
	}				/* end iteration loop */

	/*
	 * Step 7: Unlink disk files; write results[] to file
	 */
	disk_unlink_all_test_files(ALL_FILES);	/* used by disk tests */
	dump_results();			/* save results */
	/*
	 * Step 8: We're done!
	 */
	if (!no_logfile)
		fclose(logfile);	/* close logfile */
	(void)printf("\n%s\n   Testing over\n", benchmark_name);	/* talk to human */
	return 0;			/* return success */
}

void
runtap(int current_user_load_number,
       int number_processes_per_user)
     /*
      * Current_User_Load_Number = current task load number
      * number_processes_per_user = # procs per user
      */
{
	int
	  user, i, j, dj,		/* disks array index */
	  k, status, num_children = 0, jti = 0,	/* for calculating job timing */
	  umbilical[2];			/* for forking children */

	char
	  child_string[STRLEN], cmd[256],	/* executable command to "system" */
	  params[STRLEN],		/* holds parameters for test function calls */
	 *ptr;				/* string holder */

	int
	  randnum,			/* random number */
	  (*func) (),			/* the function pointer */

	  wlist[WORKLD],		/* work load list, use workfile weight 
					 * to calc % of LOADNUM for each test*/

	  reduce_list();		/* trim list function */

	Cargs * realargs, *find_arguments();

	double
	  processes_per_second, real_time, cpu_time,	/*  */
	  new_load,			/* temporary variable */
	 
		elapsed_seconds = 0.0,
		sum_elapsed_times = 0.0,
		sumsq_elapsed_times = 0.0, std_dev = 0.0, cov = 0.0;

	/*
	 * for debug file only 
	 */
	FILE *fp, *fopen();
	static header = 0;

	if (debug)
		(void)fprintf(stderr,
			      "\nTask\tWeight\tLoad\t\tWlist\n-----------------------------------------\n");
	for (j = 0; j < work; j++) {	/* How many of each to sample? */
		/*
		 * figure what percentage of the specified # of user processes goes to this test 
		 */
		new_load = ((double)tasks[j].hits / tasks[work].hits) * number_processes_per_user;	/* calculate load */
		wlist[j] = (int)new_load;	/* make it an integer */
		if ((new_load - (double)wlist[j]) >= 0.5) {	/* round up or down */
			wlist[j] += 1;
		} else if ((new_load - (double)wlist[j]) != 0.0) {
			if (wlist[j] == 0) {	/* run each test at least once */
				wlist[j] = 1;
			}
		}
		if (debug)
			(void)fprintf(stderr, "%s\t%d\t%f\t%d\n", tasks[j].cmd,
				      tasks[j].hits, new_load, wlist[j]);
	}				/* end for */
	/*
	 * recompute the number of jobs for the selected mix 
	 */
	for (number_processes_per_user = i = 0; i < work; i++)
		number_processes_per_user += wlist[i];
	/*
	 * open pipe for communication with children, they will send a message up when they are 
	 * forked and ready to run, otherwise we may start the tests before all kids are ready
	 */
	if (pipe(umbilical) < 0) {	/* problem making a pipe */
		close(umbilical[0]);
		close(umbilical[1]);
		fprintf(stderr, "cannot create pipe to children\n");
		return;
	}
	kids_forked = 0;		/* init counter before start forking kids */
	/*
	 * establish time bases
	 */
	processes_per_second = real_time = cpu_time = 0.0;
	rtmsec(TRUE);			/* reset epoch in rtmsec */
	/*
	 * Spawn off the tasks for this run level
	 */
	for (user = 0; user < current_user_load_number; user++) {	/* for each user */
		if (aim_fork() == 0) {	/* fork off a child process */
			int arbitrary_msg = 1;

			close(umbilical[0]);	/* don't need this pipe end */
			/*
			 * Child code
			 */
			/*
			 * Step 1: seed random number generators
			 */
			aim_srand(user);	/* if we are child */
			aim_srand2(user);	/* seed random generators */
			/*
			 * Step 2: initialize signal handlers
			 */
			signal(SIGINT, start_child_processes);	/* handle signals */
			signal(SIGTERM, cleanup);
			signal(SIGQUIT, cleanup);
			signal(SIGSEGV, cleanup);
			signal(SIGHUP, death_of_child);
			signal(SIGFPE, math_signal_handler);
			/*
			 * Step 3: Wait for SIGINT from main task
			 */
			if (write(umbilical[1], &arbitrary_msg, sizeof arbitrary_msg) < 0) {	/* tell parent you're waiting */
				fprintf(stderr,
					"Child %d cannot write to parent\n",
					getpid());
				exit(-1);
			}
			close(umbilical[1]);
			pause();	/* wait for gun */
			/*
			 * Step 4: Set up mechanism for random selection of directory for writes during tests
			 */
			dj = (numdir > 0 ? (aim_rand2() % numdir) : 0);	/* numdir calculated from config file = #directories */
			/*
			 * Step 5: fork off the loads
			 */
			for (j = 0; j < number_processes_per_user; j++) {	/* do this number of tests */
				randnum = aim_rand();	/* get a number */
				k = randnum % work;	/* Sampling without replacement */
				if (wlist[k] > 0)	/* if there is work to do */
					wlist[k]--;	/* take one away */
				else {	/* else  */
					k = reduce_list(wlist);	/* remove it from list */
					wlist[k]--;	/* and take one away */
				}
				/*
				 * Build arguments for task
				 */
				realargs = find_arguments(tasks[k].cmd);
				strcpy(current_test, realargs->name);	/* save for signal_handler printout */
				func = realargs->f;	/* point to function to run */
				ptr = "";	/* initialize pointer */
				params[0] = 0;	/* clear buffer */
				(void)sprintf(cmd, "%s ", tasks[k].cmd);	/* build string arguments */
				if (strcmp(realargs->args, "DISKS") == 0) {	/* handle DISKS as special case */
					if (numdir > 1) {	/* if we have lots of directories in config file */
						ptr = dkarr[dj % numdir];	/* choose the next one */
						dj++;	/* keep track of which one this is */
						(void)strcat(cmd, ptr);	/* copy it into command buffer */
					} else if (numdir == 1) {	/* if we only have one in config file */
						ptr = dkarr[0];	/* point to it (and only it) */
						(void)strcat(cmd, ptr);	/* copy it over into command */
					}	/* otherwise, no directory specified on config file, 
						 * no parameter, use current working directory */
					sprintf(params, "%s", ptr);	/* use params data structure */
				} else if (strcmp(realargs->args, cmdargs[0].args) != 0) {	/* else if found cmd in cmdargs */
					ptr = realargs->args;	/* initialize pointer */
					(void)strcat(cmd, ptr);	/* and add to command line */
					sprintf(params, "%s %d", ptr, current_user_load_number);	/* add task load to param list for functions */
					/*
					 * for now only the shell_rtns_1 test uses 
					 */
				}
				/*
				 * otherwise no arguments to test 
				 */
				/*
				 * * run selected test, wait for completion or try
				 * * again; keep trying to execute a command 
				 */
				errno = 0;	/* clear errors */
				(void)sprintf(child_string, "\nChild #%d: ", user);	/* build message */
				if (debug != 0) {	/* talk to humans */
					printf("%d: %s\n", user,
					       realargs->name);
					fflush(stdout);
				}
				begin_timing();	/* start the timer */
				if ((*func) (params, &results[j]) < 0) {	/* do function */
					perror(child_string);	/* if error, print it */
					(void)fprintf(stderr, "\nFailed to execute\n\t%s\n", cmd);	/* tell what happened */
					(void)kill(getppid(), SIGTERM);	/* kill parent */
					exit(1);	/* and die */
				}
				end_timing(realargs->name, current_user_load_number);	/* release the timer */
			}		/* end of testing loop */
			if (!no_logfile)
				fclose(logfile);	/* close log file */
			exit(0);	/* we're done, so stop */
			/*
			 * End of Child
			 */
		}			/* end (if fork) */
	}				/* end for loop */
	/*
	 * make sure all processes are done forking 
	 */
	while (kids_forked < kids) {
		int msg;		/* arbitrary msg from kid */

		if (read(umbilical[0], &msg, sizeof msg) < 0) {
			fprintf(stderr, "cannot read message from child\n");
			return;
		}
		kids_forked++;
	}
	close(umbilical[1]);
	close(umbilical[0]);
	t1 = CHILDHZ(0);
	rt1 = rtmsec(FALSE);		/* start timers */
	sleep(SLEEP);			/* pause() "hangs" on some machines */
	(void)kill(0, SIGINT);		/* fire the gun to whole process group! */

	while (1) {			/* now wait for done */
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
						      ((status >> 8) & 0377));
				else {
					now = rtmsec(FALSE);
					elapsed_seconds = (now - rt1) / 1000.0;	/* calculate time this child took to run */
					sum_elapsed_times += elapsed_seconds;	/* keep a sum of elapsed times for calculating avg later */
					sumsq_elapsed_times += (elapsed_seconds * elapsed_seconds);	/* keep a sum of the square of elapsed times for calculating
													 * standard deviation later */
					num_children++;	/* keep track of number or data points */
					if (!no_logfile) {
						fprintf(logfile,
							"%d %s %g %g %g\n",
							current_user_load_number,
							"job_timing",
							elapsed_seconds *
							1000.0, 0.0, 0.0);
						fflush(logfile);
					}
				}
			}
			--kids;		/* one fewer kid */

			if (kids == 0)
				break;	/* if no more, done */
		} else if (errno == ECHILD)
			break;
	}
	t2 = CHILDHZ(0);		/* get ending timer values */
	rt2 = rtmsec(FALSE);
	/*
	 * testing over -- calculate results
	 */
	real_time = rt2 / 1000.0 - rt1 / 1000.0;	/* calculate time for stats */
	processes_per_second = (double)number_processes_per_user / real_time;	/* calculate procs/second */
	tasks_per_minute = processes_per_second * 60.0 * (double)current_user_load_number;	/* calculate jobs per minute */
	if (num_children == 1) {	/* calculate Job Timing Index */
		std_dev = cov = 0.0;
		jti = 100.0;
	} else if (num_children != 0) {
		std_dev =		/* calculate standard deviation */
			sqrt((sumsq_elapsed_times -
			      sum_elapsed_times * sum_elapsed_times /
			      num_children) / num_children);
		if (sum_elapsed_times == 0)	/* calculate scheduler "coefficient of variation": */
			cov = 0;	/* std_dev divided by avg */
		else
			cov = std_dev / (sum_elapsed_times / num_children);
		jti = (int)cov > 1 ? 0 : (1 - cov) * 100;	/* calculate jti:  (1 - cov)*100   */
	}
	/*
	 * otherwise jti remains as initialized, at 0 
	 */
	if (a_tn1 > 0.0)		/* update adaptive timer values */
		a_tn = a_tn1;
	a_tn1 = real_time;
	/*
	 * print realtime to screen so we can see what's going on 
	 */
	cpu_time = (t2 - t1) / (double)hertz;	/* calculate CPU time (have others) */
	tasks_per_minute_per_user = processes_per_second * 60.0;
	(void)printf("%12.2f%5d%15.4f%10.2f%10.2f",	/* print it out to screen */
		     tasks_per_minute, jti, tasks_per_minute_per_user,
		     real_time, cpu_time);
	fflush(stdout);			/* force screen update */
	output_suite_out(current_user_load_number, tasks_per_minute, real_time, cpu_time, processes_per_second, jti);	/* output the values */
	if (debug) {

		if ((fp = fopen("cov.ss", "a")) == NULL) {	/* open elapsed times file */
			fprintf(stderr, "Can't write to cov.ss file\n");
			exit(1);
		}
		if (!header++)
			fprintf(fp,
				"\nTasks\tMean\tStd Deviation\tCovariance\tJTI\n");
		fprintf(fp, "%d	%6.2f	%6.2f		   %6.2f	%d\n",
			current_user_load_number,
			sum_elapsed_times / num_children, std_dev, cov, jti);
		fprintf(fp,
			"start %ld\tstop %ld\telapsed %g\tsum_elapsed %g\tsum squared %g\tnum children %d\n",
			rt1, now, elapsed_seconds, sum_elapsed_times,
			sumsq_elapsed_times, num_children);
		fclose(fp);
	}
}

Cargs *
find_arguments(char *s)
{
	int i;				/* loop variable */

	for (i = 1; i < num_cmdargs; i++)	/* search all args list */
		if (strcmp(s, cmdargs[i].name) == 0)	/* match? */
			return (&cmdargs[i]);	/* if so, return it */

	(void)fprintf(stderr,
		      "find_arguments: can't find <%s> in cmdargs[], was it registered?\n",
		      s);
	return (&cmdargs[0]);		/* else return first entry, NOCMD */
}

static int
aim_fork()
{
	/*
	 *  aim_fork is called by runntap to control process forking, and process
	 *  numbering.  repeated attempts are made to fork, returning process
	 *  ids if successful, or -1 if no process is available.
	 */
	int k, fk;
	int save_errno;

	for (k = 0; k < 10; k++)	/* try to force off 1 process */
		if ((fk = fork()) < 0) {	/* if can't fork */
			save_errno = errno;	/* save reason we couldn't fork */
			sleep(1);	/* give system time to recover */
		} else {		/* else successful */
			++kids;		/* increase kids count */
			return (fk);	/* return pid */
		}
	errno = save_errno;		/* restore reason we couldn't fork */
	perror("aim_fork: ");		/* tell user why */
	kill_all_child_processes(0);	/* clean up any children that forked */
	return (-1);			/* shut up lint */
}

void
start_child_processes()
{
	gun = 1;			/*  start child process */
	signal(SIGINT, cleanup);	/* handle next SIGINT */
}

/*
 * dump_results()
 *
 *	Dump the data in results[] to file "results".  This
 *	is to fool optimizing compilers into thinking that
 *	data generated is needed and must be computed.
 *
 *	Tin Le 11/9/89
 */
void
dump_results()
{
	FILE *fp;			/* output file */
	int i;				/* loop variable */

	if ((fp = fopen("results", "a")) == NULL) {	/* open the file */
		perror("dump_results()");	/* if error, talk about it */
		(void)fprintf(stderr,
			      "Cannot open results data file.. EXIT\n");
		exit(3);
	}
	(void)fprintf(fp, "\n******************************************************************************\n");	/* header */
	for (i = 0; i < LIST; i++)	/* loop thorugh list */
		(void)fprintf(fp, "%10.5e\n", results[i].d);	/* output results */
	(void)fclose(fp);		/* close it */
}

static void
prompt_read(int *i,
	    int lower,
	    int upper,
	    char *msg)
{
	char buffer[128];
	int temp, flag;

	flag = 1;			/* don't iterate */
	do {				/* loop */
		if ((upper - lower) == 1)	/* minor wording nit */
			(void)sprintf(buffer, "%s [%d or %d]", msg, lower, upper);	/* format message */
		else
			(void)sprintf(buffer, "%s [%d to %d]", msg, lower, upper);	/* format message */
		(void)printf("%-60s: ", buffer);	/* print message */
		fflush(stdout);		/* force output */
		fgets(buffer, sizeof (buffer), stdin);	/* read it in */
		temp = atoi(buffer);	/* convert to binary */
		if (temp >= lower && temp <= upper)
			flag = 0;	/* legal value */
		if (flag)		/* if not */
			(void)printf
				("That value %d is illegal. Legal values are from %d to %d. Please try again.\n",
				 temp, lower, upper);
	} while (flag);
	*i = temp;
}

#ifdef PROMPT_FILE
static void
prompt_filesize(int *i,
		char *msg)
{
	char buffer[80], m_or_k[2];
	int flag, how_many;

	flag = 1;			/* don't iterate */
	do {				/* loop */
		(void)printf("%-60s: ", msg);	/* print message */
		fflush(stdout);		/* force output */
		fgets(buffer, sizeof (buffer), stdin);	/* read it in */

		if (strchr(buffer, '.') != NULL)	/* found a decimal point, no decimal input allowed */
			(void)printf
				("Specify an integral number of kilobytes or megabytes.\n");
		else if (sscanf(buffer, "%d%1s", &how_many, m_or_k) < 2) {	/* need size and unit, '1s' to handle any white space */
			if (how_many == 0) {	/* user not specifying a value */
				*m_or_k = 'k';	/* doesn't matter */
				flag = 0;
			} else
				(void)printf("Specify both size and unit.\n");
		} else if ((*m_or_k != 'm') && (*m_or_k != 'M') &&	/* must enter k, K, M, or m */
			   (*m_or_k != 'k') && (*m_or_k != 'K'))
			(void)printf
				("Specify kilobytes or megabytes [K or M].\n");
		else			/* input okay */
			flag = 0;
	} while (flag);
	if ((*m_or_k == 'k') || (*m_or_k == 'K'))	/* figure iteration count */
		*i = how_many;		/* our internal buffer is 1k */
	else
		*i = how_many * NBUFSIZE;	/* how many 1k buffers is it */
}
#endif

char *
alternate_gets(char *buf,
	       int buflen)
{
	int len;
	char *retval;

	retval = fgets(buf, buflen - 1, stdin);
	if (retval) {
		len = strlen(buf);
		if (len && buf[len - 1] == '\n')
			buf[len - 1] = '\0';
	}

	return retval;
}

void
get_misc_information()
{
	/*
	 *  get_misc_information reads machine name and the range of users to simulate from
	 *  standard input.  The config file is then read, and peripheral exercisers
	 *  set up.  Extensive error checking is performed.  The routine is exited,
	 *  killing everything if errors are encountered.
	 */
	char
	  rstring[STRLEN];		/* read from config file */
	FILE *fp;			/* file pointer */
	int j;

	/*
	 * initializations 
	 */
	for (j = 0; j < STRLEN; j++)	/* clear to null string */
		rstring[j] = '\0';
	/*
	 * Pick up Machine name, and other information (mname)
	 */
	(void)printf("%-60s: ", "Machine's name");	/* ask for name */
	fflush(stdout);
	alternate_gets(mname, sizeof (mname));	/* get it back */
	(void)printf("%-60s: ", "Machine's configuration");	/* ask for name */
	fflush(stdout);
	alternate_gets(mconfig, sizeof (mconfig));	/* get it back */

	vtime = time((long *)0);	/* get the time */
	ldate = ctime(&vtime);		/* convert ot ascii */
	sscanf(ldate, "%*s %21c", sdate);	/* isolate out what we want */
	sdate[20] = '\0';		/* null terminate it */
	if (!no_logfile) {
		fprintf(logfile, "%s\n%s\n%s\n%s\n",	/* put name, date, benchmark name to logfile */
			mname, mconfig, sdate, benchmark_name);
		fflush(logfile);	/* force output to disk */
	}
	/*
	 * Fork delay and Number of iterations (iters)
	 */
	prompt_read(&iters, 1, MAXITR, "Number of iterations to run");
	/*
	 * Values for each iteration
	 */
	for (j = 0; j < iters; j++) {	/* for each iteration */
		int answer;

		(void)printf("\nInformation for iteration #%d\n", j + 1);	/* prompt for next one */
		if (use_file) {
			(void)printf("%-60s: ", "Datapoint file");	/* ask for name */
			fflush(stdout);
			alternate_gets(datapt_files[j], sizeof (datapt_files[j]));	/* get it back */
			if (*datapt_files[j] != '\0')
				read_datapoints(j);
			minusers[j] = datapoints[j].points[0];
			maxusers[j] = INFINITY;	/* ignore max users */
			termination[j] = NOCROSSOVER;
		} else {
			prompt_read(&minusers[j], 1, 10000,	/* minimum users */
				    "Starting number of operation loads");
			prompt_read(&answer, 1, 2,	/* termination criteria */
				    "1) Run to crossover\n2) Run to specific operation load           Enter");
			if (answer == 1) {
				termination[j] = CROSSOVER;
				maxusers[j] = INFINITY;	/* ignore max users */
			} else {
				termination[j] = NOCROSSOVER;
				prompt_read(&maxusers[j], minusers[j], 10000,	/* maximum users */
					    "Maximum number of operation loads to simulate");
			}
			prompt_read(&incr[j], 1, 100,	/* increment */
				    "Operation load increment");
		}
	}				/* end iteration */

	/*
	 * Now read the config file
	 */
	fp = fopen(CONFIGFILE, "r");	/* open config file */
	if (fp == NULL) {		/* if error, stop here */
		(void)fprintf(stderr, "No config file: %s\n", CONFIGFILE);
		exit(1);
	}
	numdir = 0;			/* start with no directories */
	while (fgets(rstring, STRLEN, fp) != NULL) {	/* read 1 line from file */
		switch (rstring[0]) {	/* parse from first character */

		case '#':		/* comment */
			break;

		default:		/* Directories for disk exerciser */
			sscanf(rstring, "%s", dkarr[numdir]);
			printf("\nUsing disk directory <%s>", dkarr[numdir]);	/* echo to user */
			numdir++;
			break;
		}			/* end of switch */
	}				/* end of loop */
	printf("\n");
	(void)fclose(fp);		/* end of file */

	if (numdir)			/* directory test? */
		if ((strcmp(dkarr[0], "OFF")) == 0)	/* if 1st one is OFF */
			numdir = 0;	/* ALL is OFF */
	printf("HZ is <%d>", hertz);	/* echo to user */
}

void
kill_all_child_processes(int sig)
{
	/*
	 *  kill_all_child_processes sends signals to all child process and waits
	 *  for their death
	 */
	signal(SIGTERM, SIG_IGN);	/* ignore SIGTERM */
	disk_unlink_all_test_files(ALL_FILES);	/* unlink all files */
	if (sig == 0)			/* if signal 0, problem */
		(void)fprintf(stderr,
			      "Fatal Error! SIGTERM (#%d) received!\n\n\n",
			      sig);
	(void)fprintf(stderr, "\n%s Testing over....\n\n", benchmark_name);	/* else dy neatly */

	(void)kill(0, SIGTERM);		/* kill them */
	while (wait((int *)0) != -1);	/* wait for all users to die */
	exit(0);			/* clean termination */
}

void
death_of_child(int sig)
{
	int status;			/* holds result from wait() */

	signal(sig, SIG_IGN);		/* ignore signal */
	disk_unlink_all_test_files(ALL_FILES);	/* unlink all files */
	clear_ipc();			/* return system resources */
	(void)fprintf(stderr, "\ndeath_of_child() received signal SIGHUP (%d)\n", sig);	/* talk to human */
	if (wait(&status) > 0) {	/* while we don't have error */
		if ((status & 0377) == 0177)	/* child proc stopped */
			(void)fprintf(stderr,
				      "Child process stopped on signal = %d\n",
				      ((status >> 8) & 0377));
		else if ((status & 0377) != 0) {	/* child term by sig */
			(void)fprintf(stderr,
				      "Child terminated by signal = %d\n",
				      (status & 0177));
			if ((status & 0377) & 0200)
				(void)fprintf(stderr, "core dumped\n");
		} else			/* if child exit()'ed */
			(void)fprintf(stderr,
				      "Child process called exit(), status = %d\n",
				      ((status >> 8) & 0377));
	}
	(void)kill(getppid(), SIGTERM);	/* kill parent process */
	exit(1);			/* return error status */
}

void
cleanup(int sig)
{
	signal(sig, SIG_IGN);
	/*
	 * (void)fprintf(stderr, "\ncleanup() received signal (%d)\n", sig);
	 */
	clear_ipc();			/* return ipc resources */
	disk_unlink_all_test_files(ALL_FILES);	/* cleanup all test files */
}

void
math_signal_handler(int sig)
{
	int status;

	signal(sig, SIG_IGN);
	(void)fprintf(stderr,
		      "\nmath_signal_handler() received signal SIGFPE (%d)\n",
		      sig);
	(void)fprintf(stderr, "Floating Point Exception error\n");
	(void)fprintf(stderr, "Current test is %s\n", current_test);
	if (wait(&status) > 0) {
		if ((status & 0377) == 0177) {	/* child proc stopped */
			(void)fprintf(stderr,
				      "Child process stopped on signal = %d\n",
				      ((status >> 8) & 0377));
		} else if ((status & 0377) != 0) {	/* child term by sig */
			(void)fprintf(stderr,
				      "Child terminated by signal = %d\n",
				      (status & 0177));
			if ((status & 0377) & 0200)
				(void)fprintf(stderr, "core dumped\n");
		} else			/* if child exit()'ed */
			(void)fprintf(stderr,
				      "Child process called exit(), status = %d\n",
				      ((status >> 8) & 0377));
	}
	(void)kill(getppid(), SIGTERM);
	exit(1);
}

void
initialize_global_variables()
{
	int i, j;

	disk_iteration_count = -1;	/* set value to allow system initialization */

	gun = work = numdir = kids = filesize = poolsize = iters = a_cnt = 0;
	t1 = t2 = rt1 = rt2 = 0l;
	a_tn = a_tn1 = tasks_per_minute = 0.0;

	for (i = 0; i < LIST; i++)
		results[i].d = 0.0;

	for (j = 0; j < MAXDRIVES; j++)
		for (i = 0; i < STRLEN; i++)
			dkarr[j][i] = '\0';
#ifndef HZ
	hertz = sysconf(_SC_CLK_TCK);
	if (hertz == -1) {
		fprintf(stderr, "Cannot get HZ\n");
		exit(1);
	}
#else
	hertz = HZ;
#endif
	strcpy(ss_file, SS_7);
}

/*
 * write out the work files that disk_rd and disk_cp use.  instead of having
 * each write out its own, we'll write out one copy ... in each directory
 * specified in the config file ... and let them all share it as input.  This
 * has two benefits, the first being that the test results are no longer
 * obfuscated with the disk write times, the second is that less overall
 * disk space is required
 */

static int
disk_create_all_files()
{
	int fd1, j, k;
	static int first = 1;
	struct stat stbuf;		/* stat buffer */
	char buf[256];			/* used to build filename for stat */
	char cmd[1024];			/* tar command */
	char cwd[256];			/* hold current working dir, for tar */

	if (getcwd(cwd, 256) == NULL) {
		fprintf(stderr,
			"disk_create_all_files(): can't get current working directory\n");
		return (-1);
	}
	for (j = 0; j < sizeof (file_buffer); j++)	/* initialize buffer */
		file_buffer[j] = (char)(j % 127);

	if (numdir > 0) {		/* if any directories */
		for (j = 0; j < numdir; j++) {	/* loop through directories */
			(void)sprintf(fn1, "%s/%s", dkarr[j], TMPFILE1);	/* create file name */
			fd1 = creat(fn1,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
				    S_IROTH | S_IWOTH);
			if (fd1 < 0) {	/* if couldn't create */
				(void)fprintf(stderr, "disk_create_all_files: cannot create %s\n", fn1);	/* make error message */
				return (-1);	/* return failure */
			}
			k = disk_iteration_count;	/* make as big as user specified */
			while (k--) {	/* while not done */
				if (write(fd1, file_buffer, sizeof file_buffer) != sizeof file_buffer) {	/* write it */
					(void)fprintf(stderr, "disk_create_all_files: cannot create %s\n", fn1);	/* error message */
					(void)close(fd1);	/* close file */
					return (-1);	/* error here */
				}	/* end of error */
			}		/* end of loop */
			(void)strcpy(fn1arr[j], fn1);	/* copy file name into array */
			(void)close(fd1);	/* close the file */
			if (first) {	/* first time through */
				sprintf(buf, "%s/fakeh", dkarr[j]);	/* prepare to tar fakeh file into directory */
				(void)strcpy(fn2arr[j], buf);	/* save file name to array */
				if (stat(buf, &stbuf) < 0) {	/* stat fakeh */
					sprintf(cmd, "cd %s; tar xfo %s/fakeh.tar 2> /dev/null", dkarr[j], cwd);	/* if not there, untar it */
					/*
					 * printf("Setting up %s/fakeh\n", dkarr[j]);
 *//*
 * tell user 
 */
					system(cmd);	/* do it, return value implementation-dependent */
					if (stat(buf, &stbuf) < 0) {	/* tar failed */
						sprintf(cmd, "cd %s; tar xf %s/fakeh.tar 2> /dev/null", dkarr[j], cwd);	/* if not there, untar it */
						system(cmd);	/* do it, return value implementation-dependent */
						if (stat(buf, &stbuf) < 0) {	/* tar failed */
							fprintf(stderr, "disk_create_all_files(): cannot create %s/fakeh\n", dkarr[j]);	/* tell user */
							return (-1);
						}
					}
				}	/* end of if stat */
			}		/* end of if first */
		}			/* end of for loop */
		if (first)
			first = 0;	/* unset first outside loop so all directories processed */
	} else {			/* else no directories specified in config */
		(void)sprintf(fn1, "%s", TMPFILE1);	/* create temp file in current dir */
		fd1 = creat(fn1, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);	/* create file */
		if (fd1 < 0) {		/* if error */
			(void)fprintf(stderr, "disk_create_all_files: cannot create %s\n", fn1);	/* error message */
			return (-1);	/* report failure */
		}			/* end of error */
		k = disk_iteration_count;	/* make as big as user specified */
		while (k--) {		/* while not done */
			if (write(fd1, file_buffer, sizeof file_buffer) != sizeof file_buffer) {	/* write some */
				(void)fprintf(stderr, "disk_create_all_files: cannot create %s\n", fn1);	/* error message */
				(void)close(fd1);	/* close file */
				return (-1);	/* return failure */
			}		/* end of error */
		}			/* end of loop */
		(void)strcpy(fn1arr[0], fn1);	/* copy name into array */
		(void)close(fd1);	/* close file */
		if (first) {		/* first time through */
			first = 0;
			(void)strcpy(fn2arr[0], "./fakeh");	/* save name to array */
			if (stat("./fakeh", &stbuf) < 0) {	/* stat fakeh */
				sprintf(cmd, "tar xfo fakeh.tar 2> /dev/null");	/* if not there, untar it */
				/*
				 * printf("Setting up ./fakeh\n");
 *//*
 * tell user 
 */
				system(cmd);	/* do it, return value implementation-dependent */
				if (stat("./fakeh", &stbuf) < 0) {	/* tar failed */
					sprintf(cmd, "tar xf fakeh.tar 2> /dev/null");	/* if not there, untar it */
					system(cmd);	/* do it, return value implementation-dependent */
					if (stat("./fakeh", &stbuf) < 0) {	/* tar failed */
						fprintf(stderr, "disk_create_all_files(): cannot create ./fakeh\n");	/* tell user */
						return (-1);
					}
				}
			}		/* end of if stat */
		}			/* end of if first */
	}				/* end of else */
	return (0);			/* return success */
}

/*
 * get rid of tmpa.common files, fakeh directories and failed link_test files.
 */
void
disk_unlink_all_test_files(int which_files)
{
	int j;				/* loop variable */
	char cmd[1024];

	if (numdir > 0)			/* if using directories */
		for (j = 0; j < numdir; j++)	/* loop through them all */
			(void)unlink(fn1arr[j]);	/* unlink each file */
	else				/* else no directories */
		(void)unlink(fn1arr[0]);	/* so unlink the file */


	if (which_files == ALL_FILES) {
		if (numdir > 0)		/* if using directories */
			for (j = 0; j < numdir; j++) {	/* loop through them all */
				sprintf(cmd, "rm -fr %s", fn2arr[j]);	/* remove fakeh directory */
				system(cmd);
				sprintf(cmd, "rm -f %s/link* 2> /dev/null", dkarr[j]);	/* remove old link_test files */
				system(cmd);
		} else {		/* else no directories */
			system("rm -rf ./fakeh");	/* remove fakeh directory */
			system("rm -f ./link* 2> /dev/null");	/* remove link_test files */
		}
	}
}

/*
 * If adaptive flag is on and we have enough data points, then
 * "adaptively" adjust the increment.
 * 12/12/89 TVL
 * Once load and jobs/min come within 10% of each other, start
 * using Newton's Method to converge upon crossover.
 */
static int
adjust_adaptive_timer_increment(int inc,
				int j,
				int cur_load)
{
	double
	  tmp, tmp2, a_tn_avg;

	tmp2 = (double)inc;
	if ((++a_cnt >= ADAPT_START) || (minusers[j] >= 20)) {
		if (use_Newton &&
		    ((double)(tasks_per_minute - cur_load) /
		     (double)cur_load <= .10)) {
			tmp2 = (double)(tasks_per_minute - cur_load) / 2.0;
			if (tmp2 < 1.0)
				tmp2 = 1.0;
		} else {
			a_tn_avg = (a_tn + a_tn1) / 2.0;
			if (a_tn_avg == 0.0)
				a_tn_avg = 1.0;
			if (a_tn == 0.0)
				a_tn = 1.0;
			tmp = a_tn1 / a_tn;
			if (tmp < THRESHOLD_MIN)
				tmp2 *= (2.0 / (a_tn / a_tn_avg));
			else if (tmp > THRESHOLD_MAX)
				tmp2 /= (2.0 / (a_tn / a_tn_avg));
			if (tmp2 < 1.0)
				tmp2 = (double)inc;
		}
	}
	return ((int)tmp2);
}

/*
 * reduce_list()
 *	Used in sampling without replacement in runtap().
 *	Contributed by Jim Summerall of DEC.
 * 12/13/89 TVL
 */
int
reduce_list(int wlist[])
{
	register int i, total;
	int rlist[WORKLD];

	for (i = 0, total = 0; i < work; i++) {
		if (wlist[i] == 0)
			continue;
		else
			rlist[total++] = i;
	}
	if (total)
		i = aim_rand() % total;
	else {
		(void)fprintf(stderr, "FATAL ERROR - DIVIDE BY ZERO\n");
		(void)fprintf(stderr, "reduce_list(): total = 0\n");
		exit(1);
	}
	return (rlist[i]);
}

void
output_suite_out(int suite_users,
		 double tasks_min,
		 double real_time,
		 double cpu_time,
		 double job_per_sec_per_user,
		 int job_timing_index)
{
	FILE *fptr;

	if ((fptr = fopen(ss_file, "a")) == NULL) {
		perror("multitask");
		(void)fprintf(stderr, "Cannot open %s\n", ss_file);
		exit(3);
	}

	(void)fprintf(fptr, "%d\t%.1f\t\t%d\t%.1f\t%.1f\t%.4f\n",
		      suite_users, tasks_min, job_timing_index, real_time,
		      cpu_time, job_per_sec_per_user);
	(void)fclose(fptr);
}

void
output_suite_header(char *name,
		    char *suite_time)
{
	FILE *fptr;

	if ((fptr = fopen(ss_file, "a")) == NULL) {
		perror("multitask");
		(void)fprintf(stderr, "Cannot open %s\n", ss_file);
		exit(3);
	}
	(void)fprintf(fptr, "Benchmark\tVersion\tMachine\tRun Date\n");
	(void)fprintf(fptr, "%s\t\"%s\"\t%s\t%s\n\n", benchmark_name, version,
		      name, suite_time);
	(void)fprintf(fptr,
		      "Tasks\tJobs/Min\tJTI\tReal\tCPU\tJobs/sec/task\n");
	(void)fclose(fptr);
}

void
register_test(char *name,
	      char *args,
	      int (*f) (),
	      int factor,
	      char *units)
{
	if (num_cmdargs >= MAXCMDARGS) {
		fprintf(stderr,
			"\nInternal Error: Attempted to register too many tests.\n");
		exit(1);
	}
	cmdargs[num_cmdargs].name = name;
	cmdargs[num_cmdargs].args = args;
	cmdargs[num_cmdargs].f = f;
	cmdargs[num_cmdargs].factor = -1;	/* unused by multitask */
	cmdargs[num_cmdargs].units = units;
	num_cmdargs++;
}

void
begin_timing()
{					/* before each test */
	period = rtmsec(FALSE);		/* get the starting time */
	times(&start_times);
}

void
end_timing(char *test,
	   int loads)
{					/* after each test */
	long temp = rtmsec(FALSE);	/* get ending time */
	double delta = ((double)temp - (double)period);	/* convert to seconds */

	times(&end_times);
	if (!no_logfile) {
		fprintf(logfile, "%d %s %g %g %g\n", loads, test, delta,
			(end_times.tms_utime -
			 start_times.tms_utime) * 1000.0 / (double)hertz,
			(end_times.tms_stime -
			 start_times.tms_stime) * 1000.0 / (double)hertz);
		fflush(logfile);
	}
}

void
read_datapoints(int current_iter)
{
	FILE *fp;			/* file pointer */
	char buf[132];			/* line buffer for reading */
	int lineno = 1;
	int n, dpts = 0;

	if ((fp = fopen(datapt_files[current_iter], "r")) == NULL) {	/* open datapoints file and read in points */
		(void)fprintf(stderr,	/* handle error */
			      "Can't find file \"%s\".\n",
			      datapt_files[current_iter]);
		perror("read_datapoints");	/* print message */
		exit(1);		/* die here */
	}				/* end of error processing */
	while (!feof(fp) && !ferror(fp)) {	/* while we're not done */
		if (dpts >= MAXDPTS) {
			(void)fprintf(stderr, "Datapoint file too large, limited to %d entries.\n", MAXDPTS);	/* print mesg */
			exit(1);	/* and die */
		}
		if (fgets(buf, sizeof (buf) - 1, fp) != NULL) {	/* read in a line */
			if (*buf == '#') {	/* comment line */
				lineno++;
				continue;
			}
			n = sscanf(buf, "%d", &(datapoints[current_iter].points[dpts]));	/* parse out datapoint */
			if ((n < 1) && (strlen(buf) > (size_t) 0)) {	/* if not empty line and bad read */
				(void)fprintf(stderr, "Error in file '%s' (line #%d)=[%s]\n",	/* error here */
					      datapt_files[current_iter], lineno, buf);	/* dump all info */
				exit(1);	/* die */
			}
		} /* end of read ok */
		else if (!feof(fp)) {	/* if not ok read and not eof */
			if (dpts) {	/* first line? */
				perror("read_datapoints");	/* if not, */
				(void)fprintf(stderr, "Error reading file '%s' ", datapt_files[current_iter]);	/* print message */
			} else		/* else its an empty file */
				(void)fprintf(stderr, "File '%s' is empty\n", datapt_files[current_iter]);	/* print mesg */
			exit(1);	/* and die */
		} /* end of !feof() */
		else
			continue;
		dpts++;			/* add one more line */
		lineno++;		/* add one more line */
	}				/* and loop until done */
	datapoints[current_iter].cnt = dpts;
	(void)fclose(fp);		/* close the file (we're done w/ it) */
}
