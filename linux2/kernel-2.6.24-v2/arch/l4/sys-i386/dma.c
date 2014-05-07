/*
 * Dynamic DMA mapping support.
 */

#include <linux/types.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>
#include <asm/io.h>
#include <asm/page.h>

#include <iguana/thread.h>
#include <iguana/memsection.h>
#include <iguana/hardware.h>
#include <iguana/physmem.h>

/* Allocate consistent (uncached, or the appearance of it) memory */
/* We ignore the flags at the moment */
/*
 * XXX this only works because the physical memory is mapped
 * at a constant offset from the memsection heap -gl
 *
 * This code looks duplicated for scatter gather translation but
 * is deliberately duplicated as this is strictly incorrect, see
 * comment in the above paragraph, hence the common code is deliberately
 * not put into a helper function.
 */
void *dma_alloc_coherent(struct device *dev, size_t size,
			   dma_addr_t *dma_handle, gfp_t gfp)
{
	int order;
	void *ret;
	uint32_t page_addr, phys_offset;

	order = get_order(size);
	gfp &= ~(__GFP_DMA|__GFP_HIGHMEM);
	ret = (void *)__get_free_pages(gfp, order);
	if (ret != NULL) {
		memset(ret, 0, size);
		page_addr = virt_to_phys(ret);
		phys_offset = page_addr - dma_heap_base;

		*dma_handle = dma_heap_phys_base + phys_offset;
	}

	flush_write_buffers();

	return ret;

}
EXPORT_SYMBOL(dma_alloc_coherent);

/*
 * XXX this only works because the physical memory is mapped
 * at a constant offset from the memsection heap -gl
 */
void dma_free_coherent(struct device *dev, size_t size,
			 void *vaddr, dma_addr_t dma_handle)
{
	int order;

	order = get_order(size);
	free_pages((unsigned long)vaddr, order);
}
EXPORT_SYMBOL(dma_free_coherent);
