#ifndef __L4_SMP_H
#define __L4_SMP_H

//extern cpumask_t cpu_online_map;

#ifdef CONFIG_SMP

#define smp_processor_id() (current->pid)
#define cpu_logical_map(n) (n)
#define cpu_number_map(n) (n)
#define PROC_CHANGE_PENALTY	15 /* Pick a number, any number */
extern int hard_smp_processor_id(void);
#define NO_PROC_ID -1

//#define cpu_online(cpu) 1
//#cpu_isset(cpu, cpu_online_map)

//extern int ncpus;
#define cpu_possible(cpu) 0
//(cpu < ncpus)

extern inline void smp_cpus_done(unsigned int maxcpus)
{
}

#endif

#endif
