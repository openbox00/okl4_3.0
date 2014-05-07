#include "l4.h"
#include "assert.h"

#include "linux/mm.h"
#include "asm/page.h"
#include "asm/io.h"
#include "asm/pgalloc.h"
#include "asm/tlbflush.h"
#include "asm/okl4.h"

#include <okl4/env.h>

//#define DEBUG_LOOKUP_PTABS

// Used by new depriv mapping code
extern unsigned long vmalloc_ms_base;
extern unsigned long vmalloc_ms_size;
extern unsigned long vmalloc_ms_virt_to_phys_offset;

pte_t *
lookup_pte(pgd_t *page_dir, unsigned long pf_address)
{
	/*
	 * find the page table entry within the page table hierarchy
	 */
	pte_t *pte = NULL;
	pgd_t *pgd = page_dir + pgd_index(pf_address);

#ifdef DEBUG_LOOKUP_PTABS
	if ((int)page_dir < 0x1000) {
		printk("%s: page_dir=%x\n", __func__, (int)page_dir);
		enter_kdebug("page_dir<4096");
	}
	printk("%s: %lx pdir = %p\n", __func__, pf_address, pgd);
#endif
	if (pgd_present(*pgd)) {
		pmd_t *pmd = pmd_offset(pgd, pf_address);
#ifdef DEBUG_LOOKUP_PTABS
		printk("pgd_present(*%x) is true\n", pgd);
		printk(" pmd = %p\n", pmd);
#endif
		if (pmd_present(*pmd)) {
#ifdef DEBUG_LOOKUP_PTABS
			printk("pmd_present(*%x) is true\n", pmd);
#endif
			pte = pte_offset_map(pmd, pf_address);
		}
	}
#ifdef DEBUG_LOOKUP_PTABS
	printk("returning:  pte = %p\n", pte);
#endif
	return pte;
}

static inline unsigned long
ptes_same_pmd(pte_t *a, pte_t *b)
{
	unsigned long a_addr = (unsigned long)a;
	unsigned long b_addr = (unsigned long)b;

	return ((a_addr & PAGE_MASK) == (b_addr & PAGE_MASK));
}

/* A semi-optimized get next pte */
static inline pte_t * 
pte_next(pgd_t *page_dir, unsigned long pf_address, pte_t *last)
{
	pte_t *pte = last + 1;

	if ((!last) || !ptes_same_pmd(pte, last))
	{
		pte = lookup_pte(page_dir, pf_address);
	}
#ifdef DEBUG_LOOKUP_PTABS
	else
		printk("%s pte = %p\n", __func__, pte);
#endif

	return pte;
}


/* Flush a page from the user's address space */
void flush_tlb_page(struct vm_area_struct *vma, unsigned long address)
{
	address &= PAGE_MASK;

#ifdef ARM_PID_RELOC
	if (address < 0x2000000) {
		unsigned long offset = (unsigned long)vma->vm_mm->context.pid << 25;

		address += offset;
	}
#endif

	{
#if defined(CONFIG_CELL)
	    okl4_unmap_page(&vma->vm_mm->context, address);
#elif defined(CONFIG_IGUANA)
	    L4_Fpage_t fpage;

	    fpage = L4_FpageLog2(address, PAGE_SHIFT);
	    eas_unmap(vma->vm_mm->context.eas, fpage);
#else
#error
#endif
	}
}

/* Flush a range of memory from the kernel's virtual address space */
void flush_tlb_kernel_range(unsigned long start, unsigned long end)
{
#if 0
	unsigned long base, count;
	L4_Fpage_t fpage;

	count = 0;
	base = start & PAGE_MASK;

	while (1) {
		fpage = L4_FpageLog2(base, PAGE_SHIFT);

		L4_Set_Rights(&fpage, L4_FullyAccessible);  /* To unmap */
		L4_LoadMR(count++, fpage.raw);

		if (count == __L4_NUM_MRS)
		{
			L4_Unmap(count-1);
			count = 0;
		}

		base += PAGE_SIZE;
		if (base >= end)
		{
			if (count)
				L4_Unmap(count-1);
			return;
		}
	}
#endif
}

/* Flush a single tlb entry in the kernel */
void __flush_tlb_one(unsigned long addr)
{
	addr &= PAGE_MASK;
	flush_tlb_kernel_range(addr, addr + PAGE_SIZE);
}

/* Flush a range of tlb entries in the user's address space
 * in the address range given.
 * This could be optimized to factorize the area into 2^n
 * fpages to reduce the number of upmaps, but the algorithm
 * looks to be too complicated and may cause slowdown.
 */
void flush_tlb_range(struct vm_area_struct *vma, unsigned long start, 
		     unsigned long end)
{
	unsigned long address = start;

#ifdef ARM_PID_RELOC
	if (address < 0x2000000) {
		unsigned long offset = (unsigned long)vma->vm_mm->context.pid << 25;

		address += offset;
		end += offset;
	}
#endif

	while (address < end) {
#if defined(CONFIG_IGUANA)
		L4_Fpage_t fpage;

		fpage = L4_FpageLog2(address, PAGE_SHIFT);
		eas_unmap(vma->vm_mm->context.eas, fpage);
#elif defined(CONFIG_CELL)
		okl4_unmap_page(&vma->vm_mm->context, address);
#else
#error
#endif
		address += PAGE_SIZE;
	}
}

void remove_tlb_pte(pte_t *ptep, unsigned long address)
{
	if (pte_mapped(*ptep)) {
		struct mm_struct * curr_mm = current->mm;

#ifdef ARM_PID_RELOC
		if (address < 0x2000000)
			address += ((unsigned long)curr_mm->context.pid << 25);
#endif
#if defined(CONFIG_CELL)
		okl4_unmap_page(&curr_mm->context, address);
#elif defined(CONFIG_IGUANA)
		{
			L4_Fpage_t fpage;
			fpage = L4_FpageLog2(address, PAGE_SHIFT);
			eas_unmap(curr_mm->context.eas, fpage);
		}
#else
#error
#endif
		*ptep = pte_mkunmapped(*ptep);
	}
}

void flush_tlb_mm(struct mm_struct *mm)
{
	unsigned long shift, base = 0;
#ifdef ARM_PID_RELOC
 	struct vm_area_struct *vma;

 	shift = 25;
	base = ((unsigned long)mm->context.pid << shift);
#else
	shift = TASK_SHIFT;
#endif
#if defined(CONFIG_CELL)
	okl4_unmap_page_size(&mm->context, base, shift);
#elif defined(CONFIG_IGUANA)
	{
		L4_Fpage_t fpage;
		fpage = L4_FpageLog2(base, shift);
		eas_unmap(mm->context.eas, fpage);
	}
#endif
#ifdef ARM_PID_RELOC
	/* Walk through the list of VMAs and flush those
	 * that are outside the PID relocation region
	 */
	vma = mm->mmap;
	while(vma) {
		if (vma->vm_start >= 0x2000000UL)
			flush_tlb_range(vma, vma->vm_start, vma->vm_end);
		vma = vma->vm_next;
	}
#endif
}

/* Update the user's address space with the new mapping
 * This does not do the L4 map, but the message is loaded
 * and the reply in the syscall loop handles this.
 */
void update_mmu_cache(struct vm_area_struct *vma,
	unsigned long address, pte_t *ptep, pte_t pte)
{
	unsigned long phys;

	if (pte_present(pte))
	{
		unsigned long attrib = L4_DefaultMemory;
		L4_Fpage_t fpage;

		if (unlikely(pte_val(pte) & _PAGE_ATTRIBS)) {
			switch (pte_val(pte) & _PAGE_ATTRIBS) {
			case _PAGE_WRITECOMBINE:
			    attrib = L4_IOCombinedMemory; break;
			case _PAGE_WRITETHROUGH:
			    attrib = L4_WriteThroughMemory; break;
			case _PAGE_NOCACHE:
			default:
			    attrib = L4_UncachedMemory; break;
			}
		}

		phys = pte_pfn(pte) << PAGE_SHIFT;

		fpage = (L4_Fpage_t)(L4_FpageLog2((unsigned long)phys, 
		    PAGE_SHIFT).raw + pte_access(pte));

#ifdef ARM_PID_RELOC
		if (address < 0x2000000)
			address += ((unsigned long)vma->vm_mm->context.pid << 25);
#endif

#if defined(CONFIG_IGUANA)
		l4_map_page(&vma->vm_mm->context, fpage, address, attrib);
#elif defined(CONFIG_CELL)
		okl4_map_page(&vma->vm_mm->context, address, phys, 
			pte_access(pte), attrib);
#endif
		*ptep = pte_mkmapped(pte);
	}
}

void
tlb_modify(struct mm_struct *mm,
		 unsigned long address, pte_t *ptep)
{
	unsigned long phys;
	unsigned long attrib = L4_DefaultMemory;
	L4_Fpage_t fpage;
	pte_t pte = *ptep;

	phys = pte_pfn(pte) << PAGE_SHIFT;

	if (unlikely(pte_val(pte) & _PAGE_ATTRIBS)) {
		switch(pte_val(pte) & _PAGE_ATTRIBS) {
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

	fpage = (L4_Fpage_t) (L4_FpageLog2 (
				(unsigned long) phys, PAGE_SHIFT).raw + pte_access(pte));

#ifdef ARM_PID_RELOC
	if (address < 0x2000000)
		address += ((unsigned long)mm->context.pid << 25);
#endif

#if defined(CONFIG_IGUANA)
	l4_map_page(&mm->context, fpage, address, attrib);
#elif defined(CONFIG_CELL)
	okl4_map_page(&mm->context, address, phys, pte_access(pte), attrib);
#endif
}

#if defined(CONFIG_IGUANA)
void
l4_map_page(mm_context_t *context, L4_Fpage_t fpage, unsigned long address,
		unsigned long attrib)
{
	unsigned long dest_addr, src_addr;
	L4_Fpage_t vpage;
	L4_PhysDesc_t pdesc;
	int rwx;

	src_addr = L4_Address(fpage);
#if 0
	if (src_addr >= vmalloc_ms_base && src_addr <= vmalloc_ms_base + vmalloc_ms_size) {
		// Within the vmalloc bounds so use the new depriv mapping
		dest_addr = address & 0xfffff000;
		vpage = L4_Fpage(dest_addr, L4_Size(fpage));
		pdesc = L4_PhysDesc(src_addr + vmalloc_ms_virt_to_phys_offset, attrib);
		rwx = L4_Rights(fpage);

		L4_FpageAddRightsTo(&vpage, rwx);
		L4_MapFpage(context->space_id, vpage, pdesc);
	} else {
#endif

		eas_map(context->eas, fpage, address, attrib);
//	}
}
#endif

#if defined(CONFIG_CELL)
unsigned long last_vstart = -1UL;
unsigned long last_vend, last_seg;

int
okl4_find_segment(unsigned long vaddr, unsigned long *offset, unsigned long *seg)
{
	okl4_env_segments_t *segments = OKL4_ENV_GET_SEGMENTS("SEGMENTS");
	unsigned long i;

	assert(segments);
	for (i = 0; i < segments->num_segments; i++) {
	    if (vaddr >= segments->segments[i].virt_addr &&
		    vaddr <= (segments->segments[i].virt_addr + 
			segments->segments[i].size - 1)) {
			*offset = vaddr - segments->segments[i].virt_addr;
			*seg = segments->segments[i].segment;

			/* Cache lookup */
			last_vstart = segments->segments[i].virt_addr;
			last_vend = last_vstart + segments->segments[i].size - 1;
			last_seg = segments->segments[i].segment;
			return 1;
		}
	}

	return 0;
}

void
okl4_map_page(mm_context_t *context, unsigned long virt_addr, unsigned long src_addr,
		unsigned rwx, unsigned long attrib)
{
	unsigned long seg;
	L4_MapItem_t map;

	virt_addr &= PAGE_MASK;

	if (likely((last_vstart <= src_addr) && (src_addr <= last_vend))) {
		src_addr = src_addr - last_vstart;
		seg = last_seg;
	} else {
		if (!okl4_find_segment(src_addr, &src_addr, &seg))
			panic("non existent segment");
	}

	L4_MapItem_Map(&map, seg, src_addr, virt_addr, PAGE_SHIFT, attrib, rwx);
	if (L4_ProcessMapItem(context->space_id, map) == 0)
		panic("L4_ProcessMapItem: map failed");
}

void
okl4_map_page_kernel(unsigned long virt_addr, unsigned long src_addr,
		unsigned rwx, unsigned long attrib)
{
	unsigned long seg;
	L4_MapItem_t map;

	virt_addr &= PAGE_MASK;

	if (!okl4_find_segment(src_addr, &src_addr, &seg))
		panic("non existent segment");

	L4_MapItem_Map(&map, seg, src_addr, virt_addr, PAGE_SHIFT, attrib, rwx);
	if (L4_ProcessMapItem(linux_space, map) == 0)
		panic("L4_ProcessMapItem: map failed");
}

void
okl4_unmap_page(mm_context_t *context, unsigned long address)
{
	L4_MapItem_t	unmap;

	address = address & PAGE_MASK;
	L4_MapItem_Unmap(&unmap, address, PAGE_SHIFT);
	L4_ProcessMapItem(context->space_id, unmap);
}

void
okl4_unmap_page_kernel(unsigned long address)
{
	L4_MapItem_t	unmap;

	address = address & PAGE_MASK;
	L4_MapItem_Unmap(&unmap, address, PAGE_SHIFT);
	L4_ProcessMapItem(linux_space, unmap);
}

#if 0 // need kernel support
void
okl4_unmap_pages(mm_context_t *context, unsigned long address, unsigned long pages)
{
	L4_MapItem_t	unmap;

	address = address & PAGE_MASK;
	L4_MapItem_Unmap(&unmap, address, PAGE_SHIFT);
	if (unlikely(pages > 1)) {
		L4_MapItem_SetMultiplePages(&unmap, pages);
	}
	L4_ProcessMapItem(context->space_id, unmap);
}
#endif

void
okl4_unmap_page_size(mm_context_t *context, unsigned long address, unsigned long page_shift)
{
	L4_MapItem_t	unmap;

	address = address & (~((1UL << page_shift) - 1));
	L4_MapItem_Unmap(&unmap, address, page_shift);
	L4_ProcessMapItem(context->space_id, unmap);
}

#endif
