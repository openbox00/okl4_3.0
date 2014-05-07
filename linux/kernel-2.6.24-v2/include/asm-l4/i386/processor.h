#ifndef __L4_I386_PROCESSOR_H
#define __L4_I386_PROCESSOR_H

#include <linux/cache.h>

#include INC_SYSTEM(cpufeature.h)

#ifdef CONFIG_IA32_VDSO_ENABLE
extern int sysenter_setup(void);
#endif

#define cpu_relax()	barrier()

struct desc_struct {
	unsigned long a,b;
};   

struct thread_struct {
};

#define INIT_THREAD  { }

/*
 * User space process size: 2GB. This is hardcoded into a few places,
 * so don't change it unless you know what you are doing.
 */
#define TASK_SIZE	0x7fff8000UL
#define TASK_SHIFT	31

/*
 * This decides where the kernel will search for a free chunk of vm
 * space during mmap's.
 */
#define TASK_UNMAPPED_BASE	(PAGE_ALIGN(TASK_SIZE / 3))

/*
 * This decides where the kernel will map the __wombat_user_sig_handler
 * page to for signal and thread startup/modification.
 * This must not conflict with the utcb and kip area setup in
 * arch/l4/kernel/setup.c
 */
#define TASK_SIG_BASE	(0x98765000UL)

struct cpuinfo_x86 {
	__u8    x86;	    /* CPU family */
	__u8    x86_vendor;     /* CPU vendor */
	__u8    x86_model;
	__u8    x86_mask;
	char    wp_works_ok;	/* It doesn't on 386's */
	char    hlt_works_ok;	/* Problems on some 486Dx4's and old 386's */
	char    hard_math;
	char    rfu;
	int     cpuid_level;	/* Maximum supported CPUID level, -1=no CPUID */
	unsigned long   x86_capability[NCAPINTS];
	char    x86_vendor_id[16];
	char    x86_model_id[64];
	int     x86_cache_size;	/* in KB - valid for CPUS which support this
					call  */
	int     x86_cache_alignment;	/* In bytes */
	char    fdiv_bug;
	char    f00f_bug;
	char    coma_bug;
	char    pad0;
	int     x86_power;
	unsigned long loops_per_jiffy;
#ifdef CONFIG_SMP
	cpumask_t llc_shared_map;	/* cpus sharing the last level cache */
#endif
	unsigned char x86_max_cores;	/* cpuid returned max cores value */
	unsigned char apicid;
	unsigned short x86_clflush_size;
#ifdef CONFIG_SMP
	unsigned char booted_cores;     /* number of cores as seen by OS */
	__u8 phys_proc_id;		/* Physical processor id. */
	__u8 cpu_core_id;		/* Core id */
#endif
} __attribute__((__aligned__(SMP_CACHE_BYTES)));

extern struct cpuinfo_x86 boot_cpu_data;

/* generic versions from gas */
#define GENERIC_NOP1    ".byte 0x90\n"
#define GENERIC_NOP2            ".byte 0x89,0xf6\n"
#define GENERIC_NOP3        ".byte 0x8d,0x76,0x00\n"
#define GENERIC_NOP4        ".byte 0x8d,0x74,0x26,0x00\n"
#define GENERIC_NOP5        GENERIC_NOP1 GENERIC_NOP4
#define GENERIC_NOP6    ".byte 0x8d,0xb6,0x00,0x00,0x00,0x00\n"
#define GENERIC_NOP7    ".byte 0x8d,0xb4,0x26,0x00,0x00,0x00,0x00\n"
#define GENERIC_NOP8    GENERIC_NOP1 GENERIC_NOP7

/* 
 * In the real version they have ifdefs for AMD processors but we
 * just use the generic guys.
 */
#define ASM_NOP1 GENERIC_NOP1
#define ASM_NOP2 GENERIC_NOP2
#define ASM_NOP3 GENERIC_NOP3
#define ASM_NOP4 GENERIC_NOP4
#define ASM_NOP5 GENERIC_NOP5
#define ASM_NOP6 GENERIC_NOP6
#define ASM_NOP7 GENERIC_NOP7
#define ASM_NOP8 GENERIC_NOP8

#define ASM_NOP_MAX 8

/* Stop speculative execution */
static inline void sync_core(void)
{
	int tmp;
	asm volatile("cpuid" : "=a" (tmp) : "0" (1) : "ebx","ecx","edx","memory"
);
}


#endif /* __L4_I386_PROCESSOR_H */
