#define _GNU_SOURCE 1

#include <sys/signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define NUM_EXTRA 20

/* Globals for building a list of caught signals */
int nextSig = 0;
int sigOrder[10];

/* Catch a signal and record that it was handled */
void handler (int signo) 
{
    sigOrder[nextSig++] = signo;
}

int main(int argc, char **argv)
{
    sigset_t mask;
    sigset_t oldMask;
    struct sigaction act;
    int i;

    /* Signals we're handling in this program */
    sigemptyset(&mask);
    sigaddset(&mask, SIGRTMIN);
    sigaddset(&mask, SIGRTMIN + 1);
    sigaddset(&mask, SIGUSR1);

    /* Send signals to handler() and keep all signals blocked that handler() 
     * has been configured to catch to avoid races in manipulating the global
     * variables
     */
    act.sa_handler = handler;
    act.sa_mask = mask;
    act.sa_flags = 0;

    sigaction(SIGRTMIN, &act, NULL);
    sigaction(SIGRTMIN + 1, &act, NULL);
    sigaction(SIGUSR1, &act, NULL);

    /* Block the signals we're working with so we can see the
     * queueing and ordering behaviour */
    sigprocmask(SIG_BLOCK, &mask, &oldMask);

    /* Generate signals */
    raise (SIGRTMIN + 1);
    raise (SIGRTMIN);
    raise (SIGRTMIN);
    raise (SIGRTMIN + 1);
    raise (SIGUSR1);
    raise (SIGUSR1);
    for(i=0; i < NUM_EXTRA; ++i){
        raise(SIGRTMIN + 1);
    }

    /* Enable delivery of the signals. They'll all be delivered 
     * right before this call returns (On Linux; this is NOT portable
     * behaviour.) */

    sigprocmask(SIG_SETMASK, &oldMask, NULL);

    /* Display the ordered list of signals we caught */
    printf("signals received:\n");
    for (i = 0; i < nextSig; i++){
        if(sigOrder[i] < SIGRTMIN){
            if(i != 0){
                printf("FAILED:\tOnly one instance of SIGUSR1 should be delivered, prior to RT signals\n");
                return 1;
            } else {
                printf("INFO:\t%s as expected\n", strsignal(sigOrder[i]));
            }
        } else {
            static int last = 0;
            if (last > sigOrder[i] - SIGRTMIN) {
                printf("FAILED:\tDelivered RT signal SIGRTMIN + %d before SIGRTMIN %d\n", sigOrder[i] - SIGRTMIN, last);
                return 1;
            } else {
                printf("INFO:\tSIGRTMIN + %d\n", sigOrder[i] - SIGRTMIN);
                last = sigOrder[i] - SIGRTMIN;
            }
        }
    }
    if (i != 5 + NUM_EXTRA){
        printf("FAILED:\tDid not deliver all RT singals\n");
        return 1;
    } else {
        printf("PASSED:\tAll RT Singals Delivered\n");
    }

    return 0;
}

