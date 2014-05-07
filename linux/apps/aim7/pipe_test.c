
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
#define _INCLUDE_HPUX_SOURCE		/* does not compile & link on HP w/o this */
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#ifdef BSDMEMMAP
#include <sys/mman.h>
#else
#include <sys/sem.h>
#endif
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <memory.h>

#include "suite.h"

#define SHMKEY ((key_t)getpid())
#define SEMKEY ((key_t)getpid())

#define MAXBLK (2048)
#define dump_socket(a,b)

#define FN1 "AIM_file.XXXXXXXXXX"	/*  changed from 6X to 10X  10/17/95  1.1  */
#define SHMFILE "AIM_SHM.XXXXXXXXXX"	/* file for shared memory for BSD */

#define FATAL(a) {fprintf(stderr,"\nFatal error %d at line %d of file %s: %s -- ",errno,__LINE__,__FILE__,a); perror(""); return(-1); }
#define FATAL_SHARED_MEM(a) {fprintf(stderr,"\nFatal error %d at line %d of file %s: %s -- ",errno,__LINE__,__FILE__,a); perror(""); \
                             clear_ipc();	    /* free the semaphore */         \
			     return(-1); }

/*
 * Forward Declarations
 */

static int read_write_close(int,
			    char *title,
			    int rd_fd,
			    int wr_fd);	/* the actual test */

static int count = 0;
static int sizes[] = {
	1, 3, 5, 7, 16, 32, 64, 512, 1024, 2048,	/* misc. sizes */
	1, 3, 5, 7, 16, 32, 64, 512, 1024, 2048,
	32, 32, 32, 32, 32, 32,		/* x windows mostly... */
	512, 512, 512, 512, 512,	/* DBMS's mostly */
};

static int sem = -1,			/* semaphore id */
  shmid = -1;				/* shared memory id */

static char
 *bufptr = NULL;			/* pointer to message buffer in shared memory */

double
  xy[100][2];				/* holds xy pairs */

int
  flag = 0;				/* completion flag */

struct sockaddr_in
  rd_in,				/* read socket */
  wr_in;				/* write socket */

struct hostent
 *hp;

char
  myname[1024];				/* hostname placed here */

COUNT_START FILE * plot;		/* plot output goes here */

union semun {				/* semaphore union */
	int val;
	struct semid_ds *buf;
	ushort *array;
} semarg;				/* argument to semctl */

extern void aim_mktemp();

static int shared_memory();
static int tcp_test();
static int udp_test();
static int stream_pipe();
static int dgram_pipe();
static int pipe_cpy();

source_file *
pipe_test_c()
{
	static source_file s = { " @(#) pipe_test.c:1.15 3/4/94 17:23:30",	/* SCCS info */
		__FILE__, __DATE__, __TIME__
	};
	int status;
	extern int gethostname();


	/*
	 * Do initialization
	 */
#if 0
	status = gethostname(myname, sizeof (myname));	/* get machine name */
	if (status < 0) {		/* handle errors */
		fprintf(stderr, "pipe_test_c(): couldn't gethostname()\n");
		exit(-1);
	}
	hp = gethostbyname(myname);	/* what is my address? */
	if (hp == NULL) {		/* handle errors */
		fprintf(stderr,
			"pipe_test_c(): couldn't gethostbyname() using <%s>\n",
			myname);
		exit(-1);
	}
#endif
	/*
	 * register the tests
	 */
	register_test("shared_memory", "DISKS", shared_memory, 100, "Shared Memory Operations");	/* CAUTION: update '100' in both places in shared_memory() if change here */
	register_test("tcp_test", "90", tcp_test, 90, "TCP/IP Messages");
	register_test("udp_test", "100", udp_test, 100, "UDP/IP DataGrams");
#ifdef NO_SOCKETPAIR
	register_test("stream_pipe", "100", tcp_test, 100,
		      "Stream Pipe Messages");
	register_test("dgram_pipe", "100", udp_test, 100,
		      "DataGram Pipe Messages");
#else
	register_test("stream_pipe", "100", stream_pipe, 100,
		      "Stream Pipe Messages");
	register_test("dgram_pipe", "100", dgram_pipe, 100,
		      "DataGram Pipe Messages");
#endif
	register_test("pipe_cpy", "100", pipe_cpy, 100, "Pipe Messages");
	/*
	 * return pointer to sccs info
	 */
	return &s;
}

static int
tcp_test(char *argv,
	 Result * res)
{
	int status, wr, rd, length, result, new;
	int i64;

	/*
	 * Get arguments
	 */
	if (sscanf(argv, "%d", &i64) < 1) {
		fprintf(stderr, "tcp_test(): needs 1 argument!\n");
		return -1;
	}
	/*
	 * Step 1: create write socket and everything
	 */
	status = wr = socket(AF_INET, SOCK_STREAM, 0);	/* create write socket */
	if (status < 0)
		FATAL("Creating write stream socket");	/* error test */
	/*
	 * Step 2: initialize socket structure and then bind it
	 */
	memset(&wr_in, 0, sizeof (wr_in));	/* clear it to zeros */
	wr_in.sin_family = AF_INET;	/* set family of socket */
	memcpy((void *)&wr_in.sin_addr.s_addr, (void *)hp->h_addr, hp->h_length);	/* ignore addresses */
	wr_in.sin_port = 0;		/* set write port (make kernel choose) */
	status = bind(wr, (struct sockaddr *)&wr_in, sizeof (wr_in));	/* do the bind */
	if (status < 0)
		FATAL("bind on write");	/* error test */
	/*
	 * Step 3: update info
	 */
	length = sizeof (wr_in);	/* get length */
	status = getsockname(wr, (struct sockaddr *)&wr_in, &length);	/* get socket info */
	if (status < 0)
		FATAL("getsockname()");	/* if error */
	/*
	 * tell it not to delay sending data
	 */
#ifdef TCP_NODELAY
	result = 1;			/* enable the option */
	status = setsockopt(wr, IPPROTO_TCP, TCP_NODELAY, (char *)&result, sizeof (result));	/* set it */
	if (status < 0)
		FATAL("setsockopt()");	/* die here if error */
	length = sizeof (result);	/* establish size */
	result = 0;			/* clear the result */
	status = getsockopt(wr, IPPROTO_TCP, TCP_NODELAY, (char *)&result, &length);	/* get it */
	if (status < 0)
		FATAL("getsockopt()");	/* die here if error */
#endif
	/*
	 * Step 4: Allow connections
	 */
	listen(wr, 5);			/* listen for connections */
	/*
	 * Step 5: create read socket
	 */
	status = rd = socket(AF_INET, SOCK_STREAM, 0);	/* create read socket */
	if (status < 0)
		FATAL("Creating read stream socket");	/* error check */
	/*
	 * Step 6: connect to write socket
	 */
	dump_socket("Write just before connect()", wr_in);
	status = connect(rd, (struct sockaddr *)&wr_in, sizeof (wr_in));	/* do the connect */
	if (status < 0)
		FATAL("Connect failure");	/* error check */
	/*
	 * Step 7: allow connections
	 */
	/*
	 * new = status = accept(wr, 0, 0); 
 *//*
 * do the accept 
 */
	/*
	 * if (status == -1) FATAL("accept"); 
 *//*
 * error check 
 */
      retry:				/* add retry 10/17/95 1.1  */
	new = status = accept(wr, 0, 0);	/* do the accept */
	if (status == -1 && errno == EINTR)
		goto retry;		/* error check */
	/*
	 * Step 8: update information
	 */
	length = sizeof (wr_in);	/* get length */
	status = getsockname(rd, (struct sockaddr *)&rd_in, &length);	/* get socket info */
	if (status < 0)
		FATAL("getsockname()");	/* check for error */
	/*
	 * Step 9: go for it
	 */
	status = read_write_close(i64, "TCP/IP", rd, new);	/* do the test */
	if (status < 0)
		FATAL("read_write_close()");	/* check for error */
	COUNT_END("tcp_test");
	close(wr);			/*  add 10/17/95  1.1  */
	return 0;
}

static int
udp_test(char *argv,
	 Result * res)
{
	int status, wr, rd, length;
	int i64;

	/*
	 * Get arguments
	 */
	if (sscanf(argv, "%d", &i64) < 1) {
		fprintf(stderr, "udp_test(): needs 1 argument!\n");
		return -1;
	}
	/*
	 * Step 1: create write socket and everything
	 */
	status = wr = socket(AF_INET, SOCK_DGRAM, 0);	/* create write socket */
	if (status < 0)
		FATAL("Creating write datagram socket");	/* error test */
	/*
	 * Step 2: initialize socket structure and then bind it
	 */
	memset(&wr_in, 0, sizeof (wr_in));	/* clear it to zeros */
	wr_in.sin_family = AF_INET;	/* set family of socket */
	memcpy((void *)&wr_in.sin_addr.s_addr, (void *)hp->h_addr, hp->h_length);	/* ignore addresses */
	wr_in.sin_port = 0;		/* set write port (make kernel choose) */
	status = bind(wr, (struct sockaddr *)&wr_in, sizeof (wr_in));	/* do the bind */
	if (status < 0)
		FATAL("bind on write");	/* error test */
	/*
	 * Step 3: update info
	 */
	length = sizeof (wr_in);	/* get length */
	status = getsockname(wr, (struct sockaddr *)&wr_in, &length);	/* get socket info */
	if (status < 0)
		FATAL("getsockname()");	/* if error */
	/*
	 * Step 4: create read socket
	 */
	status = rd = socket(AF_INET, SOCK_DGRAM, 0);	/* create read socket */
	if (status < 0)
		FATAL("Creating read datagram socket");	/* error check */
	/*
	 * Step 5: initialize socket structure and bind it
	 */
	memset(&rd_in, 0, sizeof (rd_in));	/* clear it to zeros */
	rd_in.sin_family = AF_INET;	/* set family of socket */
	memcpy((void *)&rd_in.sin_addr.s_addr, (void *)hp->h_addr, hp->h_length);	/* ignore addresses */
	rd_in.sin_port = 0;		/* set write port (make kernel choose) */
	status = bind(rd, (struct sockaddr *)&rd_in, sizeof (rd_in));	/* do the bind */
	if (status < 0)
		FATAL("bind on write");	/* error test */
	/*
	 * Step 6: update info 
	 */
	length = sizeof (rd_in);	/* get length */
	status = getsockname(rd, (struct sockaddr *)&rd_in, &length);	/* get socket info */
	if (status < 0)
		FATAL("getsockname()");	/* if error */
	/*
	 * Step 7: Connect ports
	 */
	status = connect(wr, (struct sockaddr *)&rd_in, sizeof (rd_in));	/* writes go to the read socket */
	if (status < 0)
		FATAL("connect()");	/* if error */
	/*
	 * Step 8: go for it
	 */
	status = read_write_close(i64, "UDP/IP", rd, wr);	/* do the test */
	if (status < 0)
		FATAL("read_write_close()");	/* check for error */
	COUNT_END("udp_test");
	return 0;
}

  /**********************************************************
   * Stream Pipes
   **********************************************************/
static int
stream_pipe(char *argv,
	    Result * res)
{
	int status, pipefd[2];
	int i64;

	/*
	 * Get arguments
	 */
	if (sscanf(argv, "%d", &i64) < 1) {
		fprintf(stderr, "stream_pipe(): needs 1 argument!\n");
		return -1;
	}
#ifndef NO_SOCKETPAIR
	status = socketpair(AF_UNIX, SOCK_STREAM, 0, pipefd);	/* create stream sockets */
#endif
	if (status < 0)
		FATAL("Cannot create stream pipes");	/* if error, stop */
	status = read_write_close(i64, "Stream Pipe", pipefd[0], pipefd[1]);	/* do the test */
	if (status < 0)
		FATAL("read_write_close()");	/* check for error */
	COUNT_END("stream_pipe");
	return 0;
}

  /**********************************************************
   * Datagram Pipes
   **********************************************************/
static int
dgram_pipe(char *argv,
	   Result * res)
{
	int status, pipefd[2];
	int i64;

	/*
	 * Get arguments
	 */
	if (sscanf(argv, "%d", &i64) < 1) {
		fprintf(stderr, "dgram_pipe(): needs 1 argument!\n");
		return -1;
	}
#ifndef NO_SOCKETPAIR
	status = socketpair(AF_UNIX, SOCK_DGRAM, 0, pipefd);	/* create datagram sockets */
#endif
	if (status < 0)
		FATAL("Cannot create datagram pipes");	/* if error, stop */
	status = read_write_close(i64, "Datagram Pipe", pipefd[0], pipefd[1]);	/* do the test */
	if (status < 0)
		FATAL("read_write_close()");	/* check for error */
	COUNT_END("dgram_pipe");
	return 0;
}

  /**********************************************************
   * Pipes
   **********************************************************/
static int
pipe_cpy(char *argv,
	 Result * res)
{
	int status, pipefd[2];
	int i64;

	/*
	 * Get arguments
	 */
	if (sscanf(argv, "%d", &i64) < 1) {
		fprintf(stderr, "pipe_cpy(): needs 1 argument!\n");
		return -1;
	}
	status = pipe(pipefd);		/* create pipe */
	if (status < 0)
		FATAL("Cannot create pipes");	/* if error, stop */
	status = read_write_close(i64, "Pipe", pipefd[0], pipefd[1]);	/* do the test */
	if (status < 0)
		FATAL("read_write_close()");	/* check for error */
	COUNT_END("pipe_cpy");
	return 0;
}
static int
readn(int fd,
      char *buf,
      int size)
{
	int
	  count, total, result;

	count = size + 2;		/* maximum iteration count */
	total = size;			/* initial amount to read */
	while (total > 0) {		/* while not done */
		errno = 0;		/* clear errors */
		result = read(fd, buf, total);	/* read some */
		if (result < 0) {
			if (errno == EINTR)
				continue;	/* try again if interrupted */
			return result;	/* return errors here */
		}
		total -= result;	/* else reduce total */
		buf += result;		/* update pointer */
		if (--count <= 0) {
			fprintf(stderr,
				"\nMaximum iterations exceeded in readn(%d, %#x, %d)",
				fd, (unsigned)buf, size);
			return (-1);
		}
	}				/* and loop */
	return (size - total);		/* calculate # bytes read */
}

static int
writen(int fd,
       char *buf,
       int size)
{
	int
	  count, total, result;

	count = size + 2;		/* initialize max count */
	total = size;			/* total */
	while (total > 0) {		/* while not done */
		errno = 0;		/* clear error number */
		result = write(fd, buf, total);	/* write some */
		if (result < 0) {	/* handle unusual case */
			if (errno == EINTR)
				continue;	/* if interrupted, loop */
			return result;	/* return errors here */
		}
		total -= result;	/* else reduce total */
		buf += result;		/* update pointer */
		if (--count <= 0) {	/* handle too many loops */
			fprintf(stderr,
				"\nMaximum iterations exceeded in writen(%d, %#x, %d)",
				fd, (unsigned)buf, size);
			return (-1);
		}
	}				/* and loop */
	return (size - total);		/* calculate # bytes read */
}

static int
read_write_close(int loops,
		 char *title,
		 int rd_fd,
		 int wr_fd)
{					/* the actual test */
	char buffer[MAXBLK];		/* input and output buffer */
	int
	  size,				/* transaction size */
	  result;			/* status result */

	COUNT_ZERO;			/* clear the count */
	while (--loops > 0) {
		size = sizes[count++ % Members(sizes)];	/* get next size */
		result = writen(wr_fd, buffer, size);	/* write this much */
		if (result != size) {
			fprintf(stderr, "Write returned %d -- ", result);
			perror("pipe write");
			return (-1);
		}
		result = readn(rd_fd, buffer, size);	/* read this much */
		if (result != size) {
			fprintf(stderr, "Read returned %d -- ", result);
			perror("pipe read");
			return (-1);
		}
		COUNT_BUMP;
	}
	close(rd_fd);
	close(wr_fd);
	return (0);
}

#ifdef BSDMEMMAP

/* WARNING: this code will go away next release.
 * It is to be used by pure BSD systems w/o shared memory
 * and semaphores only!
 *
 * create a shared memory segment, if it doesnot exist 
 *  ARGS    shmfile -   disk file for shared memory
 * 
 *  RETURNS     0 - successfully, fills in 'shmid'
 *             -1 - abnormal termination
 */
static int
create_shm(char *shmfile)
{
	static char buffer[MAXBLK];	/* arbitrary buffer for file initialization */

	/*
	 * first attempt to open file, if failed then create file. 
	 */

	shmid = open(shmfile, O_RDWR | O_CREAT | O_EXCL, 0666);
	if (shmid == -1) {
		fprintf(stderr, "create_shm(): create and open of %s failed",
			shmfile);
		return (-1);
	}
	/*
	 * write something in the file to make it big enough to hold the shared data.
	 */
	if (write(shmid, buffer, MAXBLK) == -1) {
		fprintf(stderr, "create_shm(): write file desc %d failure.\n",
			shmid);
		close(shmid);
		unlink(shmfile);
		return (-1);
	}
	unlink(shmfile);		/* can still write to it */
	return (0);
}
#else

/*
 * loops until both shared memory and semaphore are available.
 * returns semaphore id in "sem" and shared memory id in "shmid"
 */
static void
create_shared_memory(int *sem,
		     int *shmid)
{
	int say_mem = 0, say_sem = 0;	/* only tell user once about pause */

	for (;;) {			/* loop until succeed */
		*shmid = shmget(SHMKEY, MAXBLK, IPC_CREAT | 0666);	/* shared memory creation */
		if (*shmid < 0) {	/* try again later */
			if (say_mem % 5 == 0)	/* don't bother user too much */
				fprintf(stderr,
					"create_shared_memory(): can't create shared memory, pausing...\n");
			sleep(1);
			say_mem++;
		} else {
			*sem = semget(SEMKEY, 1, 0666 | IPC_CREAT);	/* create semaphore, init to 0 */
			if (*sem < 0) {	/* try again later */
				shmctl(*shmid, IPC_RMID, 0);	/* free the shared memory, let someone else use */
				if (say_sem % 5 == 0)	/* don't bother user too much */
					fprintf(stderr,
						"create_shared_memory(): can't create semaphore, pausing...\n");
				sleep(1);
				say_sem++;
			} else
				return;	/* we have everything we need */
		}
	}				/* end outer shared memory loop */
}
#endif

void
clear_ipc()
{
#ifdef BSDMEMMAP

	if (bufptr != NULL) {		/* only clear if set */
		if (munmap((caddr_t) bufptr, MAXBLK) == -1)
			fprintf(stderr,
				"clear_ipc(): munmap failure bufptr=%x errno=%d\n",
				bufptr, errno);
		bufptr = (char *)NULL;
	}
	if (shmid != -1) {		/* only clear if set */
		if (close(shmid) == -1)	/* Close the memory mapped file */
			fprintf(stderr, "clear_ipc(): file close failure");
		shmid = -1;
	}
#else
	if (bufptr != NULL) {		/* only clear if set */
		if (shmdt(bufptr) < 0)	/* unattach the shared memory */
			fprintf(stderr,
				"clear_ipc(): can't detach shared memory\n");
		bufptr = NULL;
	}
	if (shmid != -1) {		/* only clear if set */
		if (shmctl(shmid, IPC_RMID, 0) < 0)	/* free the shared memory */
			fprintf(stderr,
				"clear_ipc(): can't remove shared memory\n");
		shmid = -1;
	}
	if (sem != -1) {		/* only clear if set */
		if (semctl(sem, 0, IPC_RMID, 0) < 0)	/* free the semaphore */
			fprintf(stderr,
				"clear_ipc(): can't remove semaphore\n");
		sem = -1;
	}
#endif
}

static int
shared_memory(char *argv,
	      Result * res)
{
	static char buffer[MAXBLK],	/* input and output buffer */
	  shmfile[STRLEN];
	int
	  status, size,			/* transaction size */
	  result;			/* status result */

#ifndef BSDMEMMAP
	struct sembuf sem_wait, sem_signal;
#endif
	int n;

	COUNT_ZERO;			/* clear the count */

	/*
	 * Get directory for shared memory file 
	 */
	/*
	 * because we must pass an directory to the BSD version, the loop arg is hardwired :( 
	 */
	if (*argv)			/* if directory passed */
		sprintf(shmfile, "%s/%s", argv, SHMFILE);	/* append values */
	else				/* no directory passed */
		sprintf(shmfile, "%s", SHMFILE);	/* copy the filename over */
#ifdef BSDMEMMAP
	/*
	 * WARNING: this code will go away next release.
	 * * It is to be used by pure BSD systems w/o shared memory
	 * * and semaphores only!
	 */
	aim_mktemp(shmfile);		/* generate new file name */
	if (create_shm(shmfile) == -1) {	/* create shared memory segment */
		return -1;
	}
	/*
	 * Attach to the segment 
	 */
	bufptr = (char *)mmap((caddr_t) 0, MAXBLK, PROT_READ | PROT_WRITE,
			      MAP_SHARED, shmid, (off_t) 0);
	if (bufptr == (char *)-1)
		FATAL_SHARED_MEM("Unable to attach to shared memory.");	/* error handling */
	for (n = 0; n < 100; n++) {
		size = sizes[count++ % Members(sizes)];	/* get the next size */
		/*
		 * Writer Side
		 */
		result = flock(shmid, LOCK_EX);
		if (result < 0)
			FATAL_SHARED_MEM("lock for write");
		memcpy(bufptr, buffer, size);	/* copy data over there */
		result = flock(shmid, LOCK_UN);
		if (result < 0)
			FATAL_SHARED_MEM("unlock for write");
		/*
		 * Reader Side
		 */
		result = flock(shmid, LOCK_EX);
		if (result < 0)
			FATAL_SHARED_MEM("lock for read");
		memcpy(buffer, bufptr, size);	/* copy data over there */
		result = flock(shmid, LOCK_UN);
		if (result < 0)
			FATAL_SHARED_MEM("unlock for read");
		COUNT_BUMP;
	}
#else
	/*
	 * Create shared memory segment and semaphore
	 */
	create_shared_memory(&sem, &shmid);
	errno = 0;
	/*
	 * Attach to the segment
	 */
	bufptr = (char *)shmat(shmid, 0, 0);	/* attach anywhere system wants */
	if (bufptr == (char *)-1)
		FATAL_SHARED_MEM("Unable to attach to shared memory.");	/* error handling */
	semarg.val = 1;			/* initialize semaphore to 1 */
	status = semctl(sem, 0, SETVAL, semarg);	/* do it */
	if (status < 0)
		FATAL_SHARED_MEM("semctl()");	/* error here */

	sem_wait.sem_num = sem_signal.sem_num = 0;	/* use first semaphore only */
	sem_wait.sem_op = -1;		/* reduce by 1 */
	sem_signal.sem_op = 1;		/* bump by 1 */
	/*
	 * commented out because only a small # of processes at a time can set this
	 * * we were hitting the limit after 35 users, got "ENOSPC" error
	 */
	/*
	 * sem_wait.sem_flg = sem_signal.sem_flg = SEM_UNDO;
 *//*
 * handle at the end 
 */
	sem_wait.sem_flg = sem_signal.sem_flg = 0;	/* inititalize flag 1/22/96 */

	for (n = 0; n < 100; n++) {
		size = sizes[count++ % Members(sizes)];	/* get the next size */
		/*
		 * Writer Side
		 */
		result = semop(sem, &sem_wait, 1);	/* lock on first one */
		if (result < 0)
			FATAL_SHARED_MEM("sem_wait");
		memcpy(bufptr, buffer, size);	/* copy data over there */
		result = semop(sem, &sem_signal, 1);	/* free it */
		if (result < 0)
			FATAL_SHARED_MEM("sem_signal");
		/*
		 * Reader Side
		 */
		result = semop(sem, &sem_wait, 1);	/* lock on first one */
		if (result < 0)
			FATAL_SHARED_MEM("sem_wait");
		memcpy(buffer, bufptr, size);	/* copy data over there */
		result = semop(sem, &sem_signal, 1);	/* free it */
		if (result < 0)
			FATAL_SHARED_MEM("sem_signal");
		COUNT_BUMP;
	}
#endif
	clear_ipc();			/* free up shared memory and semaphore */
	return 0;			/* success */
}
