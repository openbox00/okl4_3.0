
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
#define _POSIX_SOURCE 1			/* turn on POSIX features */

/* #define COUNT */
#include <stdio.h>			/* enable sprintf(), etc. */
#include <stdlib.h>			/* for wait(), etc. */
#include <unistd.h>			/* enable close(), pause(), etc. */
#include <sys/types.h>			/* types */
#include <sys/wait.h>			/* wait */
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>			/* for use in link_test */
#include <signal.h>			/* for signal test */
#include <setjmp.h>			/* for setjump() and longjmp() */
#include "suite.h"

static int				/* the tests in this file */
  creat_clo(),				/* create and then close files */

  brk_test(),				/* brk() alot of times */

  fork_test(),				/* fork alot of times */

  exec_test(),				/* fork and then exec alot of times */

  jmp_test(),				/* setjmp/longjmp test */

  signal_test(),			/* signal test */

  link_test(),				/* link to a file alot of times */

  page_test();				/* brk() and then dirty alot of times */

extern void
  aim_mktemp();				/* use our portable mktemp() */

#define CREAT_MODE (S_IRWXU | S_IRWXG | S_IRWXO)	/* 0777 permissions */
#define LINK_LOOP 9
#define THE_LINK_MAX 8

source_file *
creat_clo_c()
{
	static source_file s = { " @(#) creat-clo.c:1.17 3/4/94 17:19:34",	/* SCCS info */
		__FILE__, __DATE__, __TIME__
	};

	register_test("creat-clo", "DISKS", creat_clo, 1000,
		      "File Creations and Closes");
	register_test("page_test", "100", page_test, 1700,
		      "System Allocations & Pages");
	register_test("brk_test", "1000", brk_test, 17000,
		      "System Memory Allocations");
	register_test("jmp_test", "1000", jmp_test, 1000, "Non-local gotos");
	register_test("signal_test", "1000", signal_test, 1000,
		      "Signal Traps");
	register_test("exec_test", "5", exec_test, 5, "Program Loads");
	register_test("fork_test", "100", fork_test, 100, "Task Creations");
	register_test("link_test", "DISKS", link_test,
		      LINK_LOOP * (THE_LINK_MAX - 1), "Link/Unlink Pairs");

	return &s;
}

/*
 * Signal Test
 *
 * This test repeatedly sends itself a SIGUSR2 and catches it.
 * The signal hander simply increments 'sigcount' after making sure
 * that the signal it received is SIGUSR2. (If the signal isn't
 * SIGUSR2, a message is printed and 'error' is set to 1 which
 * will terminate the test.
 */
static int sigcount;			/* count of signals */
static int error;			/* no errors now */
static void
sighandler(int sig)
{					/* trap handler */
	signal(SIGUSR2, sighandler);	/* install the signal hander */
	if (sig != SIGUSR2) {		/* if not our signal */
		fprintf(stderr, "sighandler(): received signal %d not SIGUSR2 (%d)\n",	/* error here */
			sig, SIGUSR2);
		error = 1;		/* tell caller to stop */
	}
	sigcount++;			/* bump the count and return */
}

static int
signal_test(char *argv,
	    Result * res)
{
	int
	  i32,				/* loop count */
	  status;			/* result of kill() */
	pid_t mypid;			/* our PID */

	/*
	 * Step 1: get argument
	 */
	if (sscanf(argv, "%d", &i32) < 1) {	/* get value form cmd line */
		fprintf(stderr, "signal_test(): needs 1 argument!\n");	/* error here */
		return (-1);		/* stop run */
	}
	/*
	 * Step 2: initialize variables
	 */
	error =				/* clear the error flag */
		sigcount = 0;		/* clear the count */
	mypid = getpid();		/* get our pid */
	signal(SIGUSR2, sighandler);	/* install the signal hander */
	/*
	 * Step 3: Loop enough times.
	 */
	while (sigcount < i32) {	/* do the loop */
		status = kill(mypid, SIGUSR2);	/* send the signal */
		if (status == -1) {	/* error? */
			perror("\nkill(signal_test)");	/* tell more info */
			fprintf(stderr, "signal_test(): unable to send kill(%d, SIGUSR2).\n", mypid);	/* if so talk to human */
			return -1;	/* die */
		}
		if (error == 1) {	/* handle sig hande error */
			fprintf(stderr, "signal_test(): signal handler discovered an error.\n");	/* talk to human (again) */
			return -1;	/* die */
		}
	}				/* loop */
	return 0;			/* return success */
}

/*
 * Jump Test:
 * 
 * This test exercises the setjmp() and longjmp() functions in the standard
 * library. Since these functions are used extensively in some programs (including
 * the most popular portable threads implementations), it is reasonable to talk about
 * their timings.
 *
 * This test repeatedly performs a setjmp() followed by a longjmp().
 */
static void
dummy_function(jmp_buf buf,
	       int count)
{
	if (count == 0)
		longjmp(buf, 1);
	dummy_function(buf, --count);	/* go for kernel trap, max out register windows */
}

static int
jmp_test(char *argv,
	 Result * res)
{
	static int			/* static so they'll survive */
	  i32,				/* number of iterations */
	  result,			/* result of setjmp */
	  i;				/* loop count */

	jmp_buf buf;			/* the context */
	/*
	 * Step 1: get argument
	 */
	if (sscanf(argv, "%d", &i32) < 1) {	/* get value form cmd line */
		fprintf(stderr, "jmp_test(): needs 1 argument!\n");	/* error here */
		return (-1);		/* die */
	}
	/*
	 * Step 2: do the loop
	 */
	for (i = 0; i < i32; i++) {	/* loop a bunch of times */
		result = setjmp(buf);	/* save the context */
		switch (result) {	/* look at the result */
		case 0:
			dummy_function(buf, 10);	/* different stack frame */
		case 1:
			break;		/* we must have just longjmp()ed so loop */
		default:
			return -1;	/* something must have happened so error */
		}			/* end of switch */
	}				/* end of loop */
	return 0;			/* return success */
}

static int
link_test(char *argv,
	  Result * res)
     /*
      * This test creates a file in the current directory and then 
      * adds 7 additional links to it. (POSIX spec says that LINK_MAX's 
      * smallest legal value is 8 so 1 + 7 == 8.) The links are then
      * removed and then reattached in a round-robin fashion. For some
      * vendors, this involves sync directory updates, others it doesn't.
      * Anyway, we've seen some POSIX wannabes claim to be POSIX compliant
      * with values of less than 8 for LINK_MAX so we just use 8 anyway.
      * If they aren't compliant, then they can't run this test anyway.
      * And they shouldn't be claiming POSIX compliance since it'll break
      * lots of applications.
      */
{
	int
	  i,				/* misc loop variable */
	  status,			/* OS function status return */
	  fd,				/* file discriptor */
	  n;				/* loop variable */

	static char
	  buffer0[1024],		/* filename buffers */
	 
		buffer1[1024],
		buffer2[1024],
		buffer3[1024],
		buffer4[1024],
		buffer5[1024],
		buffer6[1024],
		buffer7[1024],
		*fns[THE_LINK_MAX] =
		{ buffer0, buffer1, buffer2, buffer3, buffer4, buffer5,
	       buffer6, buffer7 };
	/*
	 * Step 1: create unique filenames
	 */
	for (i = 0; i < THE_LINK_MAX; i++) {	/* for filenames 0..7 */
		if (*argv)		/* if directory passed */
			sprintf(fns[i], "%s/linkXXXXXXXXXX", argv);	/* put value into string */
		else			/* no directory passed */
			sprintf(fns[i], "linkXXXXXXXXXX");	/* put value into string */
		aim_mktemp(fns[i]);	/* force it to be unique (use our function) */
	}
	/*
	 * Step 2: create the first file
	 */
	fd = creat(fns[0], CREAT_MODE);	/* create the file */
	if (fd < 0) {			/* check for errors */
		fprintf(stderr, "link_test(): unable to create file %s\n",
			fns[0]);
		perror("link_test(): ");
		return (-1);
	}
	/*
	 * Step 3: Create the links
	 */
	for (i = 1; i < THE_LINK_MAX; i++) {	/* create the links for 1..7 */
		status = link(fns[0], fns[i]);	/* link them */
		if (status != 0) {	/* error here? */
			if (errno == EMLINK)
				fprintf(stderr,
					"Link_test: POSIX Violation detected.\n");
			else
				fprintf(stderr,
					"link_test: Unable to create link to file %s\n",
					fns[i]);
			perror("link_test(): ");
			return (-1);
		}
		res->i++;
	}
	/*
	 * Step 4: the loop
	 */
	n = LINK_LOOP;			/* establish loop count */
	while (n--) {			/* while looping */
		for (i = 1; i < THE_LINK_MAX; i++) {	/* try each one in turn */
			status = unlink(fns[i]);	/* unlink it */
			if (status != 0) {	/* check for errors */
				fprintf(stderr, "link_test: Unable to unlink file %s\n", fns[i]);	/* talk to human */
				perror("link_test(): ");
				return (-1);
			}
			status = link(fns[0], fns[i]);	/* link them back */
			if (status != 0) {	/* error here? */
				if (errno == EMLINK)	/* Some non-UNIX POSIX claimants aren't compliant */
					fprintf(stderr,
						"Link_test: POSIX Violation detected.\n");
				else	/* else just an error here */
					fprintf(stderr,
						"link_test: Unable to create link to file %s\n",
						fns[i]);
				perror("link_test(): ");
				return (-1);	/* die anyway */
			}		/* end of error */
			res->i++;
		}			/* end of for */
	}				/* end of while */
	/*
	 * Step 5: clean up
	 */
	for (i = 0; i < THE_LINK_MAX; i++) {
		status = unlink(fns[i]);	/* unlink it */
		if (status != 0) {
			perror("link_test(): ");
			fprintf(stderr,
				"link_test: Unable to unlink file %s\n",
				fns[i]);
			return (-1);
		}
	}
	close(fd);			/* make file go away (its already unlinked) */
	/*
	 * Step 6: we're done
	 */
	return 0;
}

static int
page_test(char *argv,
	  Result * res)
{
	int
	  i,				/* loop variable */
	  j,				/* dirty page pointer */
	  n,				/* iteration count */
	  i32,				/* argument */
	  brk();			/* system call */

	void
	 *oldbrk,			/* old top of memory */
	 *newbrk,			/* new top of memory */
	 *sbrk();			/* system call */

	char *cp;			/* pointer to new memory region */

	/*
	 * Step 1: Get argument
	 */
	if (sscanf(argv, "%d", &i32) < 1) {
		fprintf(stderr, "page_test(): needs 1 argument!\n");
		return (-1);
	}
	/*
	 * Step 2: Get old memory top, move top higher
	 */
	oldbrk = sbrk(0);		/* get current break value */
	newbrk = sbrk(1024 * 1024);	/* move up 1 megabyte */
	if ((int)newbrk == -1) {
		perror("\npage_test");	/* tell more info */
		fprintf(stderr, "page_test: Unable to do initial sbrk.\n");
		return (-1);
	}
	/*
	 * Step 3: The loop
	 */
	n = i32;
	while (n--) {			/* while not done */
		newbrk = sbrk(-4096 * 16);	/* deallocate some space */
		for (i = 0; i < 16; i++) {	/* now get it back in pieces */
			newbrk = sbrk(4096);	/* Get pointer to new space */
			if ((int)newbrk == -1) {
				perror("\npage_test");	/* tell more info */
				fprintf(stderr,
					"page_test: Unable to do sbrk.\n");
				return (-1);
			}
			cp = (char *)newbrk;	/* prepare to dirty it so pages into memory */
			for (j = 0; j < 4096; j += 128) {	/* loop through new region */
				cp[j] = ' ';	/* dirty the page */
			}		/* end for */
			res->i++;
		}			/* end for */
	}				/* end while */
	/*
	 * Step 4: Clean up by moving break back to old value
	 */
	brk(oldbrk);
	return 0;
}

static int
brk_test(char *argv,
	 Result * res)
{
	int
	  i, n, i32, brk();
	void
	 *oldbrk, *newbrk, *sbrk();

	/*
	 * Step 1: get argument
	 */
	if (sscanf(argv, "%d", &i32) < 1) {
		fprintf(stderr, "brk_test(): needs 1 argument!\n");
		return (-1);
	}
	/*
	 * Step 2: get old memory top, move top higher
	 */
	oldbrk = sbrk(0);		/* get old break value */
	newbrk = sbrk(1024 * 1024);	/* move up 1 megabyte */
	if ((int)newbrk == -1) {
		perror("\nbrk_test");	/* tell more info */
		fprintf(stderr, "brk_test: Unable to do initial sbrk.\n");
		return (-1);
	}
	/*
	 * Step 3: The loop
	 */
	n = i32;
	while (n--) {			/* while not done */
		newbrk = sbrk(-4096 * 16);	/* deallocate some space */
		for (i = 0; i < 16; i++) {	/* allocate it back */
			newbrk = sbrk(4096);	/* 4k at a time (should be ~ 1 page) */
			if ((int)newbrk == -1) {
				perror("\nbrk_test");	/* tell more info */
				fprintf(stderr,
					"brk_test: Unable to do sbrk.\n");
				return (-1);
			}
			res->i++;
		}
	}
	/*
	 * Step 4: clean up
	 */
	brk(oldbrk);
	return 0;
}

static int
exec_test(char *argv,
	  Result * res)
{
	int
	  fval, status, n, i32;

	if (sscanf(argv, "%d", &i32) < 1) {
		fprintf(stderr, "exec_test(): needs 1 argument!\n");
		return (-1);
	}
	n = i32;
	while (n--) {
		fval = fork();		/* fork the task off */
		if (fval == 0) {	/* we're the child */
			status = execl("./true", "true", NULL);
			perror("\nexec_test");	/* tell more info */
			fprintf(stderr,
				"Cannot execute `./true' (status = %d)\n",
				status);
			exit(-1);	/* quit painlessly */
		} else {
			while (1) {	/* now wait for done */
				if (wait(&status) > 0) {	/* if one has completed OK */
					if ((status & 0377) == 0177)	/* child proc stopped */
						(void)fprintf(stderr,
							      "\nChild process stopped by signal #%d\n",
							      ((status >> 8) &
							       0377));
					else if ((status & 0377) != 0) {	/* child term by sig */
						if ((status & 0377) & 0200) {
							(void)fprintf(stderr,
								      "\ncore dumped\n");
							(void)fprintf(stderr,
								      "\nChild terminated by signal #%d\n",
								      (status &
								       0177));
						}
					} else {	/* child exit()ed */
						if (((status >> 8) & 0377))	/* isolate status */
							(void)fprintf(stderr,
								      "\nChild process called exit(), status = %d\n",
								      ((status
									>> 8) &
								       0377));
					}
				} else if (errno == ECHILD)
					break;
				else if (errno == EINTR)
					continue;
			}
		}
		res->i++;
	}
	return 0;
}

static int
fork_test(char *argv,
	  Result * res)
{
	int
	  fval, status, n, i32;

	if (sscanf(argv, "%d", &i32) < 1) {
		fprintf(stderr, "fork_test(): needs 1 argument!\n");
		return (-1);
	}
	n = i32;
	while (n--) {
		fval = fork();		/* fork the task off */
		if (fval == 0) {	/* we're the child */
			exit(0);	/* quit painlessly */
		} else {
			while (1) {	/* now wait for done */
				if (wait(&status) > 0) {	/* if one has completed OK */
					if ((status & 0377) == 0177)	/* child proc stopped */
						(void)fprintf(stderr,
							      "\nChild process stopped by signal #%d\n",
							      ((status >> 8) &
							       0377));
					else if ((status & 0377) != 0) {	/* child term by sig */
						if ((status & 0377) & 0200) {
							(void)fprintf(stderr,
								      "\ncore dumped\n");
							(void)fprintf(stderr,
								      "\nChild terminated by signal #%d\n",
								      (status &
								       0177));
						}
					} else {	/* child exit()ed */
						if (((status >> 8) & 0377))	/* isolate status */
							(void)fprintf(stderr,
								      "\nChild process called exit(), status = %d\n",
								      ((status
									>> 8) &
								       0377));
					}
				} else if (errno == ECHILD)
					break;
				else if (errno == EINTR)
					continue;
			}
		}
		res->i++;
	}
	return 0;
}

static int
creat_clo(char *argv,
	  Result * res)
{					/* does the creat/close loop */
	int
	  n,				/* loop counter */
	  j;				/* status value */
	char fn4[FILENAME_MAX];		/* file name buffer */

#ifdef COUNT
	int count = 0;
#endif

	n = 1000;			/* initialize loop count */
	j = 0;				/* make gcc happy */
	if (*argv)			/* if directory passed */
		sprintf(fn4, "%s/%s", argv, TMPFILE2);	/* append values */
	else				/* no directory passed */
		sprintf(fn4, "%s", TMPFILE2);	/* copy the filename over */
	aim_mktemp(fn4);		/* create unique string */
	while (n--) {			/* loop a set number of times */
		if ((j = creat(fn4, CREAT_MODE)) < 0) {	/* if can't creat */
			fprintf(stderr, "creat error\n");	/* error */
			perror("creat-clo(): ");
			return (-1);
		} /* end of error */
		else if (close(j) < 0) {	/* if can't close */
			fprintf(stderr, "close error\n");	/* error */
			perror("creat-clo(): ");
			return (-1);
		}			/* end of error */
		res->i++;
#ifdef COUNT
		count++;
#endif
	}				/* end of loop */
	unlink(fn4);			/* unlink the file */
#ifdef COUNT
	printf("creat_clo: %d\n", count);
#endif
	return (j);			/* return last file handle */
}					/* the end */
