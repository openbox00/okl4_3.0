#include <linux/kernel.h>

EXPORT_SYMBOL(boot_cpu_data);

struct cpuinfo_x86 {
        __u8    x86;            /* CPU family */
        __u8    x86_vendor;     /* CPU vendor */
        __u8    x86_model;
        __u8    x86_mask;
        char    wp_works_ok;    /* It doesn't on 386's */
        char    hlt_works_ok;   /* Problems on some 486Dx4's and old 386's */
        char    hard_math;
        char    rfu;
        int     cpuid_level;    /* Maximum supported CPUID level, -1=no CPUID */
        unsigned long   x86_capability[NCAPINTS];
        char    x86_vendor_id[16];
        char    x86_model_id[64];
        int     x86_cache_size;  /* in KB - valid for CPUS which support this
                                    call  */
        int     x86_cache_alignment;    /* In bytes */
        char    fdiv_bug;
        char    f00f_bug;
        char    coma_bug;
        char    pad0;
        int     x86_power;
#ifdef CONFIG_SMP
        cpumask_t llc_shared_map;       /* cpus sharing the last level cache */
#endif
        unsigned char x86_max_cores;    /* cpuid returned max cores value */
        unsigned char apicid;
        unsigned short x86_clflush_size;
#ifdef CONFIG_SMP
        unsigned char booted_cores;     /* number of cores as seen by OS */
        __u8 phys_proc_id;              /* Physical processor id. */
        __u8 cpu_core_id;               /* Core id */
#endif
} __attribute__((__aligned__(SMP_CACHE_BYTES)));

struct cpuinfo_x86 boot_cpu_data __read_mostly = { 0, 0, 0, 0, -1, 1, 0, 0, -1 }
; 

