#include "l4.h"
#include "assert.h"
#include "wombat.h"

#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/kernel.h>
#include <linux/bootmem.h>
#include <linux/fs.h>
#include <linux/root_dev.h>
#include <linux/initrd.h>
#include <linux/highmem.h>
#include <linux/file.h>
#include <linux/module.h>

#include <asm/page.h>
#include <asm/uaccess.h>
#include <asm/syscalls.h>
#include <asm/signal.h>
#include <asm/signal_l4.h>
#include <asm/tlbflush.h>
#include <asm/setup.h>
#include <asm/okl4.h>

#if defined(CONFIG_CELL)

#include <okl4/env.h>
#include <okl4/kclist.h>
#include <okl4/init.h>

#elif defined(CONFIG_IGUANA)

#include <iguana/thread.h>
#include <iguana/cap.h>
#include <iguana/memsection.h>
#include <iguana/tls.h>
#include <iguana/env.h>

uintptr_t temp_cap_slot;
uintptr_t temp_cap_used;
uintptr_t temp_cap_size;
uintptr_t temp_cap_addr;

void *main_tls_area[KERNEL_THREADS][32];

#endif

uintptr_t dma_heap_base, dma_heap_phys_base;

ALIGNED(32) static uintptr_t main_stack[STACK_SIZE];
#ifdef __ENABLE_OKL4FS
ALIGNED(32) static uintptr_t intervm_stack[STACK_SIZE] ALIGNED(32);
#endif

extern void interrupt_loop(void);

extern void start_kernel(void);
extern void intervm_loop(void);

void *console_out, *console_in;

#if defined(CONFIG_CELL)
void *libokl4_forced_symbols = &okl4_forced_symbols;
#endif

/* Our per cpu irq enable/disable. Where should this go? */
DEFINE_PER_CPU(irq_info_t, _l4_irq_state) = { 0 };
EXPORT_PER_CPU_SYMBOL(_l4_irq_state);
#define RCU_tasklet(cpu) (per_cpu(rcu_tasklet, cpu))

L4_SpaceId_t linux_space;

L4_Word_t start_phys_mem, end_phys_mem;
L4_ThreadId_t main_thread;
L4_ThreadId_t timer_thread;
L4_ThreadId_t timer_handle;
L4_ThreadId_t intervm_thread;

unsigned long get_instr(unsigned long addr)
{
	extern pte_t * lookup_pte(pgd_t *page_dir, unsigned long pf_address);
	pte_t *ptep;
	unsigned long base;

	ptep = lookup_pte((pgd_t *)current->mm->pgd, addr);
	//printk("\rpp: pdir: %p, faddr: %lx, ptep: %p\n",
	//	(pgd_t *)current->mm->pgd, addr, ptep);

	if (ptep == NULL)
		return -1UL;

	base = (unsigned long)phys_to_virt(pte_pfn(*ptep) << PAGE_SHIFT);
	return *(unsigned long*)(base + (addr & ~PAGE_MASK));
}

unsigned long set_instr(unsigned long addr, unsigned long instr)
{
	extern pte_t * lookup_pte(pgd_t *page_dir, unsigned long pf_address);
	pte_t *ptep;
	unsigned long base;

	ptep = lookup_pte((pgd_t *)current->mm->pgd, addr);
	//printk("\rpp: pdir: %p, faddr: %lx, ptep: %p\n",
	//	(pgd_t *)current->mm->pgd, addr, ptep);

	if (ptep == NULL)
		return -1UL;

	base = (unsigned long)phys_to_virt(pte_pfn(*ptep) << PAGE_SHIFT);
	*(unsigned long*)(base + (addr & ~PAGE_MASK)) = instr;
	return 0;
}

/*
Malloc
*/
void *
malloc(size_t size)
{
	void *ret;
	//printk("malloc: %d\n", (int)size);
	ret = kmalloc(size, GFP_ATOMIC);
	//printk("malloc returned: %p\n", ret);
	return ret;
}

void *
calloc(size_t nmemb, size_t size)
{
	printk("calloc called\n");
	return NULL;
}

void
free(void *ptr)
{
	return kfree(ptr);
}

int
puts(const char *format)
{
	printk("puts called\n");
	return 0;
}

int
printf(const char *format, ...)
{
	printk("printf\n");
	return 0;
//	return printk(format);
}



void 
__linux_cap_init(uintptr_t cap_slot, uintptr_t cap_used, uintptr_t cap_size, uintptr_t cap_addr)
{
//	temp_cap_slot = cap_slot;
//	temp_cap_used = cap_used;
//	temp_cap_size = cap_size;
//	temp_cap_addr = cap_addr;
//	first_cap = (cap_t*) cap_addr;
}


void
__libc_setup(void *callback, void *stdin_p, void *stdout_p, void *stderr_p, 
	     unsigned long heap_base, unsigned long heap_end)
{
	console_out = stdout_p;
	console_in = stdin_p;

	/* Setup the memory areas from iguana */
	/*
	 * +1 is added because the heap_end is the last address
	 * so when we lop of the bottom bits it returns the
	 * pfn of the last page, so we are one less the number
	 * of pages without the +1.
	 */
	assert(heap_end > heap_base);
	bootmem_areas = 1;
	bootmem_area[0].page_base = heap_base >> PAGE_SHIFT;
	bootmem_area[0].pages = (heap_end >> PAGE_SHIFT) - bootmem_area[0].page_base + 1;
}

extern unsigned long vmalloc_ms_phys;
extern unsigned long vmalloc_ms_base;
extern unsigned long vmalloc_ms_size;

void
__lib_init(uintptr_t *buf)
{
#if defined(CONFIG_CELL)
	okl4_env_segment_t *heap;

	heap = okl4_env_get_segment("MAIN_HEAP");
	assert(heap != NULL);
	__libc_setup(NULL, NULL, NULL, NULL,
			(unsigned long)heap->virt_addr,
			(unsigned long)(heap->virt_addr + heap->size - 1));

	dma_heap_base = (uint32_t)heap->virt_addr;
	dma_heap_phys_base = *((uint32_t *)okl4_env_get("heap_physical"));

	//vmalloc_ms_phys = env->vmalloc_phys;
	//vmalloc_ms_base = env->vmalloc_base;
	//vmalloc_ms_size = env->vmalloc_size;
#else
	void* callback;
	void* stdin_p;
	void* stdout_p;
	void* stderr_p;
	char* heap_base;
	size_t heap_size;
	uint32_t dma_size;

	__lib_iguana_init(buf);

    callback  = env_memsection_base(iguana_getenv("__OKL4_CALLBACK_BUFFER"));
    stdin_p   = NULL; //env_memsection_base(iguana_getenv("OKL4_SERIAL_SERVER"));
    stdout_p  = NULL; //env_memsection_base(iguana_getenv("OKL4_SERIAL_SERVER"));
    stderr_p  = NULL; //env_memsection_base(iguana_getenv("OKL4_SERIAL_SERVER"));
    heap_base = (char*) env_const(iguana_getenv("HEAP_BASE"));
    heap_size = env_const(iguana_getenv("HEAP_SIZE"));


	__libc_setup(callback, stdin_p, stdout_p, stderr_p,
		     (unsigned long) heap_base,
                     (unsigned long) (heap_base + heap_size - 1));
         __cap_init();

//	__linux_cap_init((uintptr_t) iguana_getenv(IGUANA_GETENV_CLIST_SLOT),
//			 (uintptr_t) iguana_getenv(IGUANA_GETENV_CLIST_USED),
//			 CLIST_MEMORY_SIZE / sizeof(cap_t),
//			 (uintptr_t) iguana_getenv(IGUANA_GETENV_CLIST_BASE));
//
	dma_heap_base = (uint32_t)heap_base;
	dma_heap_phys_base = memsection_virt_to_phys((uintptr_t)heap_base,
	    &dma_size);
#endif
}

#if defined(CONFIG_IGUANA)
void __sys_entry(void *buf, int argc, char** argv);
#else
void __sys_entry(void);
#endif
void _Exit(int status);

int main(int argc, char** argv);

/* This is the entry point from the C libraries crt0 */
void
#if defined(CONFIG_CELL)
__sys_entry(void)
#else
__sys_entry(void *buf, int argc, char** argv)
#endif
{
	int result;

#if defined(CONFIG_CELL)
	void *env;
	okl4_env_args_t *args;
	int argc = 0;
	char **argv = NULL;

	L4_StoreMR(1, (void *)&env);
	__okl4_environ = env;

	args = OKL4_ENV_GET_ARGS("MAIN_ARGS");
	if (args != NULL) {
		argc = args->argc;
		argv = &args->argv;
	}

#else
	void *env = buf;
#endif
	__lib_init((void *)env);
	/* Start Linux */
	result = main(argc, argv);
	/* Exit Linux Server */
	_Exit(result);
}

#if defined(CONFIG_IGUANA)
void
setup_tls(int thread_num)
{
	__tls_init(main_tls_area[thread_num]);
}
#endif

int
main(int argc, char **argv)
{
	int r;

#if defined(CONFIG_CELL)
    L4_CapId_t cap;
    okl4_kcap_item_t kcap;
    okl4_kclist_t *kclist = okl4_env_get("MAIN_KCLIST");
    assert(kclist != NULL);

    okl4_kclist_kcap_allocany(kclist, &kcap);
    cap = okl4_kcap_item_getkcap(&kcap);
#endif

	/* Start a new thread */
	assert(argc > 0 && argv[0] != NULL);

	strlcpy(boot_command_line, argv[0], COMMAND_LINE_SIZE);
	L4_KDB_SetThreadName(L4_myselfconst, "L_timer");
#if defined(CONFIG_CELL)
	linux_space = L4_SpaceId(*(OKL4_ENV_GET_MAIN_SPACEID("MAIN_SPACE_ID")));
	main_thread = okl4_create_sys_thread(linux_space,
			L4_rootserver,
			L4_rootserver,
#if defined(NO_UTCB_RELOCATE)
			(void *)-1UL,
#else
			(void *)L4_GetUtcbBase() + 0x400,
#endif
			cap.raw);
#else	/*!CONFIG_CELL*/
	thread_create(&main_thread);
#endif	/*CONFIG_CELL*/
	r  = L4_Set_Priority(main_thread, 99);
	assert (r != 0);

#if defined(CONFIG_IGUANA)
	timer_thread = thread_l4tid(env_thread(iguana_getenv("MAIN")));
#else
	timer_thread = L4_CapId(TYPE_CAP, 0);
#endif
	L4_KDB_SetThreadName(main_thread, "L_syscall");

	/* Setup our TLS as well */
#if defined(CONFIG_IGUANA)
	setup_tls(TIMER_THREAD);
#endif

#ifdef __ENABLE_OKL4FS
    thread_create(&intervm_thread);
    r = L4_Set_Priority(intervm_thread, 98);
    assert(r != 0);
    L4_KDB_SetThreadName(intervm_thread, "L_intervm");
#endif

	/* Thread info setup. */
	/* FIXME:  remember for SMP startup */
	current_tinfo(smp_processor_id()) = (unsigned long)&init_thread_union.thread_info;
	current_thread_info()->user_tid = L4_nilthread;
	current_thread_info()->user_handle = L4_nilthread;

	L4_Start_SpIp(main_thread,
		      (L4_Word_t) &main_stack[STACK_SIZE-2],
		      (L4_Word_t) start_kernel);
    
#ifdef __ENABLE_OKL4FS
	L4_Start_SpIp(intervm_thread,
		      (L4_Word_t) &intervm_stack[STACK_SIZE-2],
		      (L4_Word_t) intervm_loop);
#endif

	/* Now we go and do the timer stuff */
	interrupt_loop();
	return 0;
}
