/*
* Copyright (c) 2005, Bull S.A..  All rights reserved.
* Created by: Sebastien Decugis

* This program is free software; you can redistribute it and/or modify it
* under the terms of version 2 of the GNU General Public License as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it would be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write the Free Software Foundation, Inc., 59
* Temple Place - Suite 330, Boston MA 02111-1307, USA.


* This sample test aims to check the following assertion:
*
* The function does not return EINTR

* The steps are:
* -> kill a thread which calls pthread_atfork
* -> check that EINTR is never returned

*/

/* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
#define _POSIX_C_SOURCE 200112L

/******************************************************************************/
/*********************** standard includes ************************************/
/******************************************************************************/
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sched.h>
#include <semaphore.h>
#include <errno.h>
#include <signal.h>

/******************************************************************************/
/***********************   Test framework   ***********************************/
/******************************************************************************/
#include "testfrmw.h"
#include "testfrmw.c" 
/* This header is responsible for defining the following macros:
 * UNRESOLVED(ret, descr);  
 *    where descr is a description of the error and ret is an int 
 *   (error code for example)
 * FAILED(descr);
 *    where descr is a short text saying why the test has failed.
 * PASSED();
 *    No parameter.
 * 
 * Both three macros shall terminate the calling process.
 * The testcase shall not terminate in any other maneer.
 * 
 * The other file defines the functions
 * void output_init()
 * void output(char * string, ...)
 * 
 * Those may be used to output information.
 */

/******************************************************************************/
/***************************** Configuration **********************************/
/******************************************************************************/
#ifndef VERBOSE
#define VERBOSE 1
#endif

//#define WITH_SYNCHRO

/******************************************************************************/
/*****************************    Test case   *********************************/
/******************************************************************************/

char do_it = 1;
unsigned long count_ope = 0;
#ifdef WITH_SYNCHRO
sem_t semsig1;
sem_t semsig2;
unsigned long count_sig = 0;
#endif

sigset_t usersigs;

typedef struct
{
	int sig;
#ifdef WITH_SYNCHRO
	sem_t *sem;
#endif
}

thestruct;

/* the following function keeps on sending the signal to the process */
void * sendsig ( void * arg )
{
	thestruct * thearg = ( thestruct * ) arg;
	int ret;
	pid_t process;

        printf("%s :\n",__func__);

	process = getpid();

	/* We block the signals SIGUSR1 and SIGUSR2 for this THREAD */
	ret = pthread_sigmask( SIG_BLOCK, &usersigs, NULL );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Unable to block SIGUSR1 and SIGUSR2 in signal thread" );
	} else {
            printf("%s: pthread_sigmask( SIG_BLOCK, &usersigs, NULL) returned successfully\n", __func__);
        }

	while ( do_it )
	{
#ifdef WITH_SYNCHRO

		if ( ( ret = sem_wait( thearg->sem ) ) )
		{
			UNRESOLVED( errno, "Sem_wait in sendsig" );
		} else {
                    //printf("%s: sem_wait(thearg->sem) returned successfully\n", __func__);
                }


		count_sig++;
#endif

		ret = kill( process, thearg->sig );

		if ( ret != 0 )
		{
			UNRESOLVED( errno, "Kill in sendsig" );
		} else {
                    //printf("%s: kill(process, thearg->sig) returned successfully\n", __func__);
                }


	}
        printf("%s: done_it\n", __func__);

        printf("%s: exiting successfully\n", __func__);
	return NULL;
}

/* Next are the signal handlers. */
/* This one is registered for signal SIGUSR1 */
void sighdl1( int sig )
{
    printf("%s: start\n", __func__);
#ifdef WITH_SYNCHRO

	if ( sem_post( &semsig1 ) )
	{
		UNRESOLVED( errno, "Sem_post in signal handler 1" );
	}

#endif
}

/* This one is registered for signal SIGUSR2 */
void sighdl2( int sig )
{
    printf("%s: start\n", __func__);
#ifdef WITH_SYNCHRO

	if ( sem_post( &semsig2 ) )
	{
		UNRESOLVED( errno, "Sem_post in signal handler 2" );
	}

#endif
}

void prepare( void )
{
    printf("%s: start\n", __func__);
	return ;
}

void parent( void )
{
    printf("%s: start\n", __func__);
	return ;
}

void child( void )
{
    printf("%s: start\n", __func__);
	return ;
}

/* Test function -- calls pthread_setschedparam() and checks that EINTR is never returned. */
void * test( void * arg )
{
	int ret = 0;

	/* We don't block the signals SIGUSR1 and SIGUSR2 for this THREAD */
	ret = pthread_sigmask( SIG_UNBLOCK, &usersigs, NULL );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Unable to unblock SIGUSR1 and SIGUSR2 in worker thread" );
	} else {
                    printf("%s: pthread_sigmask( SIG_UNBLOCK, &usersigs, NULL) returned successfully\n", __func__);
        }



        printf("%s: running pthread_atfork( prepare, parent, child) in a poll loop\n", __func__);
	while ( do_it )
	{
		count_ope++;
		ret = pthread_atfork( prepare, parent, child );

		if ( ret == EINTR )
		{
			FAILED( "EINTR was returned" );
		}

	}
        printf("%s: done_it\n", __func__);

        printf("%s: exiting successfully\n", __func__);
	return NULL;
}


/* Main function */
int main ( int argc, char * argv[] )
{
	int ret;
	pthread_t th_work, th_sig1, th_sig2, me;
	thestruct arg1, arg2;

	struct sigaction sa;

	/* Initialize output routine */
	output_init();
        fprintf(stderr, "%s:\n", argv[0]);

	/* We need to register the signal handlers for the PROCESS */
	sigemptyset ( &sa.sa_mask );
	sa.sa_flags = 0;
	sa.sa_handler = sighdl1;

	if ( ( ret = sigaction ( SIGUSR1, &sa, NULL ) ) )
	{
		UNRESOLVED( ret, "Unable to register signal handler1" );
	} else {
                printf("sigaction ( SIGUSR1, &sa, NULL ) ) returned successfully\n" );
        }


	sa.sa_handler = sighdl2;

	if ( ( ret = sigaction ( SIGUSR2, &sa, NULL ) ) )
	{
		UNRESOLVED( ret, "Unable to register signal handler2" );
	} else {
                printf("sigaction ( SIGUSR2, &sa, NULL ) ) returned successfully\n" );
        }


	/* We prepare a signal set which includes SIGUSR1 and SIGUSR2 */
	sigemptyset( &usersigs );

	ret = sigaddset( &usersigs, SIGUSR1 );

	ret |= sigaddset( &usersigs, SIGUSR2 );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Unable to add SIGUSR1 or 2 to a signal set" );
	} else {
            printf("ret from sigaddset for SIGUSR1 & SIGUSR2 is 0 as expected\n" );
        }


	/* We now block the signals SIGUSR1 and SIGUSR2 for this THREAD */
	ret = pthread_sigmask( SIG_BLOCK, &usersigs, NULL );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Unable to block SIGUSR1 and SIGUSR2 in main thread" );
	} else {
            printf("ret from pthread_sigmask is 0 as expected\n" );
        }


#ifdef WITH_SYNCHRO
	if ( sem_init( &semsig1, 0, 1 ) )
	{
		UNRESOLVED( errno, "Semsig1  init" );
	}

	if ( sem_init( &semsig2, 0, 1 ) )
	{
		UNRESOLVED( errno, "Semsig2  init" );
	}

#endif

	me = pthread_self();

	if ( ( ret = pthread_create( &th_work, NULL, test, &me ) ) )
	{
		UNRESOLVED( ret, "Worker thread creation failed" );
	} else {
            printf("ret from pthread_create( &th_work, NULL, test, &me) is 0 as expected\n" );
        }


	arg1.sig = SIGUSR1;
	arg2.sig = SIGUSR2;
#ifdef WITH_SYNCHRO
	arg1.sem = &semsig1;
	arg2.sem = &semsig2;
#endif



	if ( ( ret = pthread_create( &th_sig1, NULL, sendsig, ( void * ) & arg1 ) ) )
	{
		UNRESOLVED( ret, "Signal 1 sender thread creation failed" );
	} else {
            printf("ret from pthread_create( &th_sig1, NULL, sendsig, ( void * ) & arg1 ) is 0 as expected\n" );
        }


	if ( ( ret = pthread_create( &th_sig2, NULL, sendsig, ( void * ) & arg2 ) ) )
	{
		UNRESOLVED( ret, "Signal 2 sender thread creation failed" );
	} else {
            printf("ret from pthread_create( &th_sig2, NULL, sendsig, ( void * ) & arg2 ) is 0 as expected\n" );
        }




	/* Let's wait for a while now */
	sleep( 1 );


	/* Now stop the threads and join them */
	do
	{
		do_it = 0;
	}
	while ( do_it );

        printf("%s: done_it\n", __func__);


	if ( ( ret = pthread_join( th_sig1, NULL ) ) )
	{ 
		UNRESOLVED( ret, "Signal 1 sender thread join failed" );
	} else {
            printf("pthread_join( th_sig1, NULL) returned success.\n");
        }

	if ( ( ret = pthread_join( th_sig2, NULL ) ) )
	{
		UNRESOLVED( ret, "Signal 2 sender thread join failed" );
	} else {
            printf("pthread_join( th_sig2, NULL) returned success.\n");
        }



	if ( ( ret = pthread_join( th_work, NULL ) ) )
	{
		UNRESOLVED( ret, "Worker thread join failed" );
	} else {
            printf("pthread_join( th_work, NULL) returned success.\n");
        }



#if VERBOSE > 0
	output( "Test executed successfully.\n" );

	output( "  %d operations.\n", count_ope );

#ifdef WITH_SYNCHRO
	output( "  %d signals were sent meanwhile.\n", count_sig );

#endif
#endif
	PASSED;
}


