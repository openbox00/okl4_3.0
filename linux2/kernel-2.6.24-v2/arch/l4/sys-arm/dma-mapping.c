/*
 *  linux/arch/l4/sys-arm/dma-mapping.c
 *
 *  Copyright (C) 2000-2004 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  DMA uncached mapping support.
 */
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>

#include <asm/cacheflush.h>
#include <asm/tlbflush.h>
#include <asm/sizes.h>
#include <asm/okl4.h>
#include <okl4/env.h>
#include "assert.h"


#ifndef ISA_DMA_THRESHOLD
#define ISA_DMA_THRESHOLD (0xffffffffULL)
#endif

static uintptr_t CONSISTENT_END;

#define CONSISTENT_BASE	dma_region_start

#define CONSISTENT_OFFSET(x)	(((unsigned long)(x) - CONSISTENT_BASE) >> PAGE_SHIFT)

#define DMA_HEAP_VIRT_TO_PHYS(x) ((unsigned long)(x) - dma_heap_base + dma_heap_phys_base)

/*
 * These are the page tables (2MB each) covering uncached, DMA consistent allocations
 */
static DEFINE_SPINLOCK(consistent_lock);
uintptr_t dma_region_start;
/*
 * VM region handling support.
 *
 * This should become something generic, handling VM region allocations for
 * vmalloc and similar (ioremap, module space, etc).
 *
 * I envisage vmalloc()'s supporting vm_struct becoming:
 *
 *  struct vm_struct {
 *    struct vm_region	region;
 *    unsigned long	flags;
 *    struct page	**pages;
 *    unsigned int	nr_pages;
 *    unsigned long	phys_addr;
 *  };
 *
 * get_vm_area() would then call vm_region_alloc with an appropriate
 * struct vm_region head (eg):
 *
 *  struct vm_region vmalloc_head = {
 *	.vm_list	= LIST_HEAD_INIT(vmalloc_head.vm_list),
 *	.vm_start	= VMALLOC_START,
 *	.vm_end		= VMALLOC_END,
 *  };
 *
 * However, vmalloc_head.vm_start is variable (typically, it is dependent on
 * the amount of RAM found at boot time.)  I would imagine that get_vm_area()
 * would have to initialise this each time prior to calling vm_region_alloc().
 */
struct vm_region {
	struct list_head	vm_list;
	unsigned long		vm_start;
	unsigned long		vm_end;
	struct page		*vm_pages;
	int			vm_active;
};

static struct vm_region consistent_head = {
	.vm_list	= LIST_HEAD_INIT(consistent_head.vm_list),
	.vm_start	= 0,
	.vm_end		= 0,
};

static struct vm_region *
vm_region_alloc(struct vm_region *head, size_t size, gfp_t gfp)
{
	unsigned long addr = head->vm_start, end = head->vm_end - size;
	unsigned long flags;
	struct vm_region *c, *new;

	new = kmalloc(sizeof(struct vm_region), gfp);
	if (!new)
		goto out;

	spin_lock_irqsave(&consistent_lock, flags);

	list_for_each_entry(c, &head->vm_list, vm_list) {
		if ((addr + size) < addr)
			goto nospc;
		if ((addr + size) <= c->vm_start)
			goto found;
		addr = c->vm_end;
		if (addr > end)
			goto nospc;
	}

 found:
	/*
	 * Insert this entry _before_ the one we found.
	 */
	list_add_tail(&new->vm_list, &c->vm_list);
	new->vm_start = addr;
	new->vm_end = addr + size;
	new->vm_active = 1;

	spin_unlock_irqrestore(&consistent_lock, flags);
	return new;

 nospc:
	spin_unlock_irqrestore(&consistent_lock, flags);
	kfree(new);
 out:
	return NULL;
}

static struct vm_region *vm_region_find(struct vm_region *head, unsigned long addr)
{
	struct vm_region *c;
	
	list_for_each_entry(c, &head->vm_list, vm_list) {
		if (c->vm_active && c->vm_start == addr)
			goto out;
	}
	c = NULL;
 out:
	return c;
}

#ifdef CONFIG_HUGETLB_PAGE
#error ARM Coherent DMA allocator does not (yet) support huge TLB
#endif

static void *
__dma_alloc(struct device *dev, size_t size, dma_addr_t *handle, gfp_t gfp)
{
#if defined(CONFIG_CELL)
	struct page *page;
	struct vm_region *c;
	unsigned long order;
	u64 mask = ISA_DMA_THRESHOLD, limit;
	void *ptr, *dma_phys;

	if (dev) {
		mask = dev->coherent_dma_mask;

		/*
		 * Sanity check the DMA mask - it must be non-zero, and
		 * must be able to be satisfied by a DMA allocation.
		 */
		if (mask == 0) {
			dev_warn(dev, "coherent DMA mask is unset\n");
			goto no_page;
		}

		if ((~mask) & ISA_DMA_THRESHOLD) {
			dev_warn(dev, "coherent DMA mask %#llx is smaller "
				 "than system GFP_DMA mask %#llx\n",
				 mask, (unsigned long long)ISA_DMA_THRESHOLD);
			goto no_page;
		}
	}

	/*
	 * Sanity check the allocation size.
	 */
	size = PAGE_ALIGN(size);
	limit = (mask + 1) & ~mask;
	if ((limit && size >= limit) ||
	    size >= (CONSISTENT_END - CONSISTENT_BASE)) {
		printk(KERN_WARNING "coherent allocation too big "
		       "(requested %#x mask %#llx)\n", size, mask);
		goto no_page;
	}

	order = get_order(size);

	if (mask != 0xffffffff)
		gfp |= GFP_DMA;

	page = alloc_pages(gfp, order);
	if (!page)
		goto no_page;

	/*
	 * Invalidate any data that might be lurking in the
	 * kernel direct-mapped region for device DMA.
	 */

	ptr = page_address(page);
	dma_phys = DMA_HEAP_VIRT_TO_PHYS(page_address(page));
	memset(ptr, 0, size);

	L4_CacheFlushRange(linux_space, (unsigned long)ptr, (unsigned long)ptr + size);
	outer_flush_range((unsigned long)dma_phys, (unsigned long)dma_phys + size);

	/*
	 * Allocate a virtual address in the consistent mapping region.
	 */
	c = vm_region_alloc(&consistent_head, size,
			    gfp & ~(__GFP_DMA | __GFP_HIGHMEM));
	if (c) {
		unsigned long vm_dma = c->vm_start;
		struct page *end = page + (1 << order);
		c->vm_pages = page;
		split_page(page, order);

		/*
		 * Set the "dma handle"
		 */
		*handle = (dma_addr_t) dma_phys;

		do {
			SetPageReserved(page);
			okl4_map_page_kernel(vm_dma, (unsigned long)page_address(page), L4_Readable|L4_Writable, L4_StrongOrderedMemory);
			page++;
			vm_dma += PAGE_SIZE;
		} while (size -= PAGE_SIZE);

		/*
		 * Free the otherwise unused pages.
		 */
		while (page < end) {
			__free_page(page);
			page++;
		}
		return (void *)c->vm_start;
	}

	if (page)
		__free_pages(page, order);
 no_page:
	*handle = ~0;
	return NULL;
#endif
#if defined(CONFIG_IGUANA)
panic("not implemented");
#endif
}

/*
 * Allocate DMA-coherent memory space and return both the kernel remapped
 * virtual and bus address for that space.
 */
void *
dma_alloc_coherent(struct device *dev, size_t size, dma_addr_t *handle, gfp_t gfp)
{
	return __dma_alloc(dev, size, handle, gfp);
}
EXPORT_SYMBOL(dma_alloc_coherent);

/*
 * Allocate a writecombining region, in much the same way as
 * dma_alloc_coherent above.
 */
void *
dma_alloc_writecombine(struct device *dev, size_t size, dma_addr_t *handle, gfp_t gfp)
{
	return __dma_alloc(dev, size, handle, gfp);
}
EXPORT_SYMBOL(dma_alloc_writecombine);

static int dma_mmap(struct device *dev, struct vm_area_struct *vma,
		    void *cpu_addr, dma_addr_t dma_addr, size_t size)
{
	unsigned long flags, user_size, kern_size;
	struct vm_region *c;
	int ret = -ENXIO;

	user_size = (vma->vm_end - vma->vm_start) >> PAGE_SHIFT;

	spin_lock_irqsave(&consistent_lock, flags);
	c = vm_region_find(&consistent_head, (unsigned long)cpu_addr);
	spin_unlock_irqrestore(&consistent_lock, flags);

	if (c) {
		unsigned long off = vma->vm_pgoff;

		kern_size = (c->vm_end - c->vm_start) >> PAGE_SHIFT;

		/*
		 * XXX - the page protection and the attributes
		 * XXX - should come from the VMA. -gl
		 * XXX - should sync up with what should
		 * XXX  - actually be in the pagetable.
		 */
		if (off < kern_size &&
		    user_size <= (kern_size - off)) {
			ret = remap_pfn_range(vma, vma->vm_start,
					      page_to_pfn(c->vm_pages) + off,
					      user_size << PAGE_SHIFT,
					      vma->vm_page_prot);
		}
	}

	return ret;
}

int dma_mmap_writecombine(struct device *dev, struct vm_area_struct *vma,
			  void *cpu_addr, dma_addr_t dma_addr, size_t size)
{
	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	return dma_mmap(dev, vma, cpu_addr, dma_addr, size);
}
EXPORT_SYMBOL(dma_mmap_writecombine);


/*
 * free a page as defined by the above mapping.
 * Must not be called with IRQs disabled.
 */
void dma_free_coherent(struct device *dev, size_t size, void *cpu_addr, dma_addr_t handle)
{
#if defined(CONFIG_CELL)
	struct vm_region *c;
	unsigned long flags, addr;
	struct page *page;

	WARN_ON(irqs_disabled());

	size = PAGE_ALIGN(size);

	spin_lock_irqsave(&consistent_lock, flags);
	c = vm_region_find(&consistent_head, (unsigned long)cpu_addr);
	if (!c)
		goto no_area;

	c->vm_active = 0;
	spin_unlock_irqrestore(&consistent_lock, flags);

	if ((c->vm_end - c->vm_start) != size) {
		printk(KERN_ERR "%s: freeing wrong coherent size (%ld != %d)\n",
		       __func__, c->vm_end - c->vm_start, size);
		dump_stack();
		size = c->vm_end - c->vm_start;
	}

	addr = c->vm_start;
	page = c->vm_pages;
	do {
		ClearPageReserved(page);
		okl4_unmap_page_kernel(addr);
		__free_page(page);
		addr += PAGE_SIZE;
		page++;
	} while (size -= PAGE_SIZE);

	spin_lock_irqsave(&consistent_lock, flags);
	list_del(&c->vm_list);
	spin_unlock_irqrestore(&consistent_lock, flags);

	kfree(c);
	return;

 no_area:
	spin_unlock_irqrestore(&consistent_lock, flags);
	printk(KERN_ERR "%s: trying to free invalid coherent area: %p\n",
	       __func__, cpu_addr);
	dump_stack();
#endif
#if defined(CONFIG_IGUANA)
panic("not implemented");
#endif
}
EXPORT_SYMBOL(dma_free_coherent);

#if defined(CONFIG_CELL)
/*
 * Initialise the consistent memory allocation.
 */
static int __init consistent_init(void)
{
	okl4_env_segment_t *dma_region;

	dma_region = okl4_env_get_segment("MAIN_DMA_HEAP_SEGMENT");
	assert(dma_region != NULL);
	dma_region_start = (uintptr_t)dma_region->virt_addr;
        CONSISTENT_END = (uintptr_t)(dma_region->virt_addr + dma_region->size);

	consistent_head.vm_start = CONSISTENT_BASE;
	consistent_head.vm_end = CONSISTENT_END;

        /* Sanity check size */
        if (dma_region->size  % SZ_2M) {
            panic(KERN_ERR "CONSISTENT_DMA_SIZE must be multiple of 2MiB\n");
        }
	return 0;

}

core_initcall(consistent_init);
#endif

/*
 * Make an area consistent for devices.
 * Note: Drivers should NOT use this function directly, as it will break
 * platforms with CONFIG_DMABOUNCE.
 * Use the driver DMA support - see dma-mapping.h (dma_sync_*)
 */
void dma_cache_maint(const void *start, size_t size, int direction)
{
#if 0
	BUG();
	const void *end = start + size;

	BUG_ON(!virt_addr_valid(start) || !virt_addr_valid(end - 1));

	switch (direction) {
	case DMA_FROM_DEVICE:		/* invalidate only */
		dmac_inv_range(start, end);
		outer_inv_range(__pa(start), __pa(end));
		break;
	case DMA_TO_DEVICE:		/* writeback only */
		dmac_clean_range(start, end);
		outer_clean_range(__pa(start), __pa(end));
		break;
	case DMA_BIDIRECTIONAL:		/* writeback and invalidate */
		dmac_flush_range(start, end);
		outer_flush_range(__pa(start), __pa(end));
		break;
	default:
		BUG();
	}
#else
	const void *end = start + size;

	/*
	 * This is highly inefficient! But we can only use this since
	 * OKL4 doesn't support clean or invalidate cache only yet.
	 * -cch
	 */
	L4_CacheFlushRange(linux_space, (unsigned long)start, (unsigned long)end);
	outer_flush_range(DMA_HEAP_VIRT_TO_PHYS(start), DMA_HEAP_VIRT_TO_PHYS(end));
#endif
}
EXPORT_SYMBOL(dma_cache_maint);

dma_addr_t dma_map_single(struct device *dev, void *cpu_addr, size_t size,
		enum dma_data_direction dir)
{
	dma_cache_maint(cpu_addr, size, dir);

	return DMA_HEAP_VIRT_TO_PHYS(cpu_addr);
}
EXPORT_SYMBOL(dma_map_single);

void dma_unmap_single(struct device *dev, dma_addr_t handle, size_t size,
		enum dma_data_direction dir)
{
	/* nothing to do */
}
EXPORT_SYMBOL(dma_unmap_single);


