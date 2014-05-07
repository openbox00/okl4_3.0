#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/syscalls.h>
#include <linux/string.h>

#define __KERNEL_SYSCALLS__

#include <asm/unistd.h>	/* execve */

int
kernel_execve(const char *filename, char *const argv[], char *const envp[])
{
	return execve(filename, argv, envp);
}

void
check_bugs (void)
{
        /* No arch L4 bugs */

    /* If i386, setup sysenter */
#ifdef __i386__
#ifdef CONFIG_IA32_VDSO_ENABLE
	sysenter_setup();
#endif
#endif
} 

void
trap_init (void) 
{
	/* No trap vectors required on L4 */
} 

struct task_struct;

const char *get_system_type(void)
{
	return "L4 Default arch";
}

//void per_cpu__mmu_gathers (void) { printk("per_cpu__mmu_gathers called\n"); } 
//void ptrace_disable (void) { printk("ptrace_disable called\n"); } 
//void search_extable (void) { printk("search_extable called\n"); } 
void show_stack(struct task_struct *task, unsigned long *sp) { printk("show_stack called\n"); }
void dump_fpu (void) { /* printk("dump_fpu called\n"); */ }
void free_initrd_mem (void) { printk("free_initrd_mem called\n"); }

/* ARM specific */
#ifndef __HAVE_ARCH_MEMZERO
void __memzero (void) { printk("__memzero called\n"); } 
#endif

void __cpu_up (void) { printk("__cpu_up called"); } 
//void cpu_online_map (void) { printk("cpu_online_map called"); } 
void setup_profiling_timer (void) { printk("setup_profiling_timer called"); } 
//void smp_call_function (void) { printk("smp_call_function called"); } 
//void smp_prepare_boot_cpu (void) { printk("smp_prepare_boot_cpu called"); } 
void smp_prepare_cpus (void) { printk("smp_prepare_cpus called"); } 
//void smp_send_reschedule (void) { printk("smp_send_reschedule called"); } 
void smp_send_stop (void) { printk("smp_send_stop called"); } 
