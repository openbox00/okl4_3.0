#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/highmem.h>

#include <asm/uaccess.h>

#include <l4/ipc.h>
#include "assert.h"

#if defined(CONFIG_IGUANA)
#include <iguana/pd.h>
#include <iguana/thread.h>
#include <iguana/memsection.h>
#endif

extern L4_ThreadId_t main_thread;

EXPORT_SYMBOL(init_mm);
EXPORT_SYMBOL(init_task);
EXPORT_PER_CPU_SYMBOL(_l4_current_tinfo);

EXPORT_SYMBOL(l4_handle_pending);

EXPORT_SYMBOL(flush_dcache_page);
EXPORT_SYMBOL(flush_icache_range);

EXPORT_SYMBOL(copy_from_user);
EXPORT_SYMBOL(copy_to_user);

EXPORT_SYMBOL(start_thread);
EXPORT_SYMBOL(kernel_thread);
EXPORT_SYMBOL(main_thread);

#ifdef CONFIG_DISCONTIGMEM
EXPORT_SYMBOL(node_data);
EXPORT_SYMBOL(pfn_hash);
#endif

EXPORT_SYMBOL(__assert);

#if defined(CONFIG_IGUANA)
EXPORT_SYMBOL(thread_myself);
EXPORT_SYMBOL(pd_myself);

EXPORT_SYMBOL(pd_delete);
EXPORT_SYMBOL(pd_create_memsection);
EXPORT_SYMBOL(memsection_register_server);
#endif

#ifdef CONFIG_AEABI
/*
 * Needed for modules
 */
extern void *__aeabi_idivmod;

EXPORT_SYMBOL(__aeabi_idivmod);
#endif

/*
 * Because we don't use OKL4 libc we need to do this to force
 * the symbol to be there.
 */
extern void *okl4_forced_symbols[];

EXPORT_SYMBOL(okl4_forced_symbols);
