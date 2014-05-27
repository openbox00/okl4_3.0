#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <okl4/init.h>  //for okl4_init_thread
#include <okl4/kernel.h>    //資料結構???
#include <okl4/kthread.h>   //okl4_kthread_attr_init, okl4_kthread_attr_setspip
#include <okl4/message.h>   //okl4_message_send, okl4_message_wait

#include <okl4/env.h>   //for okl4_env_get
#include <okl4/kclist.h>    //okl4_kclist_kcap_allocany
#include <okl4/utcb.h>  //okl4_utcb_allocany

#define THREADS     1
#define STACK_SIZE  4096

#define MAX_CHARS  20

void adder(void);

struct args
{
    okl4_word_t id;
	okl4_word_t count;
    okl4_kcap_t parent;
    okl4_word_t temp;
};
struct args args;

static 
void aaa(int n, char *buffer)
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
    okl4_word_t i;
    void *stacks[THREADS];
    okl4_kthread_t threads[THREADS];
    okl4_kcap_t caps[THREADS];
	
    okl4_kspace_t *root_kspace;
	okl4_kthread_t *root_thread;
    okl4_kclist_t *root_kclist;
    	
    struct okl4_utcb_item *utcb_item[THREADS];
    struct okl4_kcap_item *kcap_item[THREADS];
	
	root_kspace = okl4_env_get("MAIN_KSPACE");
    root_thread = root_kspace->kthread_list;
    root_kclist = root_kspace->kclist;

    /* 在系統中，每個thread使用任何OKL4 lib function之前，
     * 必須呼叫此function
     */
    okl4_init_thread();
	
    /* okl4_env_get 是用來查找name參數指定的環境項目
     * name必須要是大小字母 
     */
    root_kspace = okl4_env_get("MAIN_KSPACE");
    root_thread = root_kspace->kthread_list;
    root_kclist = root_kspace->kclist;

    printf("\n--- This function will do 1 to 25 adder ---\n");

	/* setup each thread env */
    for (i = 0; i < THREADS; i++)
    {

        okl4_kthread_attr_t kthread_attr;

        /* Allocate UTCB的結構大小 */
        utcb_item[i] = malloc(sizeof(struct okl4_utcb_item));
		
        /* okl4_utcb_allocany用處是從UTCB區域內allocate任何UTCB
         * 所以用來決定UTCB的位置(至於大小則是由上面malloc決定) 
         */
        error = okl4_utcb_allocany(root_kspace->utcb_area, utcb_item[i]);

        /* Allocate cap結構大小 */
        kcap_item[i] = malloc(sizeof(struct okl4_kcap_item));

        /* okl4_kclist_kcap_allocany用處是，在特定kclist(kclist變數)內，
         * 用來從任何cap slot內allocate任何cap
         * 所以用來決定位置(至於大小則是由上面malloc決定) 
         */
        error = okl4_kclist_kcap_allocany(root_kclist, kcap_item[i]);


        /* 設定完環境，還要設定放置變數的空間
         * 這邊設定stack大小
         */
        stacks[i] = malloc(STACK_SIZE);
		
        /* okl4_kthread_attr_init是用來初始化kthread屬性
         * 設定好位置，將上面root設定填進去
         */
        okl4_kthread_attr_init(&kthread_attr);


		/* 設定thread內的function、stack 、屬性位置 */
		okl4_kthread_attr_setspip(&kthread_attr,
                (okl4_word_t)stacks[i] + STACK_SIZE,
                (okl4_word_t)(adder));
				
		/*  */		
        okl4_kthread_attr_setspace(&kthread_attr, root_kspace);
        okl4_kthread_attr_setutcbitem(&kthread_attr, utcb_item[i]);
        okl4_kthread_attr_setcapitem(&kthread_attr, kcap_item[i]);
		
        /* 根據kthread_attr，create一個命名為thread[i]的thread*/
        error = okl4_kthread_create(&threads[i], &kthread_attr);
        /* 用來取得命名為thread[i]的thread內的kcap item */
        caps[i] = okl4_kthread_getkcap(&threads[i]);
        /* 開始執行已經創造好的thread */
        okl4_kthread_start(&threads[i]);
    }
	
    okl4_word_t bytes;
	args.parent = okl4_kthread_getkcap(root_thread);

    char buffer[MAX_CHARS];
	printf("Wait for the first message to come in.\n");
    error = okl4_message_wait(buffer, MAX_CHARS, &bytes, caps);


    assert(!error);

  
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
		printf("\n");
   		printf("adder complete. Exiting...\n");   
        /* Reply to the client, letting them know how many bytes
         * we processed, and wait for the next message to arrive. */
		//printf(" Reply to the client, letting them know how many bytes we processed, and wait for the next message to arrive\n");
       // error = okl4_message_replywait(caps[0], &bytes, sizeof(okl4_word_t),buffer, MAX_CHARS, &bytes, caps);
       // assert(!error);
		return 0;
    }
}

void
adder(void)
{
	int error;
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
	error = okl4_message_call(args.parent, &test[i], bytes, &bytes, sizeof(bytes), NULL);
    assert(!error);
    }		
}
