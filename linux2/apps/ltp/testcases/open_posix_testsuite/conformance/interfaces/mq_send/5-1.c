/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that if the message queue is full and O_NONBLOCK is not set, mq_send()
 * will block until it can place the message in the queue.
 *
 * Test by sending messages in a child process until the message queue is full.
 * At this point, the child should be blocking on sending.  Then, have the
 * parent receive the message and return pass when the next message is sent
 * to the message queue.
 *
 * 3/13/03 - Added fix from Gregoire Pichon for specifying an attr
 *           with a mq_maxmsg >= BUFFER.
 *
 */

#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <errno.h>
#include "posixtest.h"

#define NAMESIZE 50
#define MSGSTR "0123456789"
#define BUFFER 40
#define MAXMSG 5

char gqname[NAMESIZE];
mqd_t gqueue;

/*
 * This handler is just used to catch the signal and stop sleep (so the
 * parent knows the child is still busy sending signals).
 */
void stopsleep_handler(int signo)
{
	return;
}

int main()
{
	int pid;
	char msgrcd[BUFFER];
        const char *msgptr = MSGSTR;
	struct mq_attr attr;
	int unresolved=0, pri;

        sprintf(gqname, "/mq_send_5-1_%d", getpid());

	attr.mq_msgsize = BUFFER;
	attr.mq_maxmsg = MAXMSG;
        gqueue = mq_open(gqname, O_CREAT |O_RDWR, S_IRUSR | S_IWUSR, &attr);
        if (gqueue == (mqd_t)-1) {
                perror("mq_open() did not return success");
                return PTS_UNRESOLVED;
        }

	if ((pid = fork()) == 0) {
		/* child here */
		int i;

		sleep(1);  // give parent time to set up handler
		for (i=0; i<MAXMSG+1; i++) {
        		mq_send(gqueue, msgptr, strlen(msgptr), 1);
			/* send signal to parent each time message is sent */
			kill(getppid(), SIGABRT);
		}
	} else {
		/* parent here */
		struct sigaction act;
		int j;

		/* parent runs stopsleep_handler when sleep is interrupted
                   by child */
		act.sa_handler=stopsleep_handler;
		act.sa_flags=0;
		sigemptyset(&act.sa_mask);
		sigaction(SIGABRT, &act, 0);

		for (j=0; j<MAXMSG+1; j++) {  // "infinite" loop
			if (sleep(3) == 0) {
			/* If sleep finished, child is probably blocking */
				break;
			}
		}

		if (j == MAXMSG+1) {
			printf("Child never blocked\n");
			kill(pid, SIGKILL); //kill child
			return PTS_FAIL;
		}

		/* receive message and allow blocked send to complete */
		if (mq_receive(gqueue, msgrcd, BUFFER, &pri) == -1) {
			perror("mq_receive() did not return success");
			unresolved = 1;
		}

                if (sleep(3) == 0) {
                        /*
                         * mq_send didn't succeed and interrupt sleep()
                         * with a signal
                         */
                        kill(pid, SIGKILL); //kill child
                        printf("mq_send() didn't appear to complete\n");
			mq_close(gqueue);
			mq_unlink(gqname);
                        printf("Test FAILED\n");
                        return PTS_FAIL;
                }

		mq_close(gqueue);
		mq_unlink(gqname);
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	return PTS_UNRESOLVED;
}

