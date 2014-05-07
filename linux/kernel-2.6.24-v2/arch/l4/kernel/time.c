#include "assert.h"

#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/rcupdate.h>
#include <linux/module.h>

#include <l4/thread.h>

extern void interrupt_loop(void);
void show_state(void);

extern L4_ThreadId_t timer_thread;
extern L4_ThreadId_t timer_handle;

extern void (*late_time_init)(void);
static void __init timer_start(void);


extern void mxc_timer_init(void);
extern void pxa_timer_init(void);
extern void versatile_timer_init(void);

void __init
time_init(void)
{
#if defined(CONFIG_CELL)

#if defined(CONFIG_PXA)
	/* FIXME -PXA only for now - should use initcall() or similar -cvs */
	pxa_timer_init();
#elif defined(CONFIG_KZM)
	/* FIXME -KZM only for now - should use initcall() or similar -gl */
	mxc_timer_init();
#elif defined(CONFIG_VERSATILE)
	versatile_timer_init();
#else
#error
#endif

#endif

	late_time_init = timer_start;
}


/* XXX timer_init_sysfs?? */

static void __init
timer_start(void)
{
	/* Send message to irq thread -- wake it up */
	L4_Msg_t        msg;
	L4_MsgTag_t     tag;
	L4_ThreadId_t   should_be_timer;

	L4_MsgClear(&msg);
	L4_MsgLoad(&msg);

	L4_Wait(&should_be_timer);
	timer_handle.raw = should_be_timer.raw;

	tag = L4_Send(timer_thread);

	assert(!L4_IpcFailed(tag));
	return;
}
