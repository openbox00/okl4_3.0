/*
 * Copyright (c) 2004, Bull SA. All rights reserved.
 * Created by:  Laurent.Vivier@bull.net
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 */

#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <aio.h>

#include "posixtest.h"

#define TNAME "aio_fsync/8-2.c"

int main()
{
	char tmpfname[256];
#define BUF_SIZE 111
	char buf[BUF_SIZE];
	int fd;
	struct aiocb aiocb_write;
	struct aiocb aiocb_fsync;

#if _POSIX_ASYNCHRONOUS_IO != 200112L
	exit(PTS_UNSUPPORTED);
#endif

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_fsync_8_2_%d", 
		  getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL,
		  S_IRUSR | S_IWUSR);
	if (fd == -1)
	{
		printf(TNAME " Error at open(): %s\n",
		       strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	unlink(tmpfname);

	memset(&aiocb_write, 0, sizeof(aiocb_write));
	aiocb_write.aio_fildes = fd;
	aiocb_write.aio_buf = buf;
	aiocb_write.aio_nbytes = BUF_SIZE;

	if (aio_write(&aiocb_write) == -1)
	{
		printf(TNAME " Error at aio_write(): %s\n",
		       strerror(errno));
		exit(PTS_FAIL);
	}

	memset(&aiocb_fsync, 0, sizeof(aiocb_fsync));
	aiocb_fsync.aio_fildes = fd;
	aiocb_fsync.aio_buf = NULL;

	if (aio_fsync(O_SYNC, &aiocb_fsync) != 0)
	{
		printf(TNAME " Error at aio_fsync(): %s\n", strerror(errno));
		exit(PTS_FAIL);
	}

	close(fd);
	printf ("Test PASSED\n");
	return PTS_PASS;
}
