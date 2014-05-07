/*
 * Copyright (C) Bull S.A. 2001
 * Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/******************************************************************************/
/*                                                                            */
/* Dec-03-2001  Created: Jacky Malcles & Jean Noel Cordenner                  */
/*              These tests are adapted from AIX float PVT tests.             */
/*                                                                            */
/******************************************************************************/
#include "tfloat.h"

#include "test.h"

/* LTP status reporting */
char *TCID;	 		/* Test program identifier.    */
int TST_TOTAL=1;    		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

/* To avoid extensive modifications to the code, use this bodge */
#define exit(x) myexit(x)
void
myexit (int x)
{
  if (x) 
    tst_resm (TFAIL, "Test failed");
  else
    tst_resm (TPASS, "Test passed");
  tst_exit();
}

TH_DATA *pcom;
TH_DATA **tabcom;
TH_DATA **tabcour;
#ifndef	PATH_MAX
#define  PATH_MAX        	1024
#endif
char datadir[PATH_MAX];     		/* DATA directory */

#ifndef PTHREAD_THREADS_MAX
#define PTHREAD_THREADS_MAX 1024
#endif
#define DEFAULT_NUM_THREADS             20
int     num_threads      = DEFAULT_NUM_THREADS;
int     num_loops=500; 

int     sig_cancel = 0; /* flag set by handle_signals to tell initial thread
			to stop creating new threads (signal caught) */

int	indice = 0; /* # of threads created, to be canceled by handle_signals
			or waited for by initial thread */

pthread_mutex_t sig_mutex;
pthread_t       *threads;

int     debug = 0;
int     true = 1;

static void *handle_signals (void *);

static void sys_error (const char *, int);

const double EPS=  0.1e-300;

const int nb_func = NB_FUNC;

int generate(char *datadir)
{
 char *fmt = "cd %s; ./%s";
 char *cmdline = malloc (strlen(fmt) + strlen(datadir) + strlen(GENERATOR));
 if (cmdline == NULL)
     return(1);
 sprintf(cmdline,fmt,datadir,GENERATOR);
 system(cmdline);
 free(cmdline);
 return(0);
}

int main(int argc, char *argv[])
{
        int             opt = 0;
        pid_t           pid;
        extern char     *optarg;

        void *exit_value;
        pthread_attr_t newattr;
        pthread_t sig_hand;
        size_t          stacksize = 2093056;
        int th_num;
        int retvalend = 0;
        int retval = 0;
	int error =0;
	/*int time=1;*/
	int i;

	/* Generate test ID from invocation name */
	if ((TCID = strrchr (argv[0], '/')) != NULL)
	  TCID++;
	else
	  TCID = argv[0];

	setbuf(stdout, (char *)0);
	setbuf(stderr, (char *)0);
	datadir[0] = '.';
	datadir[1] = '\0';

	if(argc != 1)
	{
 	while ( (opt = getopt( argc, argv, "vn:l:D:?" )) != EOF ) {
       	 switch ( opt ) {
                case 'v':
			++debug; /* verbose mode */
			break; 
                case 'n':
                        num_threads = atoi (optarg);
                        break;
                case 'l':
                        num_loops = atoi (optarg);
                        break;
                case 'D':
                        strncpy(datadir, optarg, PATH_MAX);
			break;
                default:
                        fprintf (stderr, "Usage: %s [-n number_of_threads] [-v]", argv[0]);
                        fprintf (stderr, "[-l number_of_loops]\n");
                        fprintf (stderr, "[-D DATAs absolute path]\n");
                        exit (1);
                }
        }
	}
	pid=fork();
        if ( pid == 0 ){                    /*Child*/
		generate((char*)&datadir);          
		return(0);} 
	else                                /*Parent*/
		waitpid(pid,NULL,0);

	if(debug)
		printf("%s: will run for %d loops\n", argv[0], num_loops);

	if(debug)
		printf("%s: using %s as data directory\n", argv[0], datadir);

	if(num_threads <= 0) {
		printf("WARNING: num_threads undefined or incorrect, using \"1\"\n");
		num_threads = 1;
	}

	if(nb_func * num_threads > PTHREAD_THREADS_MAX-2) {
		while(nb_func * num_threads > PTHREAD_THREADS_MAX-2)
			--num_threads;
	}
	if(debug)
		printf("%s: will run %d functions, %d threads per function\n",
			argv[0], nb_func, num_threads);

        retval = pthread_mutex_init (&sig_mutex, (pthread_mutexattr_t *)NULL);
        if (retval != 0 )
                sys_error("main : mutex_init(&sig_mutex) FAILED",__LINE__);

        retval = pthread_create (&sig_hand, (pthread_attr_t *)NULL,
                                 handle_signals, (void *) NULL);
        if (retval != 0)
                sys_error("main : create(&sig_hand) FAILED",__LINE__);

	/*
         * Start all calculation threads...
         */
        threads = (pthread_t * ) malloc ((size_t) (nb_func 
                                  * num_threads * sizeof (pthread_t)));

	tabcom  = (TH_DATA **) malloc ( (size_t)(sizeof (TH_DATA *)
                                       * nb_func*num_threads));
	tabcour = tabcom;

	retval = pthread_attr_init(&newattr);
	if (retval != 0)
		sys_error("main : attr_init(&newattr) FAILED",__LINE__);

        if (pthread_attr_setstacksize (&newattr, stacksize))
       	        sys_error ("main: pthread_attr_setstacksize failed", __LINE__);

	retval = pthread_attr_setdetachstate(&newattr,
                                             PTHREAD_CREATE_JOINABLE);
	if (retval != 0)
        	sys_error("main : attr_setdetachstate(&newattr) FAILED",
                           __LINE__);
	

	/* run the nb_func functions on num_threads */ 

	indice = 0;
	for (i = 0; i < nb_func; i++) {

	  for (th_num = 0; th_num < num_threads; th_num++) {

		/* allocate struct of commucation  with the thread */
		pcom =  calloc ((size_t)1, (size_t)sizeof(TH_DATA));
		*tabcour = (TH_DATA *) pcom;
		tabcour++;
		/* */
		/* update structure of communication */
		/* */
		pcom-> th_num = th_num;
		pcom-> th_func = th_func[i];

		pthread_mutex_lock (&sig_mutex);

		if (sig_cancel) { /* stop processing right now! */
			pthread_mutex_unlock (&sig_mutex);
			goto finished;
		}
		retval = pthread_create (&threads[indice], &newattr, 	 
		                         thread_code,(void * ) pcom);
		if (retval != 0)
                        sys_error("main : create FAILED",__LINE__);
		indice++;
		pthread_mutex_unlock (&sig_mutex);

	   }/* num_threads */
	} /* for i*/

	/*alarm(60*time);*/               /* start all threads for TEST_time */

	/*
         * Wait for the threads finish their task
         * pthread_join () will block
         */

finished:
	if (debug) {
		printf ("Initial thread: Waiting for %d threads to finish\n", indice);
	}
        tabcour = tabcom;

	for (th_num = 0; th_num < indice; th_num++) {
		retvalend = pthread_join (threads[th_num], &exit_value);
		if (retvalend != 0)
			sys_error("finish : join FAILED",__LINE__);

		/* test the result in TH_DATA : communication buffer */
		pcom = * tabcour++; 
		if(pcom->th_result !=0 ) {
	           error++;
                   fprintf (stderr, "thread %d (%s) terminated unsuccessfully %d errors/%d loops\n", th_num,pcom->th_func.fident,pcom->th_nerror,pcom->th_nloop);
                   fprintf (stderr, "%s", pcom->detail_data);
		}
		else if (debug) {
			printf ("thread %d (%s) terminated successfully %d loops.\n",
				th_num, pcom->th_func.fident, pcom->th_nloop-1);
		}

	}
	if (error) exit (1);
	else exit(0);
	return 0;
}



/*---------------------------------------------------------------------+
|                            handle_signals ()                         |
| ==================================================================== |
|                                                                      |
| Function:  ....                                                      |
|            If SIGALRM or SIGUSR1 or SIGINT : cancel threads          |
|                                                                      |
| Updates:   ....                                                      |
|                                                                      |
+---------------------------------------------------------------------*/
static void *handle_signals (void *arg)
{
	sigset_t signals_set;
	int	thd;
	int	sig;
	int	retvalsig = 0;

	if (debug) {
		printf ("signal handler %lu started\n", pthread_self());
	}
	/*
         * Set up the signals that we want to handle...
         */
	sigemptyset (&signals_set);
	sigaddset (&signals_set, SIGINT);
	sigaddset (&signals_set, SIGQUIT);
	sigaddset (&signals_set, SIGTERM);
	sigaddset (&signals_set, SIGUSR1);
	sigaddset (&signals_set, SIGALRM);
	do
	{
		if (debug) {
			printf ("Signal handler starts waiting...\n");
		}

		sigwait (&signals_set, &sig);
		if (debug) {
			printf ("Signal handler caught signal %d\n", sig);
		}

		switch (sig) {
		case SIGALRM:
		case SIGUSR1:
		case SIGINT:
			if (sig_cancel) {
				printf ("Signal handler: Already finished, Ignoring signal\n");
			}
			else {
				/*
                                 * Have to signal all non started threads...
                                 */

				retvalsig = pthread_mutex_lock (&sig_mutex);
				if (retvalsig != 0)
					sys_error("handle_signal : mutex_lock(&sig_mutex) FAILED",__LINE__);

				sig_cancel = 1;
				retvalsig = pthread_mutex_unlock (&sig_mutex);
				if (retvalsig != 0)
					sys_error("handle_signal : mutex_unlock(&sig_mutex) FAILED",__LINE__);

				/*
				 * ......... and all started
		     	       	 */
				for (thd = 0; thd < indice; thd++) {
					if (debug) {
						printf ("Signal handler: canceling thread  (%d of %d)\n", thd, indice);
					}
					retvalsig = pthread_cancel (threads[thd]);
					if (retvalsig != 0)
						sys_error("handle_signal : cancel FAILED",__LINE__);
				}
			}
			break;
		case SIGQUIT:
			printf ("Signal handler: Caught Quit, Doing nothing\n");
			break;
		case SIGTERM:
			printf ("Signal handler: Caught Termination, Doing nothing\n");
			break;
		default:
			exit (-1);
		}
	}while (TRUE);
}
/*---------------------------------------------------------------------+
|                               error ()                               |
| ==================================================================== |
|                                                                      |
| Function:  Prints out message and exits...                           |
|                                                                      |
+---------------------------------------------------------------------*/
static void error (const char *msg, int line)
{
        fprintf (stderr, "ERROR [line: %d] %s\n", line, msg);
        exit (-1);
}



	
/*---------------------------------------------------------------------+
|                             sys_error ()                             |
| ==================================================================== |
|                                                                      |
| Function:  Creates system error message and calls error ()           |
|                                                                      |
+---------------------------------------------------------------------*/
static void sys_error (const char *msg, int line)
{
        char syserr_msg [256];

        sprintf (syserr_msg, "%s: %s\n", msg, strerror(errno));
        error (syserr_msg, line);
}

