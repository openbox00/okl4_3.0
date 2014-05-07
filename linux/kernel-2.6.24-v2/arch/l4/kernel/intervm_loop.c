/*
 * linux/arch/l4/kernel/intervm_loop.c
 */

#include <l4.h>
#include <linux/kernel.h>
#include <fs/fs.h>
#include <iguana/memsection.h>
#include <iguana/thread.h>
#include <iguana/env.h>
#include <linux/sched.h>

#include <assert.h>

struct blocked_linux_thread {                 
    struct task_struct *task;                 
    STAILQ_ENTRY(blocked_linux_thread) links; 
};                                            

extern L4_ThreadId_t intervm_thread;

extern STAILQ_HEAD(lrqhead, blocked_linux_thread) okl4fs_read_queue;
extern STAILQ_HEAD(lwqhead, blocked_linux_thread) okl4fs_write_queue;

extern int notified;

void
intervm_loop(void)
{
    L4_Word_t mask;
    L4_MsgTag_t tag;
    struct control_block *cb;
    uintptr_t u;
    memsection_ref_t ms;
    const envitem_t *e;
    struct blocked_linux_thread *blt;

    e = iguana_getenv("INTERVM_FWD");   
    if (e == NULL)
        goto error;
    ms = env_memsection(e);    
    u = (uintptr_t)memsection_base(ms);
    assert(u != NULL);

    u += sizeof(int);
    cb = (struct control_block *)u;
    cb->linux_thread.raw = intervm_thread.raw;
    cb->mask = 0xa;

    L4_Set_NotifyMask(cb->mask);
    L4_Accept(L4_NotifyMsgAcceptor);

    while (1) {
        tag = L4_WaitNotify(&mask);

        if (!STAILQ_EMPTY(&okl4fs_read_queue)) {
            blt = STAILQ_FIRST(&okl4fs_read_queue);
            STAILQ_REMOVE(&okl4fs_read_queue, blt, blocked_linux_thread, links);
            notified = 1;
            wake_up_process(blt->task);
        }
        if (!STAILQ_EMPTY(&okl4fs_write_queue)) {
            blt = STAILQ_FIRST(&okl4fs_write_queue);
            STAILQ_REMOVE(&okl4fs_write_queue, blt, blocked_linux_thread, links);
            notified = 1;
            wake_up_process(blt->task);
        }
    }

error:
    thread_delete_self();

    return;
}
