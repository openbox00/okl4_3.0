/* @LICENCE("Public", "2008")@ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <l4/schedule.h>
#include <l4/arch/syscalls.h>
#include <okl4/init.h>
#include <okl4/env.h>
#include <okl4/kernel.h>
#include <okl4/message.h>

/* The number of characters to send each IPC. */
#define MAX_CHARS  OKL4_MESSAGE_MAX_SIZE

int
main(int argc, char **argv)
{
	printf("*******************************************************\n");
    int error;
    okl4_word_t i, bytes, msglen;
    okl4_kcap_t *echo_server;

    /* The message to send to the echo server. */
    char *message = "deserunt mollit anim id est laborum.\0\n";
    msglen = strlen(message) + 1;
	int r;


    /* Initialise the libokl4 API for this thread. */
    okl4_init_thread();
    L4_Yield();
    /* Get the capability entry for the echo server. */
    echo_server = okl4_env_get("ROOT_CELL_CAP");
    assert(echo_server != NULL);
	r = L4_ThreadControl(*echo_server, L4_nilspace, L4_nilthread, L4_nilthread, L4_nilthread, 0, (void *)0);


    /* Send the message. */
    for (i = 0; i < msglen; i += bytes) {
        /* Determine how many bytes to send. */
        if (i < msglen - MAX_CHARS) {
            bytes = MAX_CHARS;
        } else {
            bytes = msglen - i;
        }

        /* Send this chunk to the server. The server will reply the
         * number of bytes actually written. */
        error = okl4_message_call(*echo_server, &message[i], bytes,
                &bytes, sizeof(bytes), NULL);
        assert(!error);
    }
	printf("*******************************************************\n");
}

