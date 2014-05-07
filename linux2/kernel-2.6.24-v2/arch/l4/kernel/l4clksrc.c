/*
 * l4clksrc.c
 *
 * L4-specific clocksource for OK Linux.
 *
 * Geoffrey Lee < glee at ok-labs dot com >
 *
 * This file implements an L4-specific clocksource for OK Linux.
 *
 * OPTIONS
 *
 * L4_FASTCLOCK
 *
 * In Linux reading the timer is a relatively frequent operation.
 * When running on bare metal this is fine, it's cheap.  When running
 * on L4 and using the virtual timer service it is an expensive because
 * everytime you want to do that, you need to ask the server via IPC.
 * To get around this, you can turn on the L4_FASTCLOCK feature.  In
 * this feature the clock is not read if the time is requested.  Instead,
 * every time a timeout is requested the delta value is noted and
 * added onto the current time value at every timeout.  Rather than 
 * reading the timer this last updated "current time" is returned 
 * if the timer is read.  While it is not very accurate, it does
 * does alleviate the load on the timer if timeouts are frequent.  Because
 * of the potential for abuse, this option is OFF by default.
 */

#include <linux/clockchips.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/spinlock.h>

#include <asm/smp.h>

#include <iguana/memsection.h>
#include <iguana/thread.h>
#include <iguana/hardware.h>
#include <iguana/env.h>

#include <vtimer/timer.h>
#include <interfaces/vtimer_client.h>

DEFINE_SPINLOCK(l4clksrc_lock);

extern L4_ThreadId_t	timer_thread;
extern int corba_l4_error(CORBA_Environment *env);

#if 1
#define dprintk(args...)	do {} while (0/*CONSTCOND*/)
#else
#define dprintk(args...)	printk(args)
#endif

struct l4clksrc_state {
	u64			lasttime;
	u64			rtimeo;		/* ticks - actually ns */
	u64			time;		/* cycles */
	enum clock_event_mode	mode;
	server_t		timer_server;
	device_t		timer_dev;
};

static struct l4clksrc_state l4clkstate;

static void l4clksrc_timer_init(enum clock_event_mode mode, 
				struct clock_event_device *e);
static int l4clksrc_timer_next_event(unsigned long delta, 
				struct clock_event_device *e);
static cycle_t l4clksrc_read(void);
static irqreturn_t l4clksrc_intr(int irq, void *div_id);

int __init l4clksrc_init(void);

/*
 * Program the clocksources to have a one-to-one correspondence
 * between ticks and nanoseconds.  This can be adjusted on
 * as-needed basis, but even on 32-bit systems you have
 * ~0ul / (10 ** 9) = 4 seconds before the timer counter overflows.
 */
static struct clocksource l4clksrc = {
	.name	="l4clksrc",
	.rating	= 250,
	.read	= l4clksrc_read,
	.mask	= CLOCKSOURCE_MASK(32),
	.flags	= CLOCK_SOURCE_IS_CONTINUOUS,
	.mult	= 1,
	.shift	= 0
};

static struct clock_event_device l4clksrc_clockevent = {
	.name		= "l4clksrc",
	.features	= CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.set_mode	= l4clksrc_timer_init,
	.set_next_event	= l4clksrc_timer_next_event,
	.max_delta_ns	= ~0ul,
	.min_delta_ns	= 1000,
	.mult		= 1,
	.shift		= 0,
	.irq		= 32 /* XXX magic */
};

static struct irqaction l4clksrc_action = {
	.name		= "l4-timer",
	.handler	= l4clksrc_intr,
	.flags		= IRQF_DISABLED|IRQF_NOBALANCING,
	.mask		= CPU_MASK_ALL
};

static irqreturn_t
l4clksrc_intr(int irq, void *dev_id)
{
	/* no conversion required for now since it is 1-1 */
	l4clkstate.time += l4clkstate.rtimeo;
	l4clksrc_clockevent.event_handler(&l4clksrc_clockevent);
	return IRQ_HANDLED;
}

static cycle_t
l4clksrc_read(void)
{
	dprintk("l4clksrc: l4clksrc_read\n");

#if defined(CONFIG_L4_FASTCLOCK)
	return l4clkstate.time;
#else
	{
		cycle_t tmp;
		/*
		 * Same as cycles: ns and cycles has 1-1 conversion
		 */
		tmp = virtual_timer_current_time(
			l4clkstate.timer_server,
			l4clkstate.timer_dev,
			NULL);
		/*
		 * XXX - This a workaround for the timer sometimes
		 * not being monotonically increasing when it should
		 * be.	-gl
		 */
		if (l4clkstate.lasttime > tmp)
			tmp = ++l4clkstate.lasttime;
		else
			l4clkstate.lasttime = tmp;
		return tmp;
	}
#endif
}

static void
l4clksrc_timer_init(enum clock_event_mode mode, struct clock_event_device *e)
{
	unsigned long	flags;
	char		*modestr = NULL;

	spin_lock_irqsave(&l4clksrc_lock, flags);

	switch (mode) {
		case CLOCK_EVT_MODE_PERIODIC:
			l4clkstate.mode = CLOCK_EVT_MODE_PERIODIC;
			l4clkstate.rtimeo = LATCH;
			modestr = "periodic";
			(void)virtual_timer_request(
				l4clkstate.timer_server,
				l4clkstate.timer_dev,
				LATCH,
				TIMER_PERIODIC,
				NULL);
			break;
		case CLOCK_EVT_MODE_SHUTDOWN:
		case CLOCK_EVT_MODE_UNUSED:
			l4clkstate.mode = CLOCK_EVT_MODE_SHUTDOWN;
			modestr = "shutdown";
			break;
		case CLOCK_EVT_MODE_ONESHOT:
			l4clkstate.mode = CLOCK_EVT_MODE_ONESHOT;
			modestr = "oneshot";
			break;
		case CLOCK_EVT_MODE_RESUME:
			modestr = "resume";
			break;
	}
	spin_unlock_irqrestore(&l4clksrc, flags);

	dprintk("l4clksrc: l4clksrc_timer_init mode = %s\n", modestr);
}

/*
 * Program the next event in oneshot mode.
 * 
 */
static int
l4clksrc_timer_next_event(unsigned long delta, struct clock_event_device *e)
{
	CORBA_Environment	env;
	unsigned long		flags;
	int			c;

	dprintk("l4clksrc: requesting timeout 0x%lx\n", delta);

	spin_lock_irqsave(&l4clksrc_lock, flags);
	l4clkstate.rtimeo = delta;
	(void)virtual_timer_request(
		l4clkstate.timer_server,
		l4clkstate.timer_dev,
		delta,
		TIMER_ONESHOT,
		&env);
	c = corba_l4_error(&env);
	if (c)
		panic("%s: virtual timer request failed", __func__);
	spin_unlock_irqrestore(&l4clksrc_lock, flags);
	return 0;
}

void __init
l4clksrc_timer_setup(void)
{
	l4clksrc_clockevent.cpumask = cpumask_of_cpu(smp_processor_id());
	clockevents_register_device(&l4clksrc_clockevent);
	setup_irq(32, &l4clksrc_action);
}

int __init
l4clksrc_init(void)
{
	CORBA_Environment	env;

	l4clkstate.timer_server = env_thread_id(iguana_getenv("VTIMER_TID"));
	l4clkstate.timer_dev = env_const(iguana_getenv("VTIMER_HANDLE"));

	printk("l4clksrc: Initializing\n");

	virtual_timer_init(
		l4clkstate.timer_server, 
		l4clkstate.timer_dev,
		timer_thread.raw,
		0x1,
		&env);
	if (corba_l4_error(&env))
		panic("%s: virtual_timer_init_failed", __func__);

	return clocksource_register(&l4clksrc);
}
