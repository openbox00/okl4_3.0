/*
 * Copyright 2004-2006 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

/*!
 * @file sdma_malloc.c
 * @brief This file contains functions for SDMA non-cacheable buffers allocation
 *
 * SDMA (Smart DMA) is used for transferring data between MCU and peripherals
 *
 * @ingroup SDMA
 */

#include <linux/init.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <asm/dma.h>
#include <asm/arch/hardware.h>

#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>

#define DEBUG 0

#if DEBUG
#define DPRINTK(fmt, args...) printk("%s: " fmt, __FUNCTION__ , ## args)
#else
#define DPRINTK(fmt, args...)
#endif

/*!
 * Defines SDMA non-cacheable buffers pool
 */
static struct dma_pool *pool;

/*!
 * SDMA memory conversion hashing structure
 */
typedef struct {
	/*! Virtual address */
	void *virt;
	/*! Physical address */
	unsigned long phys;
} virt_phys_struct;

/*!
 * Defines SDMA hash table size
 */
#define HASH_SIZE 512

/*!
 * Virtual to physical address conversion hash table
 */
static virt_phys_struct *virt_to_phys_hash[HASH_SIZE];
/*!
 * Physical to virtual address conversion hash table
 */
static virt_phys_struct *phys_to_virt_hash[HASH_SIZE];

/*!
 * Defines the size of each buffer in SDMA pool.
 * The size must be at least 512 bytes, because
 * sdma channel control blocks array size is 512 bytes
 */
#define SDMA_POOL_SIZE 512

/*!
 * Adds new buffer structure into conversion hash tables
 *
 * @param   vf   SDMA memory conversion hashing structure
 *
 * @return       1 on success, 0 on fail
 */
static int add_entry(virt_phys_struct * vf)
{
	int i, offset;
	int added;
	int res;

	res = 0;
	added = 0;
	offset = (((unsigned long)vf->virt) / SDMA_POOL_SIZE) % HASH_SIZE;

	for (i = 0; !added && i < HASH_SIZE; i++) {
		if (virt_to_phys_hash[(i + offset) % HASH_SIZE] == 0) {
			added = 1;
			virt_to_phys_hash[(i + offset) % HASH_SIZE] = vf;
		}
	}
	if (added) {
		added = 0;
		offset = (vf->phys / SDMA_POOL_SIZE) % HASH_SIZE;
		for (i = 0; !added && i < HASH_SIZE; i++) {
			if (phys_to_virt_hash[(i + offset) % HASH_SIZE] == 0) {
				added = 1;
				phys_to_virt_hash[(i + offset) % HASH_SIZE] =
				    vf;
			}
		}
	} else {
		res = -ENOMEM;
	}
	return res;
}

/*!
 * Deletes buffer stracture from conversion hash tables
 *
 * @param   vf   SDMA memory conversion hashing structure
 *
 * @return       1 on success, 0 on fail
 */
static int delete_entry(virt_phys_struct * vf)
{
	int i, offset, elem;
	int deleted;
	int res;

	res = 0;
	deleted = 0;
	offset = (((unsigned long)vf->virt) / SDMA_POOL_SIZE) % HASH_SIZE;

	for (i = 0; !deleted && i < HASH_SIZE; i++) {
		elem = (i + offset) % HASH_SIZE;
		if (virt_to_phys_hash[elem] != 0 &&
		    virt_to_phys_hash[elem]->virt == vf->virt) {
			deleted = 1;
			virt_to_phys_hash[elem] = 0;
		}
	}
	if (deleted) {
		deleted = 0;
		offset = (vf->phys / SDMA_POOL_SIZE) % HASH_SIZE;
		for (i = 0; !deleted && i < HASH_SIZE; i++) {
			elem = (i + offset) % HASH_SIZE;
			if (phys_to_virt_hash[elem] != 0 &&
			    phys_to_virt_hash[elem]->phys == vf->phys) {
				deleted = 1;
				kfree(phys_to_virt_hash[elem]);
				phys_to_virt_hash[elem] = 0;
			}
		}
	} else {
		res = -ENOMEM;
	}
	return res;
}

/*!
 * Virtual to physical address conversion functio
 *
 * @param   buf  pointer to virtual address
 *
 * @return       physical address
 */
unsigned long sdma_virt_to_phys(void *buf)
{
	int i, offset, elem;
	int found;
	unsigned long res;

	found = 0;
	res = 0;
	offset = (((unsigned long)buf) / SDMA_POOL_SIZE) % HASH_SIZE;

	for (i = 0; !found && i < HASH_SIZE; i++) {
		elem = (i + offset) % HASH_SIZE;
		if (virt_to_phys_hash[elem] != 0 &&
		    ((unsigned long)(virt_to_phys_hash[elem]->virt)) /
		    SDMA_POOL_SIZE == ((unsigned long)buf) / SDMA_POOL_SIZE) {
			found = 1;
			res = ((unsigned long)(virt_to_phys_hash[elem]->phys)) +
			    (unsigned long)((unsigned long)buf %
					    SDMA_POOL_SIZE);
		}
	}

	if (!found) {
		res = virt_to_phys(buf);
	}

	return res;
}

/*!
 * Physical to virtual address conversion functio
 *
 * @param   buf  pointer to physical address
 *
 * @return       virtual address
 */
void *sdma_phys_to_virt(unsigned long buf)
{
	int i, offset, elem;
	int found;
	void *res;

	found = 0;
	res = 0;
	offset = (buf / SDMA_POOL_SIZE) % HASH_SIZE;

	for (i = 0; !found && i < HASH_SIZE; i++) {
		elem = (i + offset) % HASH_SIZE;
		if (phys_to_virt_hash[elem] != 0 &&
		    ((unsigned long)phys_to_virt_hash[elem]->phys) /
		    HASH_SIZE == (unsigned long)buf / HASH_SIZE) {
			found = 1;
			res = (void *)(((unsigned long)
					(phys_to_virt_hash[elem]->virt)) +
				       buf % HASH_SIZE);
		}
	}

	if (!found) {
		res = phys_to_virt(buf);
	}

	return res;
}

/*!
 * Allocates uncacheable buffer
 *
 * @param   size    size of allocated buffer
 * @return  pointer to buffer
 */
void *sdma_malloc(size_t size)
{
	void *buf;
	dma_addr_t dma_addr;
	virt_phys_struct *vf;

	if (size > SDMA_POOL_SIZE) {
		printk(KERN_WARNING
		       "size in sdma_malloc is more than %d bytes\n",
		       SDMA_POOL_SIZE);
		buf = 0;
	} else {
		buf = dma_pool_alloc(pool, GFP_KERNEL, &dma_addr);
		if (buf > 0) {
			vf = kmalloc(sizeof(virt_phys_struct), GFP_KERNEL);
			vf->virt = buf;
			vf->phys = dma_addr;

			if (add_entry(vf) < 0) {
				dma_pool_free(pool, buf, dma_addr);
				kfree(vf);
				buf = 0;
			}
		}
	}

	return buf;
}

/*!
 * Frees uncacheable buffer
 *
 * @param  buf    buffer pointer for deletion
 */
void sdma_free(void *buf)
{
	virt_phys_struct vf;

	vf.virt = buf;
	vf.phys = sdma_virt_to_phys(buf);

	dma_pool_free(pool, buf, sdma_virt_to_phys(buf));

	delete_entry(&vf);
}

/*!
 * SDMA buffers pool initialization function
 */
void __init init_sdma_pool(void)
{
	int i;
	pool = dma_pool_create("SDMA", NULL, SDMA_POOL_SIZE, 0, 0);

	for (i = 0; i < HASH_SIZE; i++) {
		virt_to_phys_hash[i] = 0;
		phys_to_virt_hash[i] = 0;
	}
}

EXPORT_SYMBOL(init_sdma_pool);

MODULE_AUTHOR("Freescale Semiconductor, Inc.");
MODULE_DESCRIPTION("MXC Linux SDMA API");
MODULE_LICENSE("GPL");
