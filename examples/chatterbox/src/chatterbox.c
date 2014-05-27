#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <okl4/init.h>  //for okl4_init_thread
#include <okl4/kernel.h>    
#include <okl4/kthread.h>   //okl4_kthread_attr_init, okl4_kthread_attr_setspip
#include <okl4/message.h>   //okl4_message_send, okl4_message_wait

#include <okl4/env.h>   //for okl4_env_get
#include <okl4/kclist.h> //okl4_kclist_kcap_allocany
#include <okl4/utcb.h>  //okl4_utcb_allocany


/* The number of characters to send each IPC. */
#define MAX_CHARS  20

struct args
{
	okl4_word_t count;
    okl4_word_t temp;
};

static void aaa(int n, char *buffer)
{
	if (n == 0)
		*(buffer++) = '0';
	else {
		int f = 10000;

		if (n < 0) {
			*(buffer++) = '-';
			n = -n;
		}

		while (f != 0) {
			int i = n / f;
			if (i != 0) {
				*(buffer++) = '0'+(i%10);;
			}
			f/=10;
		}
	}
	*buffer = '\0';
}

int
main(int argc, char **argv)
{
    int error;

    okl4_kcap_t *echo_server;

/*
	for (args.count = args.id*5+1;args.count<6+args.id*5;args.count++)
	{
	args.temp = args.temp + args.count;
	}
*/

    struct args args;
	char test[20] = {0};
    okl4_word_t i, bytes;
	okl4_word_t msglen;
	args.count=0;
	args.temp = 0;

	for (args.count = 0;args.count < 26;args.count++)
	{
	args.temp = args.temp + args.count;
	}

	aaa(args.temp,test);
    msglen = strlen(test) + 1;


    /* Initialise the libokl4 API for this thread. */
    okl4_init_thread();

    /* Get the capability entry for the echo server. */
    echo_server = okl4_env_get("ROOT_CELL_CAP");
    assert(echo_server != NULL);


    /* Send the message. */
    for (i = 0; i < msglen; i += bytes) {
        /* Determine how many bytes to send. */
        if (i < msglen - MAX_CHARS) {
            bytes = MAX_CHARS;
        } else {
            bytes = msglen - i;
        }

    /* okl4_message_send用來傳送message給其他thread
     * 是blocking方式，直到目標已經準備好接收 */    

	error = okl4_message_call(*echo_server, &test[i], bytes, &bytes, sizeof(bytes), NULL);
    assert(!error);
    }
}

