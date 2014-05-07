/*
 * Copyright (c) Wipro Technologies Ltd, 2003.  All Rights Reserved.
 *
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
 *
 */
/**************************************************************************
 *
 *    TEST IDENTIFIER	: timer_create02
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Basic test for timer_create(2)
 *
 *    TEST CASE TOTAL	: 3
 *
 *    AUTHOR		: Aniruddha Marathe <aniruddha.marathe@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *     This is a Phase I test for the timer_create(2) system call.
 *     It is intended to provide a limited exposure of the system call.
 *
 * 	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	 Execute system call with different notification types for
 *	 clock ID CLOCK_REALTIME
 *	 Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *	 Otherwise, Issue a PASS message.
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 * timer_create02 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-p]
 * where:
 * 	-c n : Run n copies simultaneously.
 *	-e   : Turn on errno logging.
 *	-i n : Execute test n times.
 *	-I x : Execute test for x seconds.
 *	-p   : Pause for SIGUSR1 before starting
 *	-P x : Pause for x seconds between iterations.
 *	-t   : Turn on syscall timing.
 *
 *RESTRICTIONS:
 * None
 *****************************************************************************/

#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

#include "test.h"
#include "usctest.h"
#include "common_timers.h"

static void setup();
static void cleanup();
static void setup_test(int option);

char *TCID = "timer_create02";	/* Test program identifier.    */
int TST_TOTAL = 3;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
static struct sigevent evp;

int
main(int ac, char **av)
{
	int lc, i, j;		/* loop counter */
	char *msg;			/* message returned from parse_opts */
	timer_t created_timer_id;	/* holds the returned timer_id */
	char *message[3] = {
		"SIGEV_SIGNAL",
		"NULL",
		"SIGEV_NONE"
	};

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) !=
			(char *) NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	/* perform global setup for test */
	setup();

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			setup_test(i);

			for (j = 0; j < MAX_CLOCKS; ++j) {
				TEST(timer_create(clock_list[j], &evp,
							&created_timer_id));

				if (TEST_RETURN == -1) {
					if (TEST_ERRNO == ENOSYS) {
						/* Following statement ensures that
						 * 'remaining testcases broken' message
						 * is not displayed even though we are
						 * using tst_brkm function
						 */
						Tst_count = TST_TOTAL;
						perror("timer_create");
						tst_brkm(TBROK, cleanup,"");
					}

					TEST_ERROR_LOG(TEST_ERRNO);
					tst_resm(TFAIL, "%s with notification type %s"
							" failed with errno %d: %s",
							get_clock_str(clock_list[j]), message[i],
							TEST_ERRNO, strerror(TEST_ERRNO));
				} else {
					tst_resm(TPASS, "%s with notification type %s",
							get_clock_str(clock_list[j]), message[i]);
				}
			}
		}	/* End of TEST CASE LOOPING */
	}		/* End for TEST_LOOPING */

	/* Clean up and exit */
	cleanup();

	/* NOTREACHED */
	return 0;
}

/* setup_test() - sets up individual test */
void
setup_test(int option)
{
	switch (option) {
		case 0:
			evp.sigev_value = (sigval_t) 0;
			evp.sigev_signo = SIGALRM;
			evp.sigev_notify = SIGEV_SIGNAL;
			break;
		case 1:
			evp.sigev_value = (sigval_t) NULL;
			evp.sigev_signo = (int) NULL;
			evp.sigev_notify = (int) NULL;
			break;
		case 2:
			evp.sigev_value =  (sigval_t) 0;
			evp.sigev_signo = SIGALRM; /* any will do */
			evp.sigev_notify = SIGEV_NONE;
			break;
	}
}

/* setup() - performs all ONE TIME setup for this test */
void
setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;
}	/* End setup() */

/*
 * cleanup() - Performs one time cleanup for this test at
 * completion or premature exit
 */

void
cleanup()
{
	/*
	* print timing stats if that option was specified.
	* print errno log if that option was specified.
	*/
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}	/* End cleanup() */
