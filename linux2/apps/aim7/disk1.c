
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
#define _XOPEN_SOURCE 1			/* so O_SYNC can be used */
#define _INCLUDE_XOPEN_SOURCE 1		/* so O_SYNC can be used on HP */
#define _M_XOUT 1			/* so O_SYNC can be used on Acer/Altos */

#include <stdio.h>			/* enable scanf(), etc.  */
#include <stdlib.h>			/* enable rand(), etc. */
#include <unistd.h>			/* enable write(), etc. */
#include <sys/types.h>			/* required for creat() */
#include <sys/stat.h>			/* required for creat() */
#include <fcntl.h>			/* required for creat() */
#include <string.h>			/* for strchr() */

#include "suite.h"			/* our includes */

static int
  sync_disk_rw(),
  sync_disk_wrt(),
  sync_disk_cp(),
  sync_disk_update(),
  disk_rr(),
  disk_rw(),
  disk_rd(),
  disk_wrt(),
  disk_cp();

void aim_mktemp();

source_file *
disk1_c()
{
	static source_file s = { " @(#) disk1.c:1.20 2/10/94 13:51:47",	/* SCCS info */
		__FILE__, __DATE__, __TIME__
	};

	if (NBUFSIZE != 1024) {		/* enforce known block size */
		fprintf(stderr, "NBUFSIZE changed to %d\n", NBUFSIZE);
		exit(1);
	}

	register_test("disk_rr", "DISKS", disk_rr, WAITING_FOR_DISK_COUNT,
		      "Random Disk Reads (K)");
	register_test("disk_rw", "DISKS", disk_rw, WAITING_FOR_DISK_COUNT,
		      "Random Disk Writes (K)");
	register_test("disk_rd", "DISKS", disk_rd, WAITING_FOR_DISK_COUNT,
		      "Sequential Disk Reads (K)");
	register_test("disk_wrt", "DISKS", disk_wrt, WAITING_FOR_DISK_COUNT,
		      "Sequential Disk Writes (K)");
	register_test("disk_cp", "DISKS", disk_cp, WAITING_FOR_DISK_COUNT,
		      "Disk Copies (K)");

	register_test("sync_disk_rw", "DISKS", sync_disk_rw,
		      WAITING_FOR_DISK_COUNT, "Sync Random Disk Writes (K)");
	register_test("sync_disk_wrt", "DISKS", sync_disk_wrt,
		      WAITING_FOR_DISK_COUNT,
		      "Sync Sequential Disk Writes (K)");
	register_test("sync_disk_cp", "DISKS", sync_disk_cp,
		      WAITING_FOR_DISK_COUNT, "Sync Disk Copies (K)");
	register_test("sync_disk_update", "DISKS", sync_disk_update,
		      WAITING_FOR_DISK_COUNT, "Sync Disk Updates (K)");
	return &s;
}


static char fn1[STRLEN];
static char fn2[STRLEN];

static char nbuf[NBUFSIZE];		/* 1K blocks */

/* test file size is 2^10 */
#define SHIFT	10			/* Change with block size */

/*
 * "Semi"-Random disk read					 
 * TVL 11/28/89
 */
static int
disk_rr(char *argv,
	Result * res)
{
	int
	  i, fd, n;
	long
	  sk;

	sk = 0l;
	n = disk_iteration_count;	/* user specified size */
	if (*argv)
		sprintf(fn2, "%s/%s", argv, TMPFILE2);
	else
		sprintf(fn2, "%s", TMPFILE2);
	aim_mktemp(fn2);		/* generate new file name */

	if ((fd = creat(fn2, (S_IRWXU | S_IRWXG | S_IRWXO))) < 0) {
		fprintf(stderr, "disk_rr : cannot create %s\n", fn2);
		perror(__FILE__);
		return (-1);
	}
	/*
	 * We do this to "encourage" the system to read from disk
	 * instead of the buffer cache.
	 * 12/12/89 TVL
	 */
	while (n--) {
		if (write(fd, nbuf, sizeof nbuf) != sizeof nbuf) {
			perror("disk_rr()");
			fprintf(stderr, "disk_rr : cannot write %s\n", fn2);
			close(fd);
			unlink(fn2);
			return (-1);
		}
	}
	close(fd);
	if ((fd = open(fn2, O_RDONLY)) < 0) {
		fprintf(stderr, "disk_rr : cannot open %s\n", fn2);
		perror(__FILE__);
		return (-1);
	}

	unlink(fn2);

  /********** pseudo random read *************/
	for (i = 0; i < disk_iteration_count; i++) {
		/*
		 * get random block to read, making sure not to read past end of file 
		 */
		sk = aim_rand() % ((disk_iteration_count * (long)sizeof (nbuf)) >> SHIFT);	/* rand() % (filesize/blocksize) */
		/*
		 * sk specifies a specific block, multiply by blocksize to get offset in bytes 
		 */
		sk <<= SHIFT;
		if (lseek(fd, sk, 0) == -1) {
			perror("disk_rr()");
			fprintf(stderr, "disk_rr : can't lseek %s\n", fn2);

/*      unlink(fn2);   */
			close(fd);
			return (-1);
		}
		if ((n = read(fd, nbuf, sizeof nbuf)) != sizeof nbuf) {
			perror("disk_rr()");
			fprintf(stderr, "disk_rr : can't read %s\n", fn2);

/*      unlink(fn2);   */
			close(fd);
			return (-1);
		}
	}
	/*
	 * unlink(fn2);  
	 */
	close(fd);
	res->d = n;
	return (0);
}



/*
 * "Semi"-Random disk write
 */


static int
disk_rw(char *argv,
	Result * res)
{
	int
	  i, fd, n;

	long
	  sk;

	sk = 0l;
	n = disk_iteration_count;	/* user specified size */
	if (*argv)
		sprintf(fn2, "%s/%s", argv, TMPFILE2);
	else
		sprintf(fn2, "%s", TMPFILE2);
	aim_mktemp(fn2);

	if ((fd = creat(fn2, (S_IRWXU | S_IRWXG | S_IRWXO))) < 0) {
		fprintf(stderr, "disk_rw : cannot create %s\n", fn2);
		perror(__FILE__);
		return (-1);
	}

	while (n--) {
		if (write(fd, nbuf, sizeof nbuf) != sizeof nbuf) {
			perror("disk_rw()");
			fprintf(stderr, "disk_rw : cannot write %s\n", fn2);
			close(fd);
			unlink(fn2);
			return (-1);
		}
	}
	close(fd);
	if ((fd = open(fn2, O_WRONLY)) < 0) {
		fprintf(stderr, "disk_rw : cannot open %s\n", fn2);
		perror(__FILE__);
		return (-1);
	}

	unlink(fn2);

  /********** pseudo random write *************/

	for (i = 0; i < disk_iteration_count; i++) {
		/*
		 * get random block to read, making sure not to read past end of file 
		 */
		sk = aim_rand() % ((disk_iteration_count * (long)sizeof (nbuf)) >> SHIFT);	/* rand() % (filesize/blocksize) */
		/*
		 * sk specifies a specific block, multiply by blocksize to get offset in bytes 
		 */
		sk <<= SHIFT;
		if (lseek(fd, sk, 0) == -1) {
			perror("disk_rw()");
			fprintf(stderr, "disk_rw : can't lseek %s\n", fn2);

/*      unlink(fn2);   */
			close(fd);
			return (-1);
		}
		if ((n = write(fd, nbuf, sizeof nbuf)) != sizeof nbuf) {
			perror("disk_rw()");
			fprintf(stderr, "disk_rw : can't read %s\n", fn2);

/*      unlink(fn2);   */
			close(fd);
			return (-1);
		}
	}

/*      unlink(fn2);   */
	close(fd);
	res->d = n;
	return (0);
}

static int
sync_disk_rw(char *argv,
	     Result * res)
{
	int
	  i, fd, blocks, n;

	long
	  sk;

	sk = 0l;
	n = blocks = disk_iteration_count / 2;	/* divide by 2 so as not to pound disk */
	if (*argv)
		sprintf(fn2, "%s/%s", argv, TMPFILE2);
	else
		sprintf(fn2, "%s", TMPFILE2);
	aim_mktemp(fn2);

	if ((fd = open(fn2, (O_WRONLY | O_CREAT | O_TRUNC),	/* standard CREAT mode */
		       (S_IRWXU | S_IRWXG | S_IRWXO))) < 0) {	/* standard permissions */
		fprintf(stderr, "sync_disk_rw : cannot create %s\n", fn2);
		perror(__FILE__);
		return (-1);
	}

	while (n--) {
		if (write(fd, nbuf, sizeof nbuf) != sizeof nbuf) {
			perror("disk_rw()");
			fprintf(stderr, "sync_disk_rw : cannot write %s\n",
				fn2);
			close(fd);
			unlink(fn2);
			return (-1);
		}
	}
	close(fd);
	if ((fd = open(fn2, O_WRONLY | O_SYNC)) < 0) {	/* open it again synchronously */
		fprintf(stderr, "sync_disk_rw : cannot open %s\n", fn2);
		perror(__FILE__);
		return (-1);
	}

/*  unlink(fn2); *//*
 * unlink moved after write 10/17/95  
 */

  /********** pseudo random write *************/

	for (i = 0; i < blocks; i++) {	/* get random block to read, 
					 * making sure not to read past end of file */
		sk = aim_rand() % ((blocks * (long)sizeof (nbuf)) >> SHIFT);	/* rand() % (filesize/blocksize) */

		sk <<= SHIFT;		/* sk specifies a specific block, 
					 * multiply by blocksize to get offset in bytes */
		if (lseek(fd, sk, 0) == -1) {
			perror("sync_disk_rw()");
			fprintf(stderr, "sync_disk_rw : can't lseek %s\n",
				fn2);
			unlink(fn2);
			close(fd);
			return (-1);
		}
		if ((n = write(fd, nbuf, sizeof nbuf)) != sizeof nbuf) {
			perror("sync_disk_rw()");
			fprintf(stderr,
				"sync_disk_rw : can't write %d bytes to %s\n",
				sizeof nbuf, fn2);
			unlink(fn2);
			close(fd);
			return (-1);
		}
	}
	unlink(fn2);
	close(fd);
	res->d = n;
	return (0);
}

static int
disk_rd(char *argv,
	Result * res)
{
	int
	  i, fd;

	if (*argv)
		sprintf(fn1, "%s/%s", argv, TMPFILE1);
	else
		sprintf(fn1, "%s", TMPFILE1);
	fd = open(fn1, O_RDONLY);
	if (fd < 0) {			/*  */
		fprintf(stderr, "disk_rd : cannot open %s\n", fn1);
		perror(__FILE__);
		return (-1);
	}
	/*
	 * forward sequential read only 
	 */
	if (lseek(fd, 0L, 0) < 0) {
		fprintf(stderr, "disk_rd : can't lseek %s\n", fn1);
		perror(__FILE__);
		close(fd);
		return (-1);
	}
	for (i = 0; i < disk_iteration_count; i++) {
		if (read(fd, nbuf, sizeof nbuf) != sizeof nbuf) {
			fprintf(stderr, "disk_rd : can't read %s\n", fn1);
			perror(__FILE__);
			close(fd);
			return (-1);
		}
	}
	close(fd);
	res->d = i;
	return (0);
}

static int
disk_cp(char *argv,
	Result * res)
{
	int
	  status,			/* result of last system call */
	  n, fd, fd2;

	n = disk_iteration_count;	/* user specified size */
	if (*argv) {			/* are we passing a directory? */
		sprintf(fn1, "%s/%s", argv, TMPFILE1);	/* if so, build source file name */
		sprintf(fn2, "%s/%s", argv, TMPFILE2);	/* and destination file name */
	} else {			/* else build names in this directory */
		sprintf(fn1, "%s", TMPFILE1);	/* source file name */
		sprintf(fn2, "%s", TMPFILE2);	/* desination file nam */
	}
	aim_mktemp(fn2);		/* convert into unique temporary name */
	fd = open(fn1, O_RDONLY);	/* open the file */
	if (fd < 0) {			/* open source file */
		fprintf(stderr, "disk_cp (1): cannot open %s\n", fn1);	/* handle error */
		perror(__FILE__);	/* print error */
		return (-1);		/* return error */
	}
	fd2 = creat(fn2, (S_IRWXU | S_IRWXG | S_IRWXO));	/* create the file */
	if (fd2 < 0) {			/* create output file */
		fprintf(stderr, "disk_cp (2): cannot create %s\n", fn2);	/* talk to human on error */
		perror(__FILE__);
		close(fd);		/* close source file */
		return (-1);		/* return error */
	}
	unlink(fn2);			/* make it anonymous (and work NFS harder) */
	status = lseek(fd, 0L, SEEK_SET);	/* move pointer to offset 0 (rewind) */
	if (status < 0) {		/* handle error case */
		fprintf(stderr, "disk_cp (3): cannot lseek %s\n", fn1);	/* talk to human */
		perror(__FILE__);
		/*
		 * unlink(fn2);
 *//*
 * make it anonymous (and work NFS harder) 
 */
		close(fd);		/* close source file */
		close(fd2);		/* close this file */
		return (-1);		/* return error */
	}
	while (n--) {			/* while not done */
		status = read(fd, nbuf, sizeof nbuf);	/* do the read */
		if (status != sizeof nbuf) {	/* return the status */
			fprintf(stderr,
				"disk_cp (4): cannot read %s %d (status = %d)\n",
				fn1, fd, status);
			perror(__FILE__);
			/*
			 * unlink(fn2); 
 *//*
 * make it anonymous (and work NFS harder) 
 */
			close(fd);
			close(fd2);
			return (-1);
		}
		status = write(fd2, nbuf, sizeof nbuf);	/* do the write */
		if (status != sizeof nbuf) {	/* check for error */
			fprintf(stderr, "disk_cp (5): cannot write %s\n", fn2);
			perror(__FILE__);
			/*
			 * unlink(fn2); 
 *//*
 * make it anonymous (and work NFS harder) 
 */
			close(fd);
			close(fd2);
			return (-1);
		}
	}
	/*
	 * unlink(fn2); 
 *//*
 * make it anonymous (and work NFS harder) 
 */
	close(fd);			/* close input file */
	close(fd2);			/* close (and delete) output file */
	res->d = disk_iteration_count;	/* return number */
	return (0);			/* show success */
}

static int
sync_disk_cp(char *argv,
	     Result * res)
{
	int
	  status,			/* result of last system call */
	  n, blocks, fd, fd2;

	n = blocks = disk_iteration_count / 2;	/* divide by 2 so as not to pound disk */
	if (*argv) {			/* are we passing a directory? */
		sprintf(fn1, "%s/%s", argv, TMPFILE1);	/* if so, build source file name */
		sprintf(fn2, "%s/%s", argv, TMPFILE2);	/* and destination file name */
	} else {			/* else build names in this directory */
		sprintf(fn1, "%s", TMPFILE1);	/* source file name */
		sprintf(fn2, "%s", TMPFILE2);	/* desination file nam */
	}
	aim_mktemp(fn2);		/* convert into unique temporary name */
	fd = open(fn1, O_RDONLY);	/* open the file */
	if (fd < 0) {			/* open source file */
		fprintf(stderr, "sync_disk_cp (1): cannot open %s\n", fn1);	/* handle error */
		perror(__FILE__);	/* print error */
		return (-1);		/* return error */
	}
	fd2 = open(fn2, (O_SYNC | O_WRONLY | O_CREAT | O_TRUNC), (S_IRWXU | S_IRWXG | S_IRWXO));	/* create the file */
	if (fd2 < 0) {			/* create output file */
		fprintf(stderr, "sync_disk_cp (2): cannot create %s\n", fn2);	/* talk to human on error */
		perror(__FILE__);
		close(fd);		/* close source file */
		return (-1);		/* return error */
	}

/*  unlink(fn2);	*//*
 * make it anonymous (and work NFS harder) 
 */
	status = lseek(fd, 0L, SEEK_SET);	/* move pointer to offset 0 (rewind) */
	if (status < 0) {		/* handle error case */
		fprintf(stderr, "sync_disk_cp (3): cannot lseek %s\n", fn1);	/* talk to human */
		perror(__FILE__);
		close(fd);		/* close source file */
		close(fd2);		/* close this file */
		return (-1);		/* return error */
	}
	while (n--) {			/* while not done */
		status = read(fd, nbuf, sizeof nbuf);	/* do the read */
		if (status != sizeof nbuf) {	/* return the status */
			fprintf(stderr,
				"sync_disk_cp (4): cannot read %s %d (status = %d)\n",
				fn1, fd, status);
			perror(__FILE__);
			unlink(fn2);	/* make it anonymous (and work NFS harder) */
			close(fd);
			close(fd2);
			return (-1);
		}
		status = write(fd2, nbuf, sizeof nbuf);	/* do the write (SYNC) */
		if (status != sizeof nbuf) {	/* check for error */
			fprintf(stderr, "sync_disk_cp (5): cannot write %s\n",
				fn2);
			perror(__FILE__);
			unlink(fn2);	/* make it anonymous (and work NFS harder) */
			close(fd);
			close(fd2);
			return (-1);
		}
	}
	unlink(fn2);			/* make it anonymous (and work NFS harder) */
	close(fd);			/* close input file */
	close(fd2);			/* close (and delete) output file */
	res->d = blocks;		/* return number */
	return (0);			/* show success */
}

static int
disk_wrt(char *argv,
	 Result * res)
{
	int
	  n, fd, i;

	n = disk_iteration_count;	/* user specified size */
	i = 0;
	if (*argv)
		sprintf(fn2, "%s/%s", argv, TMPFILE2);
	else
		sprintf(fn2, "%s", TMPFILE2);
	aim_mktemp(fn2);
	fd = creat(fn2, (S_IRWXU | S_IRWXG | S_IRWXO));
	if (fd < 0) {
		fprintf(stderr, "disk_wrt : cannot create %s\n", fn2);
		perror(__FILE__);
		return (-1);
	}

	unlink(fn2);

	while (n--) {
		if ((i = write(fd, nbuf, sizeof nbuf)) != sizeof nbuf) {
			fprintf(stderr, "disk_wrt : cannot write %s\n", fn2);
			perror(__FILE__);

/*  unlink(fn2); *//*
 * unlink moved after write 10/17/95  
 */
			unlink(fn2);
			close(fd);
			return (-1);
		}
	}

/*  unlink(fn2); *//*
 * unlink moved after write 10/17/95  
 */
	close(fd);
	res->d = disk_iteration_count;
	return (0);
}

static int
sync_disk_wrt(char *argv,
	      Result * res)
{
	int
	  n, blocks, fd, i;

	n = blocks = disk_iteration_count / 2;	/* divide by 2 so as not to pound disk */
	i = 0;
	if (*argv)
		sprintf(fn2, "%s/%s", argv, TMPFILE2);
	else
		sprintf(fn2, "%s", TMPFILE2);
	aim_mktemp(fn2);
	fd = open(fn2, (O_SYNC | O_WRONLY | O_CREAT | O_TRUNC), (S_IRWXU | S_IRWXG | S_IRWXO));	/* sync creat */
	if (fd < 0) {
		fprintf(stderr, "sync_disk_wrt : cannot create %s\n", fn2);
		perror(__FILE__);
		return (-1);
	}

/*  unlink(fn2); *//*
 * unlink moved after write 10/17/95  
 */

	while (n--) {
		if ((i = write(fd, nbuf, sizeof nbuf)) != sizeof nbuf) {
			fprintf(stderr, "sync_disk_wrt : cannot write %s\n",
				fn2);
			perror(__FILE__);
			unlink(fn2);
			close(fd);
			return (-1);
		}
	}
	unlink(fn2);
	close(fd);
	res->d = blocks;
	return (0);
}

static int
sync_disk_update(char *argv,
		 Result * res)
{
	int
	  i, fd, blocks, loop, n;

	long
	  sk;

	sk = 0l;
	n = blocks = disk_iteration_count;
	if (*argv)
		sprintf(fn2, "%s/%s", argv, TMPFILE2);
	else
		sprintf(fn2, "%s", TMPFILE2);
	aim_mktemp(fn2);

	/*
	 * create a "database" file 
	 */
	if ((fd = open(fn2, (O_WRONLY | O_CREAT | O_TRUNC),	/* standard CREAT mode */
		       (S_IRWXU | S_IRWXG | S_IRWXO))) < 0) {	/* standard permissions */
		fprintf(stderr, "sync_disk_update : cannot create %s\n", fn2);
		perror(__FILE__);
		return (-1);
	}

	while (n--) {
		if (write(fd, nbuf, sizeof nbuf) != sizeof nbuf) {
			perror("disk_rw()");
			fprintf(stderr, "sync_disk_update : cannot write %s\n",
				fn2);
			close(fd);
			unlink(fn2);
			return (-1);
		}
	}
	close(fd);
	if ((fd = open(fn2, O_RDWR | O_SYNC)) < 0) {	/* open it again synchronously */
		fprintf(stderr, "sync_disk_update : cannot open %s\n", fn2);
		perror(__FILE__);
		return (-1);
	}

/*  unlink(fn2); *//*
 * unlink moved after update 10/17/95  
 */

	/*
	 * pseudo random read then update 
	 */
	loop = blocks / 2;		/* only touch 1/2 the blocks */
	for (i = 0; i < loop; i++) {	/* get random block to read, 
					 * making sure not to read past end of file */
		sk = aim_rand() % ((blocks * (long)sizeof (nbuf)) >> SHIFT);	/* rand() % (filesize/blocksize) */

		sk <<= SHIFT;		/* sk specifies a specific block, 
					 * multiply by blocksize to get offset in bytes */
		if (lseek(fd, sk, 0) == -1) {	/* look something up */
			perror("sync_disk_update()");
			fprintf(stderr, "sync_disk_update : can't lseek %s\n",
				fn2);
			unlink(fn2);
			close(fd);
			return (-1);
		}
		if ((n = read(fd, nbuf, sizeof nbuf)) != sizeof nbuf) {	/* read it in */
			perror("sync_disk_update()");
			fprintf(stderr,
				"sync_disk_update : can't read %d bytes to %s\n",
				sizeof nbuf, fn2);
			unlink(fn2);
			close(fd);
			return (-1);
		}
		if ((n = write(fd, nbuf, sizeof nbuf)) != sizeof nbuf) {	/* update it */
			perror("sync_disk_update()");
			fprintf(stderr,
				"sync_disk_update : can't write %d bytes to %s\n",
				sizeof nbuf, fn2);
			unlink(fn2);
			close(fd);
			return (-1);
		}
	}
	unlink(fn2);
	close(fd);
	res->d = n;
	return (0);
}

/* 
 * replaces the contents of the string pointed to by template with a unique file name.
 * The string in template should look like a file name with six trailing Xs.
 * aim_mktemp() will replace the Xs with a character string that can be used to create a
 * unique file name.
 * aim_mktemp() is a substitute for unix "mktemp()", which is not POSIX compliant.
 * tmpnam() does not allow a directory to be specified.
 * tempnam() is not POSIX compliant.
 * changed from 3 pid char to 5 pid char  10/17/95  1.1.
 */
void
aim_mktemp(char *template)
{
	static int counter = -1;	/* used to fill in template */
	static int pid_end;		/* holds last 5 digits of pid, constant for this process */
	char *Xs;			/* points to string of Xs in template */

	if ((template == NULL) || (*template == '\0')) {	/* make sure caller passes parameter */
		fprintf(stderr, "aim_mktemp : template parameter empty \n");
		return;
	}
	Xs = template + (strlen(template) - sizeof ("XXXXXXXXXX")) + 1;	/* find the X's */
	if (strcmp(Xs, "XXXXXXXXXX") != 0) {	/* bad parameter */
		fprintf(stderr,
			"aim_mktemp : template parameter needs 10 Xs \n");
		return;
	}
	if (counter++ == -1) {		/* initialize counter and pid */
		pid_end = getpid() % 100000;	/* use uniqueness of pid, only need 5 digits */
	} else if (counter == 100000)	/* reset, only need 5 digits */
		counter = 0;

	sprintf(Xs, "%05d%05d", pid_end, counter);	/* write over XXXXXXXXXX, zero pad counter */
}
