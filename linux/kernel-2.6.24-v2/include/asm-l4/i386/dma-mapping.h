/*
 *  linux/include/asm-l4/dma-mapping.h
 *
 *  Copyright (c) 2006, National ICT Australia
 *      David Snowdon
 *
 * Copyright (C) 2002 James.Bottomley@HansenPartnership.com 
 * Copyright (C) 2006 National ICT Australia 
 */

#ifndef _ASM_L4_I386_DMA_MAPPING_H
#define _ASM_L4_I386_DMA_MAPPING_H

#include <linux/scatterlist.h>
#include <asm/io.h>
#include <asm/page.h>
#include <linux/mm.h>

#include <l4/thread.h>
#include <l4/kdebug.h>
#include <l4/misc.h>
#include <l4/map.h>
#include <l4/space.h>
#include <l4/cache.h>

#include <iguana/thread.h>
#include <iguana/memsection.h>
#include <iguana/hardware.h>
#include <iguana/physmem.h>


static inline int
dma_supported(struct device *dev, u64 mask)
{
	return dev->dma_mask && *dev->dma_mask != 0;
}

static inline int
dma_set_mask(struct device *dev, u64 dma_mask)
{
        if (!dev->dma_mask || !dma_supported(dev, dma_mask))
                return -EIO;
        *dev->dma_mask = dma_mask;

        return 0;
}

static inline dma_addr_t
dma_map_single(struct device *dev, void *vaddr, size_t size,
	       enum dma_data_direction direction)
{
	uint32_t page_addr, phys_offset;

	BUG_ON(!valid_dma_direction(direction));
	WARN_ON(size == 0);

	page_addr = (uint32_t)virt_to_phys(vaddr);
	phys_offset = page_addr - dma_heap_base;

	return dma_heap_phys_base + phys_offset;
}

static inline void
dma_unmap_single(struct device *dev, dma_addr_t dma_addr, size_t size,
		 enum dma_data_direction direction)
{
	/* not tested, don't trust it yet */
	BUG_ON(!valid_dma_direction(direction));
}

static inline dma_addr_t
dma_map_page(struct device *dev, struct page *page,
	     unsigned long offset, size_t size,
	     enum dma_data_direction direction)
{
	BUG();


	/* 
	 * TODO:
	 * Find the linux "phys" address using:
	 * page_addr = page_to_phys(page);
	 * Then run that address through the same lookup used * in dma_map_sg()
	 */

	return 0;
}

static inline void
dma_unmap_page(struct device *dev, dma_addr_t dma_address, size_t size,
	       enum dma_data_direction direction)
{
	BUG();
}

/* 
 * Wombat physical addresses are actually Iguana virtual addresses
 * page_to_phys will return the wombat physical address of the page
 *
 * The base of the memsection containing the page is found, as
 * well as the base of the fpage backing it.
 * However calculating the offset into the fpage is less
 * straight-forward since we cannot assume the memsection is
 * backed by a single physmem. It is necessary to iterate through
 * all physmems, subtracting their sizes from the virtual offset
 * into the page until we hit the actual physmem which is backing
 * the page, leaving us with the physical offset.
 *
 * All these calculations should eventually be moved into a function
 * of their own since map_single and map_page will probably be
 * using the same translations.
 * -jsok
 */
static inline int
dma_map_sg(struct device *dev, struct scatterlist *sg, int nents,
	   enum dma_data_direction direction)
{
	int i;
	uint32_t		page_addr;
	uint32_t		phys_offset;

	BUG_ON(!valid_dma_direction(direction));
	WARN_ON(nents == 0 || sg[0].length == 0);

	for (i = 0; i < nents; i++ ) {
        	BUG_ON(!sg_page(&sg[i]));

		page_addr = (uint32_t)page_to_phys(sg_page(&sg[i]));
		phys_offset = page_addr - dma_heap_base;
		sg[i].dma_address = phys_offset + dma_heap_phys_base + 
		    sg[i].offset;
	}

	flush_write_buffers();
	return nents;
}

static inline void
dma_unmap_sg(struct device *dev, struct scatterlist *sg, int nhwentries,
	     enum dma_data_direction direction)
{
	BUG_ON(!valid_dma_direction(direction));
}

static inline void
dma_sync_single_for_cpu(struct device *dev, dma_addr_t dma_handle, size_t size,
			enum dma_data_direction direction)
{
	BUG();
}

static inline void
dma_sync_single_for_device(struct device *dev, dma_addr_t dma_handle, size_t size,
			   enum dma_data_direction direction)
{
	BUG();
}

static inline void
dma_sync_sg_for_cpu(struct device *dev, struct scatterlist *sg, int nelems,
		    enum dma_data_direction direction)
{
	BUG();
}

static inline void
dma_sync_sg_for_device(struct device *dev, struct scatterlist *sg, int nelems,
		       enum dma_data_direction direction)
{
	BUG();
}

static inline int
dma_mapping_error(dma_addr_t dma_addr)
{
	BUG();
	return 0;
}

/* Now for the API extensions over the pci_ one */

#define dma_alloc_noncoherent(d, s, h, f) dma_alloc_coherent(d, s, h, f)
#define dma_free_noncoherent(d, s, v, h) dma_free_coherent(d, s, v, h)
#define dma_is_consistent(d)	(1)

void *dma_alloc_coherent(struct device *dev, size_t size,
		dma_addr_t *dma_handle, gfp_t gfp);

void dma_free_coherent(struct device *dev, size_t size,
		void *vaddr, dma_addr_t dma_handle);

static inline int
dma_get_cache_alignment(void)
{
	BUG();
	return 0;
}

static inline void
dma_sync_single_range_for_cpu(struct device *dev, dma_addr_t dma_handle,
			      unsigned long offset, size_t size,
			      enum dma_data_direction direction)
{
	BUG();
}

static inline void
dma_sync_single_range_for_device(struct device *dev, dma_addr_t dma_handle,
				 unsigned long offset, size_t size,
				 enum dma_data_direction direction)
{
	BUG();
}

static inline void
dma_cache_sync(void *vaddr, size_t size,
	       enum dma_data_direction direction)
{
	/* could define this in terms of the dma_cache ... operations,
	 * but if you get this on a platform, you should convert the platform
	 * to using the generic device DMA API */
	BUG();
}

#endif

