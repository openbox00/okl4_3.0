/*
 * Description:   Periodic timer handling
 */

#include <soc/soc.h>
#include <soc/interface.h>
#include <interrupt.h>
#include <reg.h>
#include <soc.h>

addr_t versatile_timer0_vbase;
addr_t versatile_timer1_vbase;
addr_t versatile_sctl_vbase;

//#define TRACE_TIMER SOC_TRACEF
#define TRACE_TIMER(x...)

unsigned long soc_get_timer_tick_length(void)
{
 //   volatile Timer_t    *timer0 = (Timer_t *)VERSATILE_TIMER0_VBASE;
    volatile Timer_t    *timer1 = (Timer_t *)VERSATILE_TIMER1_VBASE;

	return timer1->val ;
//    return TIMER_TICK_LENGTH;
}

void handle_timer_interrupt(bool wakeup, continuation_t continuation)
{
    volatile Timer_t    *timer0 = (Timer_t *)VERSATILE_TIMER0_VBASE;
    TRACE_TIMER("irq:%d context:%t\n", irq, context);

    /* clear interrupt by writing any value to clear register
     *
     * This used to be after the handle timer interrupt call, but this produced
     * broken behaviour, in that a high priority thread could miss it's timeslice
     * by the interrupt being reserviced before it ran.
     */
    timer0->clear = ~(0UL);

    kernel_scheduler_handle_timer_interrupt(wakeup,
                                            TIMER_TICK_LENGTH,
                                            continuation);
}

void init_clocks(void)
{
    volatile Timer_t    *timer0 = (Timer_t *)VERSATILE_TIMER0_VBASE;
    volatile Timer_t    *timer1 = (Timer_t *)VERSATILE_TIMER1_VBASE;
    volatile SysCtl_t   *sysctl = (SysCtl_t *)VERSATILE_SCTL_VBASE;

    /*
     * set clock frequency:
     *      VERSATILE_REFCLK is 32KHz
     *      VERSATILE_TIMCLK is 1MHz
     */
    sysctl->ctrl = VERSATILE_CLOCK_MODE_NORMAL |
                    (1UL << VERSATILE_BIT_TIMER_EN0_SEL) |
                    (1UL << VERSATILE_BIT_TIMER_EN1_SEL) |
                    (1UL << VERSATILE_BIT_TIMER_EN2_SEL) |
                    (1UL << VERSATILE_BIT_TIMER_EN3_SEL);

    /* Enable 5ms L4 system tick */
    timer0->load = VERSATILE_TIMER_RELOAD;
    TRACE_TIMER("Timer reload val:0x%08x\n", VERSATILE_TIMER_RELOAD);

    /* enable irq.*/
    soc_unmask_irq(VERSATILE_TIMER0_IRQ);

    /* start timer - enable + clkdiv + periodic + int enable */
    timer0->ctrl = VERSATILE_TIMER_CTRL | VERSATILE_TIMER_MODE | VERSATILE_TIMER_IE;
    TRACE_TIMER("Timer ctrl val:0x%08x\n", timer0->ctrl);

    /* Enable 1MHz free-running timer */
    timer1->ctrl = VERSATILE_TIMER_CTRL | VERSATILE_TIMER_32BIT;
    timer1->clear = 0;

}
