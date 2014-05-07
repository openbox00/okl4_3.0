#include "l4.h"
#include "l4/misc.h"
#include "assert.h"

#include "linux/mm.h"
#include "linux/module.h"

#if defined(CONFIG_IGUANA)
#include "iguana/memsection.h"
#endif

#if defined(CONFIG_CELL)
#include <asm/okl4.h>
#endif

void flush_cache_all (void) { L4_CacheFlushAll(); }
void flush_cache_mm (void) { L4_CacheFlushAll(); }

/*
 * Cache flush a single page of user memory
 */
void flush_cache_page(struct vm_area_struct *vma, unsigned long page, unsigned long pfn)
{
#ifndef CONFIG_ARCH_I386
#ifdef ARM_PID_RELOC
	unsigned long offset = (unsigned long)vma->vm_mm->context.pid << 25;
	if (page < 0x2000000)
		page += offset;
#endif
	page = page & PAGE_MASK;
	L4_CacheFlushRange(vma->vm_mm->context.space_id, page, page+PAGE_SIZE);
#endif
}

/*
 * Cache flush a range of user memory
 */
void flush_cache_range(struct vm_area_struct *vma, unsigned long start, unsigned long end)
{
#ifndef CONFIG_ARCH_I386
#ifdef ARM_PID_RELOC
	unsigned long offset = (unsigned long)vma->vm_mm->context.pid << 25;
	if (start < 0x2000000) {
		start += offset;
		end += offset;
	}
#endif
	L4_CacheFlushRange(vma->vm_mm->context.space_id, start, end);
#endif
}

/*
 * Cache flush a page of kernel memory
 */
void flush_dcache_page(struct page *page)
{
#ifndef CONFIG_ARCH_I386
	unsigned long addr = (unsigned long)page_address(page);
	L4_CacheFlushDRange(L4_nilspace, addr, addr+PAGE_SIZE);
#endif
}

void flush_icache_user_range(struct vm_area_struct *vma,
	struct page *page, unsigned long addr, unsigned long len)
{
#ifndef CONFIG_ARCH_I386
	unsigned long end = addr + len;
#ifdef ARM_PID_RELOC
	unsigned long offset = (unsigned long)vma->vm_mm->context.pid << 25;
	if (addr < 0x2000000) {
		addr += offset;
		end += offset;
	}
#endif
	L4_CacheFlushIRange(vma->vm_mm->context.space_id, addr, end);
#endif
}

/*
 * Cache flush (instruction-cache) a page of kernel memory
 */
void flush_icache_range(unsigned long start, unsigned long end)
{
#ifndef CONFIG_ARCH_I386
	L4_CacheFlushRange(L4_nilspace, start, end);
#endif
}

pte_t *lookup_pte(pgd_t *page_dir, unsigned long address);

/*
 * Cache flush a page of kernel virtual (vmalloc) memory
 */
void flush_cache_vmap(unsigned long start, unsigned long end)
{
	pte_t *pte;
	unsigned long address, phys;
	unsigned long attrib = L4_DefaultMemory;

	/* FIXME: This is ugly, may not be needed anymore?
	 *        Map pages to ensure they exist - this was used to "fault in"
	 *        the vmalloc area.
	 */
	for (address = start; address < end; address += PAGE_SIZE) {
		pte = lookup_pte(init_mm.pgd, address);
		assert(pte != NULL);
		phys = pte_pfn(*pte) << PAGE_SHIFT;
#if defined(CONFIG_IGUANA)
		memsection_page_map(vmalloc_memsect, L4_Fpage(phys, PAGE_SIZE), L4_Fpage(address, PAGE_SIZE));
#else
		if (unlikely(pte_val(*pte) & _PAGE_ATTRIBS)) {
			switch(pte_val(*pte) & _PAGE_ATTRIBS) {
			case _PAGE_WRITECOMBINE:
				attrib = L4_IOCombinedMemory;
				break;
			case _PAGE_WRITETHROUGH:
				attrib = L4_WriteThroughMemory;
				break;
			case _PAGE_NOCACHE:
			default:
				attrib = L4_UncachedMemory;
				break;
			}
		}
		okl4_map_page_kernel(address, phys, pte_access(*pte),
			attrib);
#endif
	}
	flush_cache_all();
}

/*
 * Cache flush a page of kernel virtual (vmalloc) memory
 */
void flush_cache_vunmap(unsigned long start, unsigned long end)
{
	unsigned long address;

	flush_cache_all();
	/* XXX unmap vmalloc area: need to go elsewhere!! */
	for (address = start; address < end; address += PAGE_SIZE)
#if defined(CONFIG_IGUANA)
		memsection_page_unmap(vmalloc_memsect, L4_Fpage(address, PAGE_SIZE));
#else
		okl4_unmap_page_kernel(address);
#endif
}

/*
 * Flush L2+ cache of given physical memory range.
 */
void outer_flush_range(unsigned long phys_start, unsigned long phys_end)
{
	L4_Outer_CacheFlushRange(phys_start, phys_end);
}
