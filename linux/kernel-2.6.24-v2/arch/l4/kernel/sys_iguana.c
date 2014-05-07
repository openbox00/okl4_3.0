/*
 *	linux/arch/l4/kernel/sys_iguana.c
 */

#if defined(CONFIG_IGUANA)
#define timer_t	    	timer_t_linux
#endif

#include <l4.h>
#include <l4/interrupt.h>
#include <l4/utcb.h>

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/module.h>

#if defined(CONFIG_IGUANA)
#undef timer_t

#include <iguana/memsection.h>
#include <iguana/thread.h>
#include <iguana/hardware.h>
#include <iguana/env.h>

#include <vtimer/timer.h>
#include <circular_buffer/cb.h>

#endif

#if defined(CONFIG_CELL)
#include <okl4/env.h>
#include <asm/okl4.h>
#endif

#include "assert.h"
#include "irq_impl.h"

#include <linux/posix_types.h>

extern L4_ThreadId_t main_thread;
extern L4_ThreadId_t timer_thread;
extern L4_ThreadId_t timer_handle;
L4_Word_t decrypt_notify = 0;

/*
 * Mask is set (1) if enabled
 *
 * Bit 1 (irq 0) is reserved for the timer
 */
L4_Word_t irq_mask = 1;

#define NUM_CPUS 1
fd_set pending_irqs[NUM_CPUS];
#define PENDING_MASK_SIZE (NR_IRQS / 32)

/*
 * Need SMP-safe access to interrupt CSRs
 */
DEFINE_SPINLOCK(iguana_irq_lock);

static inline void
iguana_enable_irq(unsigned int irq)
{
	int ret;
	spin_lock(&iguana_irq_lock);
#if defined(CONFIG_IGUANA)
	if (irq < 32) {
#else
	if (irq != 0x34) {
#endif
		irq_mask |= (1UL << 31); /* always use bit 31 for hw irqs */
		L4_LoadMR(0, irq);
		ret = L4_AcknowledgeInterruptOnBehalf(timer_thread, 0, 0);
		if (ret == 0)
			printk("ErrorCode = %ld\n", L4_ErrorCode());
	} else {
		irq_mask |= (1UL << (irq - 46));
	}
	spin_unlock(&iguana_irq_lock);
}

static inline void
iguana_disable_irq(unsigned int irq)
{
	spin_lock(&iguana_irq_lock);
#if defined(CONFIG_IGUANA)
	if (irq < 32) {
#else
	if (irq != 0x34) {
#endif
		L4_LoadMR(0, irq);
		L4_UnregisterInterrupt(timer_thread, 0, 0);
	} else {
		irq_mask &= ~(1UL << (irq - 46));
	}

	spin_unlock(&iguana_irq_lock);
}

static unsigned int
iguana_startup_irq(unsigned int irq)
{
#if defined(CONFIG_IGUANA)
	if (irq < 32) 
		hardware_register_interrupt(timer_thread, irq);
#else
    if (irq != 0x34)
	{
		int retval;

		L4_LoadMR(0, irq);
		retval = L4_RegisterInterrupt(timer_thread, 31, 1, 0);
		if (!retval)
			printk("registerinterrupt() for irq %d failed\n", irq);
	}
#endif

	iguana_enable_irq(irq);

	return 0;
}

static void
iguana_ack_irq(unsigned int irq)
{
}

static void
iguana_end_irq(unsigned int irq)
{
	if (!(irq_desc[irq].status & (IRQ_DISABLED|IRQ_INPROGRESS)))
		iguana_enable_irq(irq);
}

static void
iguana_cpu_set_irq_affinity(unsigned int irq, cpumask_t affinity)
{
	printk("%s called\n", __func__);
}

static void
iguana_set_irq_affinity(unsigned int irq, cpumask_t affinity)
{
	spin_lock(&iguana_irq_lock);
	iguana_cpu_set_irq_affinity(irq, affinity);
	spin_unlock(&iguana_irq_lock);
}

static void __init
init_iguana_irqs(struct hw_interrupt_type * ops, int imin, int imax)
{
	long i;
	for (i = imin; i <= imax; ++i) {
		irq_desc[i].status = IRQ_DISABLED | IRQ_LEVEL;
		irq_desc[i].chip = ops;
	}
}

static struct hw_interrupt_type iguana_irq_type = {
	.typename       = "iguana",
	.startup        = iguana_startup_irq,
	.shutdown       = iguana_disable_irq,
	.enable         = iguana_enable_irq,
	.disable        = iguana_disable_irq,
	.ack            = iguana_ack_irq,
	.end            = iguana_end_irq,
	.set_affinity   = iguana_set_irq_affinity,
};

static void __init
iguana_init_irq(void)
{
	init_iguana_irqs(&iguana_irq_type, 0, NR_IRQS-1);
}

void __init
init_IRQ(void)
{
	iguana_init_irq();
}

#if defined(CONFIG_IGUANA)
/*
 * Iguana Family
 */
extern uintptr_t temp_cap_slot;
extern uintptr_t temp_cap_used;
extern uintptr_t temp_cap_size;
extern uintptr_t temp_cap_addr;

extern void
__cap_init(uintptr_t cap_slot, uintptr_t cap_used, uintptr_t cap_size, uintptr_t cap_addr);

memsection_ref_t vmalloc_memsect;
#endif

uintptr_t vmalloc_start, vmalloc_end;
extern unsigned long vmalloc_ms_base, vmalloc_ms_size;

/* FIXME: Should be extended for SMP? */ 
struct ig_irq_rec{
	unsigned short pending;
	L4_ThreadId_t  tid;
} irq_recs[NR_IRQS]; 

static int __init
iguana_arch_init(void)
{
	/*
	 * Enable the system error interrupts. These interrupts are 
	 * all reported to the kernel as machine checks, so the handler
	 * is a nop so it can be called to count the individual events.
	 */
#if defined(CONFIG_IGUANA)

	/*
	 * FIXME: We need to setup capabilties library properly now!
	 * Ideally we would do this really early, but this is about as early
	 * as we get!
	 */
	__cap_init(temp_cap_slot, temp_cap_used, temp_cap_size, temp_cap_addr);

        vmalloc_memsect = env_memsection(iguana_getenv("VMALLOC"));
	assert(vmalloc_memsect != 0);

	vmalloc_start = (uintptr_t)memsection_base(vmalloc_memsect);
	assert(vmalloc_start != 0);
	vmalloc_end = vmalloc_start + memsection_size(vmalloc_memsect) - 1;
#elif defined(CONFIG_CELL)
	okl4_env_segment_t *vmalloc_segment;

	vmalloc_segment = okl4_env_get_segment("MAIN_VMALLOC");
	assert(vmalloc_segment != NULL);

	vmalloc_start = vmalloc_segment->virt_addr;
	vmalloc_end = vmalloc_start + vmalloc_segment->size - 1;
#endif

	return 0;
}

arch_initcall(iguana_arch_init);

#define TIMER_IRQ 0

#if defined(CONFIG_IGUANA)
/*
 * XXX: namespace clash.  :-(
 */
#define device_create okl4_device_create

#include <driver/types.h>
#include <interfaces/vtimer_client.h>

server_t timer_server;
device_t timer_dev;
#endif

/*
 * XXX this needs 32/64bit cleanup + merge into irq.c
 */
int
mask_to_irq(L4_Word_t *mask)
{
	int i;
#if defined(CONFIG_IGUANA)
	for (i = 0 ; i < NR_IGUANA_IRQS; i++) {
		if (*mask & (1 << i)) {
			*mask &= ~(1 << i);

			/* 
			 * Bit 31 is ia32 BSP's way of saying hardware 
			 * interrupt
			 */
			if (i == 31) {
				/* 
				 * read hw irq number from 
				 * utcb->platform_reserved[0]
				 */
				return utcb_base_get()->platform_reserved[0];
			} else {
				return IGUANA_IRQ(i);
			}
		}
	}
#endif

#if defined(CONFIG_CELL)
	if (*mask & (1UL << 31)) {
		*mask &= ~(1UL << 31);
		return utcb_base_get()->platform_reserved[0];
	}
    else {
        if (*mask > 45) {
            i = *mask;
            *mask &= ~(1UL - 46);
            return i;
        }
    }
#endif
	return -1;
}

#if defined(CONFIG_IGUANA)
int
iguana_alloc_irq(void)
{
	int i;

	for (i = 0; i < NR_IGUANA_IRQS; i++) {
		if (!(irq_mask & (1UL << i))) {
			irq_mask |= (1UL << i);
			return IGUANA_IRQ(i);
		}
	}
	return -1;
}

L4_Word_t
corba_l4_error(CORBA_Environment *env)
{
	L4_Word_t	val = *(L4_Word_t *)(env);
	
	if ((val & 0xff) == CORBA_SYSTEM_EXCEPTION)
		return val >> 8;
	else
		return 0;
}
#endif

extern L4_ThreadId_t timer_thread;
extern int l4clksrc_init(void);
extern void l4clksrc_timer_setup(void);

/*
 * l4_checksched
 *
 * checks to see whether a reschedule needs to be done.
 */
static void
l4_checksched(void)
{
	if (need_resched())
		L4_Notify(main_thread, L4_PREEMPT);
}

void
interrupt_loop(void)
{
	struct thread_info *curinfo;

	/* Wait for wake up messge from timer init */
	L4_Call(main_thread);

	/* Find the timer device */
#if defined(CONFIG_IGUANA)
	l4clksrc_init();
	l4clksrc_timer_setup();
#elif defined(CONFIG_CELL)
#else
#error no timer
#endif

	for (;;) {
		int did_irq = 0;
		L4_MsgTag_t tag;
		L4_ThreadId_t sender;
		L4_Msg_t msg;
		L4_Word_t num;
		int irq;

		/*
		 * Accept everything.  While this is not ideal
		 * this ensures that when we enable the interrupt
		 * we do not miss an interrupt.  Guess we will
		 * just have to live with it.
		 */
		L4_Set_NotifyMask(~0UL);
		L4_Accept(L4_NotifyMsgAcceptor);

		tag = L4_Wait(&sender);

		curinfo = current_thread_info();
		curinfo->regs.mode++;

		L4_MsgStore(tag, &msg);
		num = L4_MsgWord(&msg, 0);
		
		if (L4_IpcFailed(tag)) {
			printk("Error Code: %lx -- %lx\n", 
			    tag.raw, L4_ErrorCode());
			panic("bad ipc");
		} else {
			if (sender.raw == L4_nilthread.raw)
            	   irq = mask_to_irq(&num);
			else {
                printk("Got unexpected IPC from 0x%x\n", sender.raw);
				panic("unexpected ipc message");
            }

			set_irq_regs(&curinfo->regs);

			while (irq != -1) {
				did_irq = 1;

				if (irqs_disabled()) {
					FD_SET(irq, &pending_irqs[smp_processor_id()]);
				} else {
					isr_irq_disable();
    				handle_irq(irq, NULL);
					isr_irq_enable();
				}
				if (sender.raw == L4_nilthread.raw) {
					irq = mask_to_irq(&num);
				}
			}

			curinfo->regs.mode--;

			if (did_irq) {
				if (irqs_disabled())
					IRQ_pending(smp_processor_id())++;
				else
					l4_checksched();
			}
		}
	}
}

static int
irq_mask_to_num(void)
{
	int i;
	int cpu = smp_processor_id();
	unsigned int least_sig;

	for (i = 0; i <= PENDING_MASK_SIZE; ++i){
		least_sig = fls(pending_irqs[cpu].fds_bits[i]);
		if (least_sig == 0)
			continue;
		else{
			int bit = (i * sizeof(long)*8) + least_sig;
			FD_CLR(bit-1, &pending_irqs[smp_processor_id()]);
			return (bit - 1);
		}
	}
	return -1;
}
/*
 * This is called to handle pending interrupts from the main linux thread
 * when we reenable interrupts and find there are interrupts pending.
 */
void
l4_handle_pending(void)
{
	int irq_num;
	struct thread_info * curinfo = current_thread_info();

	curinfo->regs.mode++;

	/*
	 * XXX
	 * 
	 * Concurrency problem: the FD_SET() and FD_CLR() are not 
	 * atomic but can be accessed either in the timer thread
	 * context or the syscall loop thread context
	 *
	 * -gl
	 */
	while((irq_num = irq_mask_to_num()) != -1)
		handle_irq(irq_num, NULL);

	IRQ_pending(smp_processor_id())--;

	curinfo->regs.mode--;
}

void local_irq_enable(void)
{
	if (unlikely(!in_interrupt() && IRQ_pending(smp_processor_id()))) {
		IRQ_state(smp_processor_id()) = 1;
		l4_handle_pending();
	}
	IRQ_state(smp_processor_id()) = 0;
}

EXPORT_SYMBOL(local_irq_enable);

