#include <stdio.h>
#include <stdlib.h>

#include <okl4/init.h>  //for okl4_init_thread
#include <okl4/kernel.h>    
#include <okl4/kthread.h>   //okl4_kthread_attr_init, okl4_kthread_attr_setspip
#include <okl4/message.h>   //okl4_message_send, okl4_message_wait

#include <okl4/env.h>   //for okl4_env_get
#include <okl4/kclist.h> //okl4_kclist_kcap_allocany
#include <okl4/utcb.h>  //okl4_utcb_allocany


#define MAX_CHARS  OKL4_MESSAGE_MAX_SIZE


int
main(int argc, char **argv)
{
	int error;
    okl4_word_t bytes;
    char buffer[MAX_CHARS];
    okl4_kcap_t client;
	
    printf("\n--- This function will do 1 to 25 adder ---\n");
	
	/* Initialise the libokl4 API for this thread. */
    okl4_init_thread();

    /* Wait for the first message to come in. */
    error = okl4_message_wait(buffer, MAX_CHARS, &bytes, &client);
    assert(!error);

    /* Main server loop. */
    while (1) {
        okl4_word_t i;

        /* If the number of bytes sent by the client exceeds our buffer
         * size, we would have lost some of the message. */
        if (bytes > MAX_CHARS) {
            bytes = MAX_CHARS;
        }

        /* Print out the message. */
        printf("Total ans is : ");
        for (i = 0; i < bytes; i++) {
            putchar(buffer[i]);
        }
   		printf("adder complete. Exiting...\n");   

        /* Reply to the client, letting them know how many bytes
         * we processed, and wait for the next message to arrive. */
        error = okl4_message_replywait(client, &bytes, sizeof(okl4_word_t),
                buffer, MAX_CHARS, &bytes, &client);
        assert(!error);
		return 0;
    }
    /* Not reached. */
    while (1);
}

