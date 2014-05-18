/*
 * Description: Generic L4 init code
 */
#include <l4.h>
#include <debug.h>

/**
 * Entry point from ASM into C kernel
 * Precondition: MMU and page tables are disabled
 * Warning: Do not use local variables in startup_system()
 */

#include <tcb.h>
#include <smp.h>
#include <schedule.h>
#include <space.h>
#include <arch/memory.h>
#include <config.h>
#include <arch/hwspace.h>
#include <kernel/generic/lib.h>
#include <soc/soc.h>
#include <mutex.h>
#include <clist.h>


#include <mp.h>
mp_t mp;

void idle_thread();
void init_all_threads();
#if defined(ARCH_MIPS64)
extern void SECTION (SEC_INIT) finalize_cpu_init (word_t cpu_id);
#endif


void __panic(void)
{
#ifndef CONFIG_DEBUG
    /* Reboot if no debugger is present. */
    soc_panic();
#else
    /* Enter KDB */
    enter_kdebug("panic");
#endif

    /* Loop just in case the reboot returns. */
    for (;;);
}

void SECTION(SEC_INIT) create_idle_thread()
{
    bool r;
    r = get_idle_tcb()->create_kernel_thread(get_idle_utcb());
    ASSERT(DEBUG, r);

    r = get_idle_tcb()->grab();
    ASSERT(DEBUG, r);
    get_idle_tcb()->initialise_state(thread_state_t::running);
    get_idle_tcb()->effective_prio = -1;
    get_idle_tcb()->base_prio = -1;
    /* set idle thread timeslice to infinite */
    (void)get_current_scheduler()->do_schedule(get_idle_tcb(), 0, ~0UL, 0);
    get_idle_tcb()->release();

#if defined(CONFIG_THREAD_NAMES)
    strcpy(get_idle_tcb()->debug_name, "idle");
#if defined(CONFIG_MUNITS)
    {
        int unit = get_current_context().unit;
        int chars = unit / 10;
        int i;
        for (i = chars; i >= 0; i --) {
            get_idle_tcb()->debug_name[4 + 1] = '0' + unit % 10;
            unit /= 10;
        }
        get_idle_tcb()->debug_name[5 + chars] = 0;
    }
#endif
#endif
}


#if defined(CONFIG_MUNITS)
/* We set this flag here, since now we are running on the idle thread's (unique) stack
 * and this avoids a race where the next context is brought up and starts using the boot
 * stack before we are finished with it.
 */
void SECTION(SEC_INIT) context_stub(void)
{
    get_mp()->context_initialized = true;
    ACTIVATE_CONTINUATION(idle_thread);
}

/* This is the C entry-point after arch specific
 * assembler routines bootstrap a context  */
extern "C" void SECTION (SEC_INIT) startup_context (void)
{
    cpu_context_t context = get_current_context();

    create_idle_thread();
    get_idle_tcb()->notify(context_stub);

    init_xcpu_handling (context);

    //get_current_scheduler()->init (false);
    get_current_scheduler()->start (context);

    /* make sure we don't fall off the edge */
    spin_forever(1);
}
#endif

/**
 * Setup system-wide scheduling.
 */
static NORETURN void SECTION(SEC_INIT)
startup_scheduler()
{
#if defined(CONFIG_SPACE_NAMES)
    strcpy(get_kernel_space()->debug_name, "kernel");
#endif

#if defined (CONFIG_MDOMAINS) || defined (CONFIG_MUNITS)
    TRACE_INIT("Initialising multiprocessing support...\n");
#endif

    /* initialise the mp class */
    get_mp()->probe();
    get_mp()->print_info();

#if defined(CONFIG_MDOMAINS) || defined(CONFIG_MUNITS)
    init_xcpu_handling (get_current_context());
#endif

#if 0
	word_t a;
	a = soc_get_timer_tick_length();
	printf("--------------------------------------------%d\n",a);


	word_t b;
	b = soc_get_timer_tick_length();
	printf("--------------------------------------------%d\n",b);
#endif 

    TRACE_INIT("Initialising scheduler...\n");
    get_current_scheduler()->init(false);


    create_idle_thread();
    get_idle_tcb()->notify (init_all_threads);

#if defined(CONFIG_MDOMAINS) || defined(CONFIG_MUNITS)
    /* initialize other processors */
    TRACE_INIT("Initialising other contexts...\n");
    get_mp()->init_other_contexts();
    TRACE_INIT("Finished initialising other contexts\n");
#endif

    /* get the thing going - we should never return */
    get_current_scheduler()->start(get_current_context());

    NOTREACHED();
}

/**
 * Perform architecture independent system-wide initialisation.
 *
 * At this point, all per-architecture initialisation has been performed.
 *
 * This function does not return.
 */
NORETURN void SECTION(SEC_INIT)
generic_init(void)
{
    /* Startup the scheduler, and begin to schedule threads. Does not
     * return. */
    startup_scheduler();

    NOTREACHED();
}
