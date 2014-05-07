#include "linux/module.h"
#include "linux/sched.h"

extern void _Exit(int);

/*
 * Power off function, if any
 */
void (*pm_power_off)(void);
EXPORT_SYMBOL(pm_power_off);

void machine_halt (void)
{
	printk("Linux server halting!\n");
	_Exit(0);
}

void machine_restart (void)
{
	printk("Linux server restart!\n");
	_Exit(1);
}

void machine_power_off(void)
{
	printk("Linux server power off!\n");
	_Exit(2);
}

EXPORT_SYMBOL(machine_power_off);
