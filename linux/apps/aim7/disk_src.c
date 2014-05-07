
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
#define _POSIX_SOURCE 1			/* turn on POSIX funct'ns  */

#include <stdio.h>			/* enable printf(), etc. */
#include <unistd.h>			/* for chdir(), etc. */
#include <stdlib.h>			/* for malloc(), etc. */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>			/* required for creat */
#include <signal.h>
#include "suite.h"			/* our goodies */

static int disk_src();

COUNT_START				/* declare counters */
	source_file * disk_src_c()
{
	static source_file s = { " @(#) disk_src.c:1.10 3/4/94 17:21:22",	/* SCCS info */
		__FILE__, __DATE__, __TIME__
	};

	register_test("disk_src", "DISKS", disk_src, 75, "Directory Searches");	/* 100 c and s files in fakeh/dirlist */
	return &s;
}

enum choices { STAT = 0, CREAT, MCHOICE };	/* encode choices here */

#define FAKEH "fakeh"
#define MFILES	75			/* number of entries */
#define MYBUF 160			/* MAX size of input line */
#define MSCR 5				/* number of scramble passes */

/*
 * dsearch exercises the directory search mechanism of unix systems.
 * it is called by the disk test program. dsearch assumes that it is 
 * invoked with its current directory is the parent directory
 * of the hand created directory that is distributed with the benchmark.
 * it assumes that in this directory is a file "dirlist" that provides a list
 * of file names under the current directory, along with a list of names to
 * search for.  some of these names are to be stat'ed while some are to be
 * creat'ed
 */

unsigned long mrand();
int get_list(FILE * file,
	     char *list[MCHOICE][MFILES]);
void scramble(char *list[],
	      int num);
void cl_list(char *list[MCHOICE][MFILES]);
void errdump(int line,
	     char *str);

int
dsearch(char *fakeh_dir)
{
	FILE *fp;			/* file containing filenames */

	int
	  fd,				/* file discriptor for creat, etc. */
	  index;			/* loop variable */

	struct stat stbuf;		/* stat buffer */

	char
	  cwd[256],			/* hold current working dir */
	  errbuf[80],			/* build error msgs in here */
	 *flist[MCHOICE][MFILES];	/* the list of target files */

	if (getcwd(cwd, 256) == NULL) {
		fprintf(stderr,
			"dsearch(): can't get current working directory\n");
		return (-1);
	}
	if (chdir(fakeh_dir) < 0) {	/* move to directory */
		perror("dsearch()");	/* if error, print it */
		errdump(__LINE__, "dsearch(): directory 'fakeh' is inaccessable\n");	/* and dump */
		return (-1);		/* return failure */
	}

	if ((fp = fopen("dirlist", "r")) == NULL) {	/* open list of filenames */
		errdump(__LINE__, "dsearch(): file 'dirlist' is inaccessable\n");	/* handle error */
		chdir(cwd);		/* move back up */
		return (-1);		/* return error */
	}
	/*
	 * end of error processing 
	 */
	if (get_list(fp, flist) < 0) {	/* load the list */
		errdump(__LINE__, "dsearch(): file 'dirlist' is corrupted\n");	/* handle errors */
		chdir(cwd);		/* go back up */
		cl_list(flist);		/* close list */
		return (-1);		/* return error */
	}
	fclose(fp);			/* close list of filenames */

	scramble(flist[STAT], MFILES);	/* scramble names */
	scramble(flist[CREAT], MFILES);	/* scramble names */

	for (index = 0; index < MFILES; index++) {	/* loop through files  */
		if (flist[STAT][index] != NULL) {	/* if not null */
			if (stat(flist[STAT][index], &stbuf) < 0) {	/* stat the file */
				perror("stat() in dsearch()");	/* handle errors */
				sprintf(errbuf, "dsearch(): can't stat '%s'\n",	/* create error message */
					flist[STAT][index]);
				errdump(__LINE__, errbuf);	/* print it */
				chdir(cwd);	/* return to proper dir */
				cl_list(flist);	/* clear list */
				return (-1);	/* return error */
			}		/* endo f error */
		}
		/*
		 * end of if not null 
		 */
		if (flist[CREAT][index] != NULL) {	/* if creating */
			if ((fd = creat(flist[CREAT][index], S_IRWXU | S_IRWXG | S_IRWXO)) < 0) {	/* try create */
				perror("creat() in dsearch()");	/* handle error */
				sprintf(errbuf, "dsearch():can't creat '%s'\n",	/* build error message */
					flist[CREAT][index]);
				errdump(__LINE__, errbuf);	/* print it */
				chdir(cwd);	/* change directories */
				cl_list(flist);	/* clear list */
				return (-1);	/* return error */
			}		/* end of error */
			close(fd);	/* close the file */
			if (unlink(flist[CREAT][index])) {	/* unlink it */
				perror("unlink() in dsearch()");	/* handle error */
				sprintf(errbuf, "dsearch():can't unlink '%s'\n",	/* build error message */
					flist[CREAT][index]);
				errdump(__LINE__, errbuf);	/* print it */
				chdir(cwd);	/* change directories */
				cl_list(flist);	/* clear list */
				return (-1);	/* return error */
			}		/* end of error */
		}
		/*
		 * end if creating 
		 */
		COUNT_BUMP;

	}				/* end of for */
	cl_list(flist);			/* clear list */
	chdir(cwd);			/* go back up */
	return (0);			/* return no error */
}

int
get_list(FILE * file,
	 char *list[MCHOICE][MFILES])
{
	char
	  buff[MYBUF],			/* holds 1 line of input */
	 *tmp;				/* holds malloc results */

	int
	  s_index, c_index, i;

	pid_t pid = getpid();		/* process ID, for unique file names */

	s_index = c_index = 0;		/* initialize indexes */

	for (i = 0; i < MFILES; i++)	/* initialize array */
		list[STAT][i] = list[CREAT][i] = NULL;	/* clear to empty */

	while (fgets(buff, MYBUF - 1, file) != NULL) {	/* get a line */
		if (buff[0] != 's' && buff[0] != 'c')	/* if it isn't legal, */
			continue;	/* ignore it */

		buff[strlen(buff) - 1] = '\0';	/* eliminate trailing new line */
		if ((tmp = malloc(strlen(buff) + 1 + 8)) == NULL) {	/* allocate space */
			cl_list(list);	/* handle bad allocate */
			return (-1);	/* return error */
		}
		/*
		 * end of malloc error chking 
		 */
		strcpy(tmp, buff + 2);	/* copy name into buffer */

		switch (buff[0]) {	/* decide on operation */
		case 's':		/* if stat */
			list[STAT][s_index++] = tmp;	/* put it into array */
			break;		/* and leave */

		case 'c':		/* if CREAT; Tin Le */
			sprintf(tmp, "%s%05d", (buff + 2), pid % 100000);	/* make unique name, last 4 digits of pid */
			list[CREAT][c_index++] = tmp;	/* save it off */
			break;		/* and leave */

		default:		/* this cannot be */
			errdump(__LINE__, "getlist(): Deadly error encountered\n");	/* print merror message */
			cl_list(list);	/* clear list */
			return (-1);	/* return error here */
		}			/* end of switch */
	}				/* end of loop */
	return (1);			/* return success */
}



void
scramble(char *list[],
	 int num)
{
	int
	  i,				/* loop variable */
	  scount,			/* scramble count */
	  rnum;				/* random index for scramble */

	char
	 *tmp;				/* intermediate location */

	for (scount = 0; scount < MSCR; scount++) {	/* for number of scrambles */
		for (i = 0; i < num; i++) {	/* go through list */
			rnum = mrand() % num;	/* get an index */
			tmp = list[i];	/* swap this one */
			list[i] = list[rnum];	/* with that one */
			list[rnum] = tmp;	/* and we're done */
		}			/* loop through each one */
	}				/* for each pass */
}


unsigned long
mrand()
{					/* return integer randome number */
	return ((unsigned long)aim_rand());	/* do it */
}

void
cl_list(char *list[MCHOICE][MFILES])
{
	int index;			/* loop variable */

	for (index = 0; index < MFILES; index++) {	/* loo through all entries */
		if (list[STAT][index] != NULL)	/* if string resides here */
			free(list[STAT][index]);	/* free it to heap */
		if (list[CREAT][index] != NULL)	/* if string here */
			free(list[CREAT][index]);	/* free it */
	}				/* end of loop */
}

void
errdump(int line,
	char *str)
{					/* print error message */
	fprintf(stderr,
		"Error in file %s (compiled at %s on %s) from line %d:\n\t%s",
		__FILE__, __TIME__, __DATE__, line, str);
}

static int
disk_src(char *argv,
	 Result * res)
{
	int i;
	char fakeh_dir[128];

	if (*argv)
		sprintf(fakeh_dir, "%s/%s", argv, FAKEH);
	else
		strcpy(fakeh_dir, FAKEH);

	i = dsearch(fakeh_dir);
	COUNT_END("disk_src");
	return (res->i = i);
}
