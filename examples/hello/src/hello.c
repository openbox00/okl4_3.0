/* @LICENCE("Public", "2008")@ */

#include <stdio.h>
#include <stdlib.h>


int
main(int argc, char **argv)
{
	printf("*****************************************************************************\n");
    /* Initialise the libokl4 API for this thread. */
    okl4_init_thread();
    L4_Yield();
}

