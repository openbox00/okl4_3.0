/*   
 * Copyright (c) 2004, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * adam.li@intel.com
 *
 * if _POSIX_THREAD_CPUTIME is defined, the new thread shall have a CPU-time
 * clock accessible, and the initial value of this clock shall be set to 0.
 */
 /* Create a new thread and get the time of the thread CUP-time clock 
  * using clock_gettime().
  * Note, the tv_nsec cannot be exactly 0 at the time of calling 
  * clock_gettime() since the thread has executed some time. */

#define _XOPEN_SOURCE 600
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "posixtest.h"

void *a_thread_func()
{
	clockid_t cpuclock = (clockid_t)0;

	struct timespec ts = {
            .tv_sec = 125, 
            .tv_nsec = 125
        };

	pthread_getcpuclockid(pthread_self(), &cpuclock);
        printf("%s: cpuclock=0x%x\n",__func__, (int)cpuclock);
	clock_gettime(cpuclock, &ts);
	/* Just test the tv_sec field here. */
	if (ts.tv_sec != 0)
	{
		printf("ts.tv_sec: %ld, ts.tv_nsec: %ld\n", 
			ts.tv_sec, ts.tv_nsec);
                printf("FAIL: thead CUP-time clock derived ts.tv_sec should be 0 for new thread\n");
		exit(PTS_FAIL);
	}
	pthread_exit(0);
	return NULL;
}

int main()
{
#ifndef _POSIX_THREAD_CPUTIME
	printf("_POSIX_THREAD_CPUTIME not supported\n");
	return PTS_UNSUPPORTED;
#endif
	pthread_t new_th;
	
	if(pthread_create(&new_th, NULL, a_thread_func, NULL) != 0)
	{	
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}
	
        printf("Info: calling pthread_join(new_th, NULL);\n");

	pthread_join(new_th, NULL);
	printf("Test PASSED\n");
	return PTS_PASS;	
}


