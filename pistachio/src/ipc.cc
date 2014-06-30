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
#define testaaa 0

int test = 1;

DECLARE_TRACEPOINT(SYSCALL_IPC);
DECLARE_TRACEPOINT(IPC_TRANSFER);


INLINE bool transfer_message(tcb_t * src, tcb_t * dst)
{

    msg_tag_t tag = src->get_tag();

    // clear all flags
    tag.clear_receive_flags();

    // we set the sender space id here
    dst->set_sender_space(src->get_space_id());
#if testaaa
	unsigned long  g;
	g = soc_get_timer_tick_length();
#endif
        if (EXPECT_FALSE(!src->copy_mrs(dst, 1, tag.get_untyped()))) {
        }

#if testaaa
	unsigned long  z;
	z = soc_get_timer_tick_length();
	printf("------------------------------------------------copy_mrs = %u---\n",(g-z));
#endif


    dst->set_tag(tag);



    return true;
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
        PROFILE_STOP(sys_ipc_e);
        ACTIVATE_CONTINUATION(TCB_SYSDATA_IPC(current)->ipc_return_continuation);
    }

    TRACE_IPC("%t received msg\n", current);

    /* XXX VU: restructure switching code so that dequeueing
     * from wakeup is removed from critical path */
    PROFILE_STOP(sys_ipc_e);
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
    PROFILE_STOP(sys_ipc_e);
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
    PROFILE_STOP(sys_ipc_e);
    enqueue_tcb_and_return(current, to_tcb,
            TCB_SYSDATA_IPC(current)->ipc_return_continuation);
    NOTREACHED();
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
        TRACE_IPC("IPC get thread handle 0x%lx\n", to_tid.get_raw());
        to_tcb = lookup_tcb_by_handle_locked(to_tid.get_raw());

        if (EXPECT_FALSE(to_tcb == NULL))
        {
            /* specified thread id invalid */
            TRACE_NOTIFY("invalid send thread handle, %lx\n",
                         to_tid.get_raw() );
            return_ipc_error_send(current, NULL, NULL, ERR_IPC_NON_EXISTING);
            NOTREACHED();
        }

        /* Check that if the to_thread is waiting for me. */
        if (EXPECT_FALSE(!to_tcb->get_state().is_waiting()
                         || !to_tcb->is_partner_valid()
                         || to_tcb->get_partner() != current))
        {
            TRACE_IPC ("dest not ready or it's partner isn't me (%t, is_wt=%d, state=%d, to_tcb->partner=0x%lx)\n",
                    to_tcb, to_tcb->get_state().is_waiting(), (word_t) to_tcb->get_state(),
                    to_tcb->is_partner_valid() ? to_tcb->get_partner() : NULL);
            return_ipc_error_send(current, to_tcb, NULL, ERR_IPC_NOPARTNER);
            NOTREACHED();
        }
    }
    else if (to_tid.is_myself()) {
        to_tcb = acquire_read_lock_current(current);

        if (EXPECT_FALSE(to_tcb == NULL))
        {
            /* someone else is likely trying to delete us */
            TRACE_IPC("pending delete, TCB %p\n", current);
            return_ipc_error_send(current, NULL, NULL, ERR_IPC_NON_EXISTING);
            NOTREACHED();
        }
    }
    else if (EXPECT_TRUE(!to_tid.is_nilthread()))
    {
        to_tcb = get_current_clist()->lookup_ipc_cap_locked(to_tid);

        if (EXPECT_FALSE(to_tcb == NULL))
        {
            /* specified thread id invalid */
            TRACE_IPC("invalid send tid, %t\n", to_tid.get_raw() );
            return_ipc_error_send(current, NULL, NULL, ERR_IPC_NON_EXISTING);
            NOTREACHED();
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
    PROFILE_START(sys_ipc_e);

#if testipc
	printf("---IPC---%d\n",test);
	test++;	
	unsigned long  a;
	a = soc_get_timer_tick_length();
	printf("a = %u\n",a);
#endif

    tcb_t * current = get_current_tcb();
    scheduler_t * scheduler = get_current_scheduler();
    bool recv_blocks;

    /* --- send phase --------------------------------------------------- */

    /* FIXME: Async IPC assumes to_tcb is in same domain */
    if (EXPECT_TRUE(to_tcb != NULL))
    {
#if testaaa
		printf("1. --------\n");
#endif
	    msg_tag_t tag = current->get_tag ();

        recv_blocks = tag.recv_blocks();
        capid_t sender_handle = threadhandle(current->tcb_idx);



//check_waiting:
        if (EXPECT_FALSE(
                    !to_tcb->get_state().is_waiting()
                    || (to_tcb->get_partner() != current &&
                      ((word_t)to_tcb->get_partner() != ANYTHREAD_RAW)) ))
        {
#if testaaa
		printf("2. --------\n");
#endif
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
			printf("b = %u\n",b);
			//printf("scheduler->deactivate_sched(send_phase)\n");
			printf("---total = %u---\n",(a-b));
/***********************************************************/
#endif
            PROFILE_STOP(sys_ipc_e);
            scheduler->
                deactivate_sched(current, thread_state_t::polling, current,
                                 TCB_SYSDATA_IPC(current)->ipc_restart_continuation,
                                 scheduler_t::sched_default);
            NOTREACHED();
        }
        else
        {
#if testaaa
		printf("3. --------\n");
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


    }
    else        /* to_tcb == NULL */
    {
#if testaaa
		printf("4. --------\n");
#endif
        recv_blocks = current->get_tag().recv_blocks();
        current->set_tag(msgtag(0));
    }

    /* --- send finished ------------------------------------------------ */
//receive_phase:
    {
#if testaaa
		printf("5. --------\n");
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
#if testaaa
		printf("6. --------\n");
#endif

/*******************************************************/
            current->set_partner(from_tcb);

/********************************************************/
            to_tcb->get_endpoint()->enqueue_recv(current);


            /* to_tcb already unlocked */
            from_tcb->unlock_read();
#if testipc
/*kk***********************************************************/
			unsigned long  k;
			k = soc_get_timer_tick_length();
			printf("k = %u\n",k);
			//printf("scheduler->deactivate_activate_sched(receive phase)\n");
			printf("---total = %u---\n",(a-k));
/**************************************************************/
#endif
            scheduler->deactivate_activate_sched(current, to_tcb,
                    thread_state_t::waiting_forever, thread_state_t::running,
                    current, check_async_ipc,
                    scheduler_t::sched_default);
            NOTREACHED();
        }

        acceptor_t acceptor;
        acceptor.clear();

        /* VU: optimize for common case -- any, closed */
        if (wait_type == 1)
        {
#if testaaa
		printf("7. --------\n");
#endif
retry_get_head:
            from_tcb = current->get_endpoint()->get_send_head();

            if (EXPECT_TRUE(from_tcb)) {
                if (EXPECT_FALSE(!from_tcb->try_lock_read())) {
                    goto retry_get_head;
                }
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
#if testaaa
		printf("8. --------\n");
#endif
            /* Update schedule inheritance dependancy on closed waits. */
            if (from_tcb != NULL) {
            } else {
#if testaaa
		printf("9. --------\n");
#endif 
				//printf("wait_type = %d--------\n",wait_type );
                current->set_partner(wait_type == 1 ? (tcb_t*)ANYTHREAD_RAW : NULL);
            }
#if testaaa
		printf("10. --------\n");
#endif
#if testipc
/*hh*******************************************/

			unsigned long  h;
			h = soc_get_timer_tick_length();
			printf("h = %u\n",h);
			//printf("scheduler->deactivate_sched(receive phase)\n");
			printf("---total = %u---\n",(a-h));

/***********************************************/
#endif	
                PROFILE_STOP(sys_ipc_e);
                scheduler->
                    deactivate_sched(current, thread_state_t::waiting_forever,
                                     current, check_async_ipc,
                                     scheduler_t::sched_default);
                NOTREACHED();
        }
        else
        {
#if testaaa
		printf("11. --------\n");
#endif
            from_tcb->unlock_read();
            {
#if testaaa
		printf("12. --------\n");
#endif	
                current->set_partner(from_tcb);
                current->get_endpoint()->dequeue_send(from_tcb);

#if testipc
/*ii********************************************/
			unsigned long  i;
			i = soc_get_timer_tick_length();
			printf("i = %u\n",i);
			//printf("scheduler->context_switch(receive phase)\n");
			printf("---total = %u---\n",(a-i));

/************************************************/
#endif

                PROFILE_STOP(sys_ipc_e);
                scheduler->context_switch(current, from_tcb, thread_state_t::waiting_forever,
                        thread_state_t::running,
                        TCB_SYSDATA_IPC(current)->ipc_return_continuation);
                NOTREACHED();
            }
        }
        NOTREACHED();
    }
    NOTREACHED();
}
