/*
 * Description:   generic IPC path
 */
#include <l4.h>
#include <debug.h>
#include <kdb/tracepoints.h>

#define DO_TRACE_IPC(x...) do { printf(x); } while(0)

#if 1
#define TRACE_IPC(x...)
#define TRACE_XIPC(x...)
#define TRACE_NOTIFY(x...)
#else

#define TRACE_IPC       DO_TRACE_IPC
#define TRACE_XIPC      DO_TRACE_IPC
#define TRACE_NOTIFY    DO_TRACE_IPC
#endif

#include <tcb.h>
#include <schedule.h>
#include <ipc.h>
#include <arch/ipc.h>
#include <syscalls.h>
#include <smp.h>
#include <arch/syscalls.h>
#include <profile.h>

#include <soc/soc.h>

#define testipc 1
#define node	1
#define lab		1

int test = 1;

DECLARE_TRACEPOINT(SYSCALL_IPC);
DECLARE_TRACEPOINT(IPC_TRANSFER);


INLINE bool transfer_message(tcb_t * src, tcb_t * dst)
{
    TRACEPOINT (IPC_TRANSFER,
                printf("IPC transfer message: src=%t, dst=%t\n", src, dst));

    msg_tag_t tag = src->get_tag();

    // clear all flags
    tag.clear_receive_flags();

    // we set the sender space id here
    dst->set_sender_space(src->get_space_id());

    /*
     * Any errors here will be reported as a message overflow error.
     * For the "exception" IPCs this is misleading as more than just overflow
     * is checked.  A new error type should be defined sometime.
     */
    if (EXPECT_FALSE(src->in_exception_ipc())) {
        if (! src->copy_exception_mrs_from_frame(dst)) {
            goto error;
        }
    } else if (EXPECT_FALSE(dst->in_exception_ipc())) {
        if (! src->copy_exception_mrs_to_frame(dst)) {
            goto error;
        }
    } else if (EXPECT_TRUE(tag.get_untyped())){
        if (EXPECT_FALSE(!src->copy_mrs(dst, 1, tag.get_untyped()))) {
            goto error;
        }
    }

    dst->set_tag(tag);
    return true;

error:
    // Report message overflow error
    dst->set_error_code (IPC_RCV_ERROR (ERR_IPC_MSG_OVERFLOW));
    src->set_error_code (IPC_SND_ERROR (ERR_IPC_MSG_OVERFLOW));

    tag.set_error();
    dst->set_tag(tag);
    return false;
}

INLINE void setup_notify_return(tcb_t *tcb)
{
    word_t mask = tcb->get_notify_mask();
    word_t bits = tcb->sub_notify_bits(mask);

    tcb->set_tag(msg_tag_t::notify_tag());
    tcb->set_mr(1, bits & mask);
}

/*
 * Enqueue 'to_tcb' (if it exists) and return from the IPC.
 */
NORETURN static void
enqueue_tcb_and_return(tcb_t *current, tcb_t *to_tcb,
        continuation_t continuation)
{
    scheduler_t * scheduler = get_current_scheduler();
    if (to_tcb) {
        scheduler->activate_sched(to_tcb, thread_state_t::running,
                                  current, continuation,
                                  scheduler_t::sched_default);
    } else {
        ACTIVATE_CONTINUATION(continuation);
    }
}

/**
 * Restart the IPC operation as destination thread is now ready to receive
 * Threads that block on sending (in polling state) have their continuation
 * set to this function
 *
 * This is a continuation function so it's parameters are in the TCB
 *
 * @param ipc_to The to thread id of the IPC
 * @param ipc_from The from thread id of the IPC
 */
static CONTINUATION_FUNCTION(restart_ipc)
{
    tcb_t * current = get_current_tcb();

    SYS_IPC_RESTART(TCB_SYSDATA_IPC(current)->to_tid,
                    TCB_SYSDATA_IPC(current)->from_tid,
                    TCB_SYSDATA_IPC(current)->ipc_return_continuation);
}


/**
 * This function checks whether the received message was an async IPC
 * and does the appropriate set up for return if it was.
 *
 * It then returns through TCB_SYSDATA_IPC(current)->ipc_return_continuation function
 */
extern "C" CONTINUATION_FUNCTION(check_async_ipc)
{
    tcb_t * current = get_current_tcb();
    if (EXPECT_TRUE(current->sent_from.is_nilthread() &&
                    (!current->get_tag().is_error())))
    {
        TRACE_IPC("Asynch notify wakeup\n");
        setup_notify_return(current);
        ACTIVATE_CONTINUATION(TCB_SYSDATA_IPC(current)->ipc_return_continuation);
    }

    /* XXX VU: restructure switching code so that dequeueing
     * from wakeup is removed from critical path */
    ACTIVATE_CONTINUATION(TCB_SYSDATA_IPC(current)->ipc_return_continuation);
}

/**
 * Handle IPC errors
 */
static void NORETURN
return_ipc_error_send(tcb_t *current, tcb_t *to_tcb, tcb_t* from_tcb, word_t error)
{
    current->set_error_code(IPC_SND_ERROR(error));
    current->set_tag(msg_tag_t::error_tag());
    current->sent_from = NILTHREAD;

    if (to_tcb) { to_tcb->unlock_read(); }
    if (from_tcb) { from_tcb->unlock_read(); }
    return_ipc();
}

/**
 * Handle IPC errors with wakeup to_tcb
 */
static void NORETURN
return_ipc_error_recv(tcb_t *current, tcb_t *to_tcb, tcb_t* from_tcb, word_t error)
{
    current->set_error_code(IPC_RCV_ERROR(error));
    current->set_tag(msg_tag_t::error_tag());
    current->sent_from = NILTHREAD;

    if (from_tcb) { from_tcb->unlock_read(); }
    enqueue_tcb_and_return(current, to_tcb,
            TCB_SYSDATA_IPC(current)->ipc_return_continuation);
}

 /**********************************************************************
 *
 *                          IPC syscall
 *
 **********************************************************************/
SYS_IPC(capid_t to_tid, capid_t from_tid)
{
    tcb_t * current = get_current_tcb();
    continuation_t continuation = ASM_CONTINUATION;

    tcb_t * to_tcb = NULL;
    tcb_t * from_tcb = NULL;

    /* setup IPC restart/return info */
    current->sys_data.set_action(tcb_syscall_data_t::action_ipc);
    TCB_SYSDATA_IPC(current)->from_tid = from_tid;
    TCB_SYSDATA_IPC(current)->to_tid = to_tid;
    TCB_SYSDATA_IPC(current)->ipc_restart_continuation = restart_ipc;
    TCB_SYSDATA_IPC(current)->ipc_return_continuation = continuation;

    /* Decode send-phase thread */
    if (to_tid.is_threadhandle())
    {
        /*
         * sending phase of ipc that uses thread handle,
         * to-thread MUST be waiting for me.
         */
		printf("IPC get thread handle 0x%u\n", to_tid.get_raw());	
        to_tcb = lookup_tcb_by_handle_locked(to_tid.get_raw());

        if (EXPECT_FALSE(to_tcb == NULL))
        {
            /* specified thread id invalid */
			printf("invalid send thread handle\n");			 
            return_ipc_error_send(current, NULL, NULL, ERR_IPC_NON_EXISTING);
            NOTREACHED();
        }

        /* Check that if the to_thread is waiting for me. */
        if (EXPECT_FALSE(!to_tcb->get_state().is_waiting()
                         || !to_tcb->is_partner_valid()
                         || to_tcb->get_partner() != current))
        {
			printf("dest not ready or it's partner isn't me\n");
            return_ipc_error_send(current, to_tcb, NULL, ERR_IPC_NOPARTNER);
        }
    }
    else if (to_tid.is_myself()) {
        to_tcb = acquire_read_lock_current(current);

        if (EXPECT_FALSE(to_tcb == NULL))
        {
            /* someone else is likely trying to delete us */
            printf("someone else is likely trying to delete us\n");
            return_ipc_error_send(current, NULL, NULL, ERR_IPC_NON_EXISTING);
        }
    }
    else if (EXPECT_TRUE(!to_tid.is_nilthread()))
    {
        to_tcb = get_current_clist()->lookup_ipc_cap_locked(to_tid);

        if (EXPECT_FALSE(to_tcb == NULL))
        {
            /* specified thread id invalid */
            printf("specified thread id invalid\n");
            return_ipc_error_send(current, NULL, NULL, ERR_IPC_NON_EXISTING);
        }
    }

    /* Decode receive-phase thread */
    word_t wait_type = 0;

    /* Is thread blocking on its self? */
    if (EXPECT_FALSE(from_tid.is_nilthread())) {
    }
    else if (EXPECT_FALSE(from_tid.is_myself())) {
        from_tcb = acquire_read_lock_current(current, to_tcb);
    }
    else if (from_tid.is_anythread()) {
        wait_type = 1;
    }
    else if (EXPECT_FALSE(from_tid.is_waitnotify())) {
        wait_type = 2;
    }
    else {
        from_tcb = get_current_clist()->lookup_ipc_cap_locked(from_tid, to_tcb);
        wait_type = 3;
    }

    ipc(to_tcb, from_tcb, wait_type);
}

/**
 * Kernel internal IPC implementation.
 *
 * @param to_tcb        Destination thread to send message to, must be NULL
 *                      or pre-locked.
 * @param from_tcb      Thread to receive from after send-phase, must be
 *                      NULL or pre-locked.
 * @param continuation  Return continuation.
 */
void ipc(tcb_t *to_tcb, tcb_t *from_tcb, word_t wait_type)
{
#if testipc
	printf("\n---IPC---%d\n",test);
	test++;	
	unsigned long  a;
	a = soc_get_timer_tick_length();
	//printf("a = %u\n",a);
#endif

    tcb_t * current = get_current_tcb();
    scheduler_t * scheduler = get_current_scheduler();
    bool recv_blocks;

    /* --- send phase --------------------------------------------------- */
#if lab
	printf("-----send phase start-----\n");
#endif
    /* FIXME: Async IPC assumes to_tcb is in same domain */
    if (EXPECT_TRUE(to_tcb != NULL))
    {
#if node	
		printf("1.(to_tcb != NULL)\n");
#endif		
        msg_tag_t tag = current->get_tag ();
        recv_blocks = tag.recv_blocks();
        capid_t sender_handle = threadhandle(current->tcb_idx);

        // not waiting || (not waiting for me && not waiting for any)
        // optimized for receive and wait any
        if (EXPECT_FALSE(
                    !to_tcb->get_state().is_waiting()
                    || (to_tcb->get_partner() != current &&
                      ((word_t)to_tcb->get_partner() != ANYTHREAD_RAW)) ))
        {
#if node		
			printf("2.not waiting || (not waiting for me && not waiting for any)\n");
#endif			
            // eagerly set these to allow atomic locking
            current->set_partner(to_tcb);
            to_tcb->get_endpoint()->enqueue_send(current);

            /* Schedule somebody else while we wait for the receiver to become
             * ready. */
            to_tcb->unlock_read();
            if (from_tcb) { from_tcb->unlock_read(); }
#if testipc
/*bb***********************************************************/
			unsigned long  b;
			b = soc_get_timer_tick_length();
			//printf("b exit\n");
			//printf("scheduler->deactivate_sched(send_phase)\n");
			printf("---(a-b) total = %u---\n",(a-b));
#if lab
			printf("-----set thread_state polling and exit-----\n");
#endif
/***********************************************************/
#endif
            scheduler->deactivate_sched(current, thread_state_t::polling, current,
                                 TCB_SYSDATA_IPC(current)->ipc_restart_continuation,
                                 scheduler_t::sched_default);
        }
        else
        {
#if node		
			printf("3.(waiting && waiting for me) || waiting for any\n");
#endif			
            to_tcb->unlock_read();
            to_tcb->remove_dependency();

            /* Mark our partner as an invalid value, to ensure that our partner
             * can't send to us twice with a reply cap. */
            to_tcb->set_partner((tcb_t*)INVALID_RAW);

            /* set sent_from to be thread handle of the sender. */
            to_tcb->sent_from = sender_handle;
        }
	
        if (EXPECT_FALSE(!transfer_message(current, to_tcb)))
        {	
        }		
#if node		
		printf("transfer_message\n");
#endif	
    }
    else        /* to_tcb == NULL */
    {
#if node	
		printf("4.(to_tcb == NULL)\n");
#endif		
        recv_blocks = current->get_tag().recv_blocks();
        current->set_tag(msgtag(0));
    }
#if lab
	printf("-----send phase finished-----\n");
#endif	
receive_phase:
    /* --- receive phase ------------------------------------------------ */
    /* from_tcb != NULL */
#if lab
	printf("-----receive phase start-----\n");
#endif
        /*
         * Optimisation for Call(), i.e., when to == from.
         *
         * This allows us to avoid a whole number of checks, because we
         * know that from_tcb is not waiting for us. This block of code
         * does not affect correctness, but gives a performance boost
         * for the Call() case.
         */
        if (to_tcb && (to_tcb == from_tcb) && recv_blocks) {
#if node		
	     printf("5.(to_tcb && (to_tcb == from_tcb) && recv_blocks)\n");
#endif		 
            current->set_partner(from_tcb);
            to_tcb->get_endpoint()->enqueue_recv(current);
            /* to_tcb already unlocked */
            from_tcb->unlock_read();
#if testipc
/*cc***********************************************************/
			unsigned long  c;
			c = soc_get_timer_tick_length();
			//printf("c exit\n");
			//printf("scheduler->deactivate_activate_sched(receive phase)\n");
			printf("---(a-c) total = %u---\n",(a-c));
#if lab
			printf("-----receive phase finished-----\n");
#endif
/**************************************************************/
#endif
            scheduler->deactivate_activate_sched(current, to_tcb,
                    thread_state_t::waiting_forever, thread_state_t::running,
                    current, check_async_ipc,
                    scheduler_t::sched_default);
        }

        acceptor_t acceptor;
        acceptor.clear();

        /* VU: optimize for common case -- any, closed */
        if (wait_type == 1)
        {
retry_get_head:
#if node
			printf("6.retry_get_head:\n");
#endif			
            from_tcb = current->get_endpoint()->get_send_head();

            if (EXPECT_TRUE(from_tcb)) {
#if node			
				printf("(EXPECT_TRUE(from_tcb))\n");
#endif				
                if (EXPECT_FALSE(!from_tcb->try_lock_read())) {
					printf("go to  retry_get_head\n");
                    goto retry_get_head;
                }
                /* Don't pick a different thread if we loop back through this code */
                wait_type = 3;
#if node
				printf("wait_type = %d,  Don't pick a different thread if we loop back through this code\n",wait_type);
#endif	
            }
            /* only accept notify bits if waiting from anythread */
            acceptor = current->get_acceptor();
        }

        /*
         * no partner || partner is not polling ||
         * partner doesn't poll on me
         */
        if( EXPECT_TRUE ( (from_tcb == NULL) ||
                          (!from_tcb->get_state().is_polling()) ||
                          (from_tcb->get_partner() != current)))
        {
#if node		
			printf("7.no partner || partner is not polling || partner doesn't poll on me\n");
#endif			
            /* Update schedule inheritance dependancy on closed waits. */
            if (from_tcb != NULL) {
#if node			
				printf("8.(from_tcb != NULL) doesn't enter\n");
#endif				
            } else {
			//	printf("9.(from_tcb == NULL)\n");
			//	printf("wait_type = %d\n",wait_type);
                current->set_partner(wait_type == 1 ? (tcb_t*)ANYTHREAD_RAW : NULL);
            }

            /* Find somebody else to schedule. */
#if node			
			printf("11.to_tcb == NULL\n");
#endif			
#if testipc
/*dd*******************************************/
			unsigned long  d;
			d = soc_get_timer_tick_length();
			//printf("d exit\n");
			//printf("scheduler->deactivate_sched(receive phase)\n");
			printf("---(a-d) total = %u---\n",(a-d));
#if lab
			printf("-----receive phase finished-----\n");
#endif
/***********************************************/
#endif	
                scheduler->deactivate_sched(current, thread_state_t::waiting_forever,
                                     current, check_async_ipc,
                                     scheduler_t::sched_default);
        }
        else
        {
#if node		
			printf("12.have partner && partner is polling && partner poll on me\n");
#endif			
            if (EXPECT_FALSE(!from_tcb->grab())) {
                goto receive_phase;
            }
            from_tcb->unlock_read();
            // both threads on the same CPU?
/*test********************************************/
            current->set_partner(from_tcb);

            current->get_endpoint()->dequeue_send(from_tcb);


/*******************************************/
			/* We need to perform a context switch here, and not a schedule.
			 * This is because 'from_tcb' is responsible for finishing the
			 * copy, and we (a potentially high priority thread) can not be
			 * scheduled until that is done. 'from_tcb' will perform a
			 * schedule when the copy is done, ensuring that the scheduler
			 * is still in control. */
#if testipc
/*ee********************************************/
			unsigned long  e;
			e = soc_get_timer_tick_length();
			//printf("e exit\n");
			//printf("scheduler->context_switch(receive phase)\n");
			printf("---(a-e) total = %u---\n",(a-e));
#if lab
			printf("-----receive phase finished-----\n");
#endif
/************************************************/
#endif
			scheduler->context_switch(current, from_tcb, thread_state_t::waiting_forever,
                        thread_state_t::running,
                        TCB_SYSDATA_IPC(current)->ipc_return_continuation);
        }
}
