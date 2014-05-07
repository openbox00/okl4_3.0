/*
 *  Copyright (C) 2004, National ICT Australia
 *  Copyright (C) 2007, Open Kernel Labs
 *
 *  from:
 *  linux/arch/mips/kernel/proc.c
 *
 *  Copyright (C) 1995, 1996, 2001  Ralf Baechle
 *  Copyright (C) 2001  MIPS Technologies, Inc.
 */

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/seq_file.h>

/*
 * Info that gets shown in /proc/cpuinfo
 */
static int show_cpuinfo(struct seq_file *m, void *v)
{
	unsigned long n = (unsigned long) v - 1;
	//char fmt [64];

#ifdef CONFIG_SMP
	if (!cpu_isset(n, cpu_online_map))
		return 0;
#endif

	/*
	 * For the first processor also print the system type
	 */
#if 0 //CONFIG_MIPS64
	if (n == 0)
		seq_printf(m, "system type\t\t: %s\n", get_system_type());
#endif

	seq_printf(m, "processor\t\t: %ld\n", n);
/*	sprintf(fmt, "cpu model\t\t: %%s V%%d.%%d%s\n",
	        cpu_has_fpu ? "  FPU V%d.%d" : "");
	seq_printf(m, fmt, cpu_name[current_cpu_data.cputype <= CPU_LAST ?
	                            current_cpu_data.cputype : CPU_UNKNOWN],
	                           (version >> 4) & 0x0f, version & 0x0f,
	                           (fp_vers >> 4) & 0x0f, fp_vers & 0x0f);*/
	seq_printf(m, "BogoMIPS\t\t: %lu.%02lu\n",
	              loops_per_jiffy / (500000/HZ),
	              (loops_per_jiffy / (5000/HZ)) % 100);
/*	seq_printf(m, "wait instruction\t: %s\n", cpu_wait ? "yes" : "no");
	seq_printf(m, "microsecond timers\t: %s\n",
	              cpu_has_counter ? "yes" : "no");
	seq_printf(m, "tlb_entries\t\t: %d\n", current_cpu_data.tlbsize);
	seq_printf(m, "extra interrupt vector\t: %s\n",
	              cpu_has_divec ? "yes" : "no");
	seq_printf(m, "hardware watchpoint\t: %s\n",
	              cpu_has_watch ? "yes" : "no");

	sprintf(fmt, "VCE%%c exceptions\t\t: %s\n",
	        cpu_has_vce ? "%d" : "not available");
	seq_printf(m, fmt, 'D', vced_count);
	seq_printf(m, fmt, 'I', vcei_count);*/

	return 0;
}

static void *c_start(struct seq_file *m, loff_t *pos)
{
	unsigned long i = *pos;

	return i < NR_CPUS ? (void *) (i + 1) : NULL;
}

static void *c_next(struct seq_file *m, void *v, loff_t *pos)
{
	++*pos;
	return c_start(m, pos);
}

static void c_stop(struct seq_file *m, void *v)
{
}

struct seq_operations cpuinfo_op = {
	.start	= c_start,
	.next	= c_next,
	.stop	= c_stop,
	.show	= show_cpuinfo,
};
