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

#define THREADS     6


struct args
{
	okl4_word_t count;
    okl4_word_t temp;
};

int
main(int argc, char **argv)
{
	int error;
    okl4_word_t i;
    okl4_kcap_t client;
	
    printf("\n--- This function will do 1 to 25 adder(multicells) ---\n");
	
	/* Initialise the libokl4 API for this thread. */
    okl4_init_thread();

    /* Wait for the first message to come in. */

    okl4_word_t bytes;
    char buffer[MAX_CHARS];

    for (i = 0; i < THREADS; i++)
    {
     	//okl4_word_t j;

  		printf("\nWait for the child thread %d to finish\n",(int)i);
        /* 用來等待其他thread傳送message過來 */        
        error = okl4_message_wait(buffer, MAX_CHARS, &bytes, &client);
		printf("Recevie child thread %d finish complete\n",(int)i);
/*
        if (bytes > MAX_CHARS) {
            bytes = MAX_CHARS;
        }

		printf("Total ans is : ");
        for (j = 0; j < bytes; j++) {
            putchar(buffer[j]);
        }
		printf("\n");     
*/

        //error = okl4_message_replywait(client, &bytes, sizeof(okl4_word_t),buffer, MAX_CHARS, &bytes, &client);
        //assert(!error);
    }

 	printf("adder complete. Exiting...\n");   
	return 0;
}

