/*
 * Description: Services provided by the platform.
 */

#ifndef __PLATFORM__VERSATILE_PLATFORM_H__
#define __PLATFORM__VERSATILE_PLATFORM_H__

extern addr_t versatile_io_vbase;
extern addr_t versatile_sctl_vbase;
extern addr_t versatile_timer0_vbase;
extern addr_t versatile_vic_vbase;
extern addr_t versatile_sic_vbase;
extern addr_t versatile_uart0_vbase;

void handle_timer_interrupt(bool wakeup, continuation_t cont) NORETURN;
void init_clocks(void);

#endif /* __PLATFORM__VERSATILE_PLATFORM_H__ */
