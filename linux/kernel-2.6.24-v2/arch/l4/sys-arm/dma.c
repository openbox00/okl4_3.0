/*
 *  linux/arch/arm/kernel/dma.c
 *
 *  Copyright (C) 1995-2000 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Front-end to the DMA handling.  This handles the allocation/freeing
 *  of DMA channels, and provides a unified interface to the machines
 *  DMA facilities.
 */
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/mman.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/errno.h>

#include <asm/dma.h>

/* Cruft */
#if 0
//#include <asm/mach/dma.h>

DEFINE_SPINLOCK(dma_spin_lock);

#undef MAX_DMA_CHANNELS
#define MAX_DMA_CHANNELS 1

#if MAX_DMA_CHANNELS > 0

/*
 * Get dma list for /proc/dma
 */
int get_dma_list(char *buf)
{
	BUG();
	return 0;
}

/*
 * Request DMA channel
 *
 * On certain platforms, we have to allocate an interrupt as well...
 */
int request_dma(dmach_t channel, const char *device_id)
{
	BUG();
        return 0;
}

/*
 * Free DMA channel
 *
 * On certain platforms, we have to free interrupt as well...
 */
void free_dma(dmach_t channel)
{
	BUG();
}

/* Set DMA address
 *
 * Copy address to the structure, and set the invalid bit
 */
void set_dma_addr (dmach_t channel, unsigned long physaddr)
{
	BUG();
}

/* Set DMA byte count
 *
 * Copy address to the structure, and set the invalid bit
 */
void set_dma_count (dmach_t channel, unsigned long count)
{
	BUG();
}

/* Set DMA direction mode
 */
void set_dma_mode (dmach_t channel, dmamode_t mode)
{
	BUG();
}

/* Enable DMA channel
 */
void enable_dma (dmach_t channel)
{
	BUG();
}

/* Disable DMA channel
 */
void disable_dma (dmach_t channel)
{
	BUG();
}

void set_dma_speed(dmach_t channel, int cycle_ns)
{
	BUG();
}

int get_dma_residue(dmach_t channel)
{
	BUG();
	return 0;
}

#else

int request_dma(dmach_t channel, const char *device_id)
{
	return -EINVAL;
}

int get_dma_residue(dmach_t channel)
{
	return 0;
}

#define GLOBAL_ALIAS(_a,_b) asm (".set " #_a "," #_b "; .globl " #_a)
GLOBAL_ALIAS(disable_dma, get_dma_residue);
GLOBAL_ALIAS(enable_dma, get_dma_residue);
GLOBAL_ALIAS(free_dma, get_dma_residue);
GLOBAL_ALIAS(get_dma_list, get_dma_residue);
GLOBAL_ALIAS(set_dma_mode, get_dma_residue);
GLOBAL_ALIAS(set_dma_page, get_dma_residue);
GLOBAL_ALIAS(set_dma_count, get_dma_residue);
GLOBAL_ALIAS(set_dma_addr, get_dma_residue);
GLOBAL_ALIAS(set_dma_sg, get_dma_residue);
GLOBAL_ALIAS(set_dma_speed, get_dma_residue);
GLOBAL_ALIAS(init_dma, get_dma_residue);

#endif

EXPORT_SYMBOL(request_dma);
EXPORT_SYMBOL(free_dma);
EXPORT_SYMBOL(enable_dma);
EXPORT_SYMBOL(disable_dma);
EXPORT_SYMBOL(set_dma_addr);
EXPORT_SYMBOL(set_dma_count);
EXPORT_SYMBOL(set_dma_mode);
//EXPORT_SYMBOL(set_dma_page);
EXPORT_SYMBOL(get_dma_residue);
//EXPORT_SYMBOL(set_dma_sg);
EXPORT_SYMBOL(set_dma_speed);

EXPORT_SYMBOL(dma_spin_lock);

#endif
