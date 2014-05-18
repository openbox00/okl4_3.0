#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//#include <soc/fpga.h>

#include <okl4/init.h>  //for okl4_init_thread
#include <okl4/kernel.h>    
#include <okl4/kthread.h>   //okl4_kthread_attr_init, okl4_kthread_attr_setspip
#include <okl4/message.h>   //okl4_message_send, okl4_message_wait

#include <okl4/env.h>   //for okl4_env_get
#include <okl4/kclist.h>    //okl4_kclist_kcap_allocany
#include <okl4/utcb.h>  //okl4_utcb_allocany


#define THREADS     5
#define STACK_SIZE  4096

void adder(void);

struct args
{
    okl4_word_t id;
	okl4_word_t count;
    okl4_kcap_t parent;
    okl4_word_t temp;
};

int
main(int argc, char **argv)
{
    int error;
    okl4_word_t i;
#if 0
	uint64_t a;
	struct timer_interface interface;
	struct timer_interface * ptr;
	ptr = &interface;
	a =	timer_get_tick_frequency(ptr);

	printf("------------------------------------------------------%lld\n",a);
#endif


    void *stacks[THREADS];
	
    okl4_kthread_t threads[THREADS];
	
    okl4_kcap_t caps[THREADS];
	
    okl4_kspace_t *root_kspace;
	
    okl4_kthread_t *root_thread;
    okl4_kclist_t *root_kclist;
    
	
    struct okl4_utcb_item *utcb_item[THREADS];	
    struct okl4_kcap_item *kcap_item[THREADS];
	
    struct args args;

    int ans = 0;

    okl4_init_thread();


#if 0
void okl4_init_thread(void)
{
    /*
     * TODO: Move this into a proper crt0.
     */
    OKL4_ARCH_THREAD_INIT(); // do { } while(0)

    /* Set up notification acceptor to always accept notification
     * bits. */
    L4_Accept(L4_NotifyMsgAcceptor);
}
#endif

	/************************************************************************/
    clock_t start, finish; 
    double Total_time;
	start = clock(); 
	finish = clock(); 
	Total_time = (double)(finish-start) / CLOCKS_PER_SEC; 
	printf( "start = %ld seconds\n", start); 
	printf( "finish = %ld seconds\n", finish); 

	printf( "%f seconds\n", Total_time); 

#if 0	
	unsigned long b;
	b = soc_get_timer_tick_length();
	printf("--------------------------------------------%ld\n",b);
#endif
	/************************************************************************/	
	
    root_kspace = okl4_env_get("MAIN_KSPACE");
#if 0
void * okl4_env_get(const char * name)
{
    okl4_word_t i;
    okl4_word_t j;

    for (i = 0; i < __okl4_environ->len; i++) {

        for (j = 0; ; j++) {
            if (_toupper(name[j]) !=
                    _toupper(__okl4_environ->env_item[i].name[j])) {
                break;
            } else if (name[j] == '\0') {
                return (__okl4_environ->env_item[i].item);
            } else {
                continue;
            }
        }
    }
    return NULL;
}
#endif

    root_thread = root_kspace->kthread_list;
    root_kclist = root_kspace->kclist;

    printf("\n--- This function will do 1 to 25 adder ---\n");

	/* setup each thread env */
    for (i = 0; i < THREADS; i++)
    {     
   	 /* The okl4_kthread_attr structure represents the kthread
	 *  attribute used to create kthreads.*/
        okl4_kthread_attr_t kthread_attr;
#if 0
struct okl4_kthread_attr {
    okl4_word_t sp;
    okl4_word_t ip;
    okl4_word_t priority;
	
    okl4_kspace_t *kspace;
	
    okl4_kcap_t pager;

    okl4_utcb_item_t *utcb_item;	
    okl4_kcap_item_t *kcap_item;
};
#endif 

        utcb_item[i] = malloc(sizeof(struct okl4_utcb_item));


        error = okl4_utcb_allocany(root_kspace->utcb_area, utcb_item[i]);

        kcap_item[i] = malloc(sizeof(struct okl4_kcap_item));
        error = okl4_kclist_kcap_allocany(root_kclist, kcap_item[i]);


        stacks[i] = malloc(STACK_SIZE);
	
        okl4_kthread_attr_init(&kthread_attr);

		okl4_kthread_attr_setspip(&kthread_attr,
                (okl4_word_t)stacks[i] + STACK_SIZE,
                (okl4_word_t)(adder));
	
        okl4_kthread_attr_setspace(&kthread_attr, root_kspace);
        okl4_kthread_attr_setutcbitem(&kthread_attr, utcb_item[i]);
        okl4_kthread_attr_setcapitem(&kthread_attr, kcap_item[i]);
		
        error = okl4_kthread_create(&threads[i], &kthread_attr);
        caps[i] = okl4_kthread_getkcap(&threads[i]);
	    okl4_kthread_start(&threads[i]);
    }

    // Message child threads to start 
    
    for (i = 0; i < THREADS; i++)
    {
        args.id = i;
        args.count = 0;
        args.parent = okl4_kthread_getkcap(root_thread);
	    args.temp = 0;
        error = okl4_message_send(caps[i], &args, sizeof(args));
    }

    // Wait for the child threads to finish
    for (i = 0; i < THREADS; i++)
    {
        error = okl4_message_wait(&args.temp, sizeof(args.temp), NULL, NULL);
        ans = ans + args.temp;
    }

    printf("Total ans is %d \n",ans);
    printf("adder complete. Exiting...\n");
    /* Finished */
}

void
adder(void)
{

    int error;
    struct args args;
	
    okl4_init_thread();

    error = okl4_message_wait(&args, sizeof(args), NULL, NULL);

	for (args.count = args.id*5+1;args.count<6+args.id*5;args.count++)
	{
	args.temp = args.temp + args.count;
	}

    error = okl4_message_send(args.parent, &args.temp , sizeof(args.temp));
}

