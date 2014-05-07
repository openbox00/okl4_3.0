/*
 *  linux/arch/l4/mm/ioremap.c
 *
 *  Copyright (c) 2004, National ICT Australia
 *	Carl van Schaik
 *
 *  Taken from m32r version.
 *    (C) Copyright 2001, 2002  Hiroyuki Kondo
 *    (C) Copyright 1995 1996 Linus Torvalds
 *    (C) Copyright 2001 Ralf Baechle
 */

/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 */

#include <linux/module.h>
#include <asm/byteorder.h>

#include <linux/vmalloc.h>
#include <asm/io.h>
#include <asm/pgalloc.h>
#include <asm/cacheflush.h>
#include <asm/tlbflush.h>

#if defined(CONFIG_IGUANA)
#include <iguana/hardware.h>
#include <iguana/memsection.h>
#endif

static int
remap_area_pages(unsigned long address, unsigned long phys_addr,
		 unsigned long size, unsigned long flags, int l4attrib,
		 unsigned long memsection)
{
	int error = 0;

	/* Let us actually allocate the backed memory.. */
#if defined(CONFIG_IGUANA)
	hardware_back_memsection(memsection, phys_addr, l4attrib);
#endif

	return error;
}

/*
 * Generic mapping function (not visible outside):
 */

/*
 * Remap an arbitrary physical address space into the kernel virtual
 * address space. Needed when the kernel wants to access device/high addresses
 * directly.
 *
 * NOTE! We need to allow non-page-aligned mappings too: we will obviously
 * have to convert them into an offset in a page-aligned mapping, but the
 * caller shouldn't need to know that small detail.
 */

void __iomem *
__ioremap(unsigned long phys_addr, unsigned long size, unsigned long flags)
{
	void __iomem * addr;
#if defined(CONFIG_IGUANA)
	unsigned long area;
	unsigned long ms;
#endif
#if 0
	struct vm_struct * area;
#endif
	unsigned long offset, last_addr;

	/* Don't allow wraparound or zero size */
	last_addr = phys_addr + size - 1;
	if (!size || last_addr < phys_addr)
		return NULL;

	/*
	 * Mappings have to be page-aligned
	 */
	offset = phys_addr & ~PAGE_MASK;
	phys_addr &= PAGE_MASK;
	size = PAGE_ALIGN(last_addr + 1) - phys_addr;

	/*
	 * Ok, go for it..
	 */
#if 0
	area = get_vm_area(size, VM_IOREMAP);
	if (!area)
		return NULL;
	area->phys_addr = phys_addr;
	addr = (void __iomem *) area->addr;
	if (remap_area_pages((unsigned long)addr, phys_addr, size, flags)) {
		vunmap((void __force *) addr);
#endif

#if defined(CONFIG_IGUANA)
	ms = memsection_create_user(size, &area);

	if (!area)
		return NULL;
#elif defined(CONFIG_CELL)
return (void __iomem *) (offset + phys_addr);
	return NULL;
#endif

#if defined(CONFIG_IGUANA)
	addr = (void __iomem *) area;
	if (remap_area_pages((unsigned long)addr, phys_addr, size, flags, 
	    L4_IOMemory, ms)) {
		memsection_delete(ms);
		return NULL;
	}
#endif

	return (void __iomem *) (offset + (char __iomem *)addr);
}

/*
 * As per ioremap, but ensures that it is uncached.
 */
void __iomem *
ioremap_nocache(unsigned long phys_addr, unsigned long size)
{
	void __iomem * addr;
#if defined(CONFIG_IGUANA)
	unsigned long area;
	unsigned long ms;
#endif
#if 0
	struct vm_struct * area;
#endif
	unsigned long offset, last_addr;

	/* Don't allow wraparound or zero size */
	last_addr = phys_addr + size - 1;
	if (!size || last_addr < phys_addr)
		return NULL;

	/*
	 * Mappings have to be page-aligned
	 */
	offset = phys_addr & ~PAGE_MASK;
	phys_addr &= PAGE_MASK;
	size = PAGE_ALIGN(last_addr + 1) - phys_addr;

	/*
	 * Ok, go for it..
	 */
#if 0
	area = get_vm_area(size, VM_IOREMAP);
	if (!area)
		return NULL;
	area->phys_addr = phys_addr;
	addr = (void __iomem *) area->addr;
	if (remap_area_pages((unsigned long)addr, phys_addr, size, flags)) {
		vunmap((void __force *) addr);
#endif

#if defined(CONFIG_IGUANA)
	ms = memsection_create_user(size, &area);

	if (!area)
		return NULL;
#elif defined(CONFIG_CELL)
	return NULL;
#endif

#if defined(CONFIG_IGUANA)
	addr = (void __iomem *) area;
	if (remap_area_pages((unsigned long)addr, phys_addr, size, 0/*flags*/,
	    L4_UncachedMemory, ms)) {
		memsection_delete(ms);
		return NULL;
	}
#endif

	return (void __iomem *) (offset + (char __iomem *)addr);
}



void iounmap(volatile void __iomem *addr)
{
#if defined(CONFIG_IGUANA)
	unsigned long ms;
	thread_ref_t dummy;

	ms = memsection_lookup((unsigned long)addr, &dummy);

	if (unlikely(ms == -1)) {
		printk(KERN_ERR "Trying to iounmap() nonexistent vm area (%p)\n",
				addr);
		WARN_ON(1);
		return;
	}
	memsection_delete(ms);

#if 0
	struct vm_struct *area;
	area = remove_vm_area((void*)addr);
	if (unlikely(!area)) {
		printk(KERN_ERR "Trying to iounmap() nonexistent vm area (%p)\n",
				addr);
		WARN_ON(1);
		return;
	}
	// XXX unmap

	kfree(area);
#endif
#elif defined(CONFIG_CELL)
#endif	/*CONFIG_CELL*/
	return;
}

