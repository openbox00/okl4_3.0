#ifndef _L4_PGTABLE_H
#define _L4_PGTABLE_H

#include <asm-generic/4level-fixup.h>

/*
 * This file contains the functions and defines necessary to modify and use
 * the L4 page table tree.
 *
 * This hopefully works with any standard L4 page-size, as defined
 * in <asm/page.h> (currently 4096).
 */

#include <linux/mmzone.h>
#include <linux/sched.h>	/* struct mm */

#include <asm/page.h>
#include <asm/processor.h>	/* For TASK_SIZE */

#define FIRST_USER_ADDRESS	0

/* For the PTE bits*/
#if PAGE_SHIFT < 12
#error a minimum of 4k pages are supported
#endif

extern void flush_tlb_pte(pte_t*, unsigned long);

/* Certain architectures need to do special things when PTEs
 * within a page table are directly modified.  Thus, the following
 * hook is made available.
 */
#define set_pte(pteptr, pteval)	do {		\
	pte_t newpte = pteval;			\
	(*(pteptr)) = (newpte);			\
} while (0);
#define set_pte_at(mm,addr,ptep,pteval) set_pte(ptep,pteval)
#define set_pte_atomic(pteptr, pteval) set_pte(pteptr,pteval)
#define set_pmd(pmdptr, pmdval) (*(pmdptr) = (pmdval))

/*
 * Establish a new mapping:
 *
 * We hold the mm semaphore for reading and vma->vm_mm->page_table_lock
 */
#define ptep_establish(__vma, __address, __ptep, __entry)               \
do {                                                                    \
	pte_t newpte = __entry;						\
        (*(__ptep)) = (newpte);						\
} while (0)

/*
 * Largely same as above, but only sets the access flags (dirty,
 * accessed, and writable). Furthermore, we know it always gets set
 * to a "more permissive" setting, which allows most architectures
 * to optimize this.
 */
#define __HAVE_ARCH_PTEP_SET_ACCESS_FLAGS
#define ptep_set_access_flags(__vma, __address, __ptep, __entry, __dirty) \
({				  					  \
	set_pte(__ptep, __entry);					  \
	1;/* assume changed: need to think more -gl*/		  	\
})

#define __HAVE_ARCH_PTEP_SET_WRPROTECT
#define ptep_set_wrprotect(__mm, __address, __ptep)			\
do {									\
	pte_t __pteval = *(__ptep);					\
	if (pte_mapped(__pteval) && pte_write(__pteval)) {		\
		__pteval = pte_wrprotect(__pteval);			\
		set_pte(__ptep, __pteval);				\
		tlb_modify(__mm, __address, __ptep);			\
	}								\
} while (0)

/*
 * XXX what about the other API in pgtable.h??
 * XXX I think, at worst it can hurt us performance-wise when
 * XXX I took a look, because there were some extra seemingly
 * XXX unnecessary TLB flush  (bad for L4 design)
 *
 * -gl
 */

#ifdef L4_64BIT

#define SIZEOF_PTR_LOG2		3

#include <asm/pgtable-3level.h>

#else	/* 32-bit */

#define SIZEOF_PTR_LOG2		2

#include <asm/pgtable-2level.h>

#endif	/* L4_64BIT */


#define USER_PTRS_PER_PGD	((TASK_SIZE + (PGDIR_SIZE - 1)) / PGDIR_SIZE)
#define FIRST_USER_PGD_NR	0

#if defined(CONFIG_IGUANA)
extern memsection_ref_t vmalloc_memsect;
#endif

extern uintptr_t vmalloc_start;
#define VMALLOC_START	vmalloc_start
extern uintptr_t vmalloc_end;
#define VMALLOC_END	vmalloc_end

/*
 * Page table bits
 *  When L4_64BIT
 *	In a PTE, the physical page number is the top 32 bit
 *	Bottom 32-bits are page flags as below
 *  else
 *	In a PTE, the physical page number is the top 20 bits
 *	Bottom 12-bits are page flags as below
 * FIXME - are there any L4 specific things that should go here?
 */
#define _PAGE_VALID	0x001	/* Page is valid */
#define _PAGE_FILE	0x002
#define _PAGE_ACCESSED	0x004	/* Page has been accessed? */
#define _PAGE_DIRTY	0x008

#define _PAGE_EXECUTE	0x010	/* Page is executeable	*/
#define _PAGE_WRITE	0x020	/* Page is writeable	*/
#define _PAGE_READ	0x040	/* Page is readable	*/
#define _PAGE_KERNEL	0x080	/* Page is priviledged	*/

#define _PAGE_ATTRIBS	0x300	/* Page attributes, arch specific, 0 = Default */

#define _PAGE_MAPPED	0x800	/* Page is mapped in the L4 address space */

#define _PAGE_FLAGS	0xfff	/* All the above flags	*/


#define __DIRTY_BITS	(_PAGE_DIRTY | _PAGE_WRITE)
#define __ACCESS_BITS	(_PAGE_ACCESSED | _PAGE_READ)

#ifdef L4_64BIT
/* PFN is in top 32-bits */
#define _PFN_MASK	0xFFFFFFFF00000000
#define PTE_PFN_OFFSET	32
#define _PTE_SIZE	64
#else /* 32-bit */
/* PFN is in top 20-bits */
#define _PFN_MASK	0xFFFFF000
#define PTE_PFN_OFFSET	12
#define _PTE_SIZE	32
#endif

#define _PAGE_TABLE	(_PAGE_VALID | __DIRTY_BITS | __ACCESS_BITS)
#define _PAGE_CHG_MASK	(_PFN_MASK | _PAGE_ACCESSED | _PAGE_DIRTY)

/*
 * All the normal masks have the "page accessed" bits on, as any time they are used,
 * the page is accessed. They are cleared only by the page-out routines
 */
#define PAGE_NONE	__pgprot(_PAGE_VALID | _PAGE_ACCESSED )
#define PAGE_SHARED	__pgprot(_PAGE_VALID | __ACCESS_BITS | __DIRTY_BITS)
#define PAGE_SHARED_EXC	__pgprot(_PAGE_VALID | __ACCESS_BITS | __DIRTY_BITS | _PAGE_EXECUTE)
#define PAGE_READ_EXC	__pgprot(_PAGE_VALID | __ACCESS_BITS | _PAGE_EXECUTE)
#define PAGE_READONLY	__pgprot(_PAGE_VALID | __ACCESS_BITS)
#define PAGE_WRITE_EXC	__pgprot(_PAGE_VALID | _PAGE_ACCESSED | __DIRTY_BITS | _PAGE_EXECUTE)
#define PAGE_WRITEONLY	__pgprot(_PAGE_VALID | _PAGE_ACCESSED | __DIRTY_BITS)
#define PAGE_EXECONLY	__pgprot(_PAGE_VALID | _PAGE_ACCESSED | _PAGE_EXECUTE)
#define PAGE_KERNEL	__pgprot(_PAGE_VALID | __ACCESS_BITS | __DIRTY_BITS | _PAGE_KERNEL | _PAGE_EXECUTE)
#define PAGE_KERNEL_RO	__pgprot(_PAGE_VALID | __ACCESS_BITS | _PAGE_KERNEL)
#define PAGE_KERNEL_EXC	__pgprot(_PAGE_VALID | _PAGE_ACCESSED | _PAGE_EXECUTE | _PAGE_KERNEL)

#define PAGE_COPY	PAGE_READONLY	/* copy on write */

/*
 * If the architecture does not handle certain combinations of
 * access rights, its L4s responsibility to extend rights
 */
	/* xwr */
#define	__P000	PAGE_NONE
#define __P001	PAGE_READONLY
#define __P010	PAGE_READONLY	/* copy on write */
#define __P011	PAGE_READONLY	/* copy on write */
#define __P100	PAGE_EXECONLY
#define __P101	PAGE_READ_EXC
#define __P110	PAGE_READ_EXC	/* copy on write */
#define __P111	PAGE_READ_EXC	/* copy on write */

	/* xwr */
#define	__S000	PAGE_NONE
#define __S001	PAGE_READONLY
#define __S010	PAGE_WRITEONLY
#define __S011	PAGE_SHARED
#define __S100	PAGE_EXECONLY
#define __S101	PAGE_READ_EXC
#define __S110	PAGE_WRITE_EXC
#define __S111	PAGE_SHARED_EXC

/* zero page used for uninitialized stuff */
extern unsigned long empty_zero_page;
extern unsigned long zero_page_mask;

/*
 * ZERO_PAGE is a global shared page that is always zero:  used
 * for zero-mapped memory areas etc..
 */
#define ZERO_PAGE(vaddr) \
	(virt_to_page(empty_zero_page + (((unsigned long)(vaddr)) & zero_page_mask)))

/* number of bits that fit into a memory pointer */
#define BITS_PER_PTR			(8*sizeof(unsigned long))

/* to align the pointer to a pointer address */
#define PTR_MASK			(~(sizeof(void*)-1))

/*
 * Conversion functions:  convert a page and protection to a page entry,
 * and a page entry and page directory to the page they refer to.
 */

#define pte_pfn(pte)		(pte_val(pte) >> PTE_PFN_OFFSET)
#define pte_page(pte)		pfn_to_page(pte_pfn(pte))
#define pte_access(pte)		((pte_val(pte) >> 4) & 0x07)
#define pte_mode(pte)		(pte_val(pte) & 0x71)
#define mk_pte(page, pgprot)	pfn_pte(page_to_pfn(page), pgprot)

extern inline pte_t pfn_pte(unsigned long physpfn, pgprot_t pgprot)
{ pte_t pte; pte_val(pte) = (physpfn << PTE_PFN_OFFSET) | pgprot_val(pgprot); return pte; }

extern inline pte_t pte_modify(pte_t pte, pgprot_t newprot)
{ pte_val(pte) = (pte_val(pte) & _PAGE_CHG_MASK) | pgprot_val(newprot); return pte; }

extern inline void pmd_set(pmd_t * pmdp, pte_t * ptep)
{ pmd_val(*pmdp) = _PAGE_TABLE | ((((unsigned long) ptep) - PAGE_OFFSET) << (PTE_PFN_OFFSET-PAGE_SHIFT)); }

extern inline void pgd_set(pgd_t * pgdp, pmd_t * pmdp)
{ pgd_val(*pgdp) = _PAGE_TABLE | ((((unsigned long) pmdp) - PAGE_OFFSET) << (PTE_PFN_OFFSET-PAGE_SHIFT)); }


extern inline unsigned long
pmd_page_kernel(pmd_t pmd)
{
	return ((pmd_val(pmd) & _PFN_MASK) >> (PTE_PFN_OFFSET-PAGE_SHIFT)) + PAGE_OFFSET;
}

#ifndef CONFIG_DISCONTIGMEM
#define pmd_page(pmd)		(pfn_to_page(pmd_val(pmd) >> PTE_PFN_OFFSET))
#endif

extern inline unsigned long pgd_page(pgd_t pgd)
{ return PAGE_OFFSET + ((pgd_val(pgd) & _PFN_MASK) >> (PTE_PFN_OFFSET-PAGE_SHIFT)); }

extern inline int pte_none(pte_t pte)		{ return !(pte_val(pte) & _PAGE_FLAGS); }
extern inline int pte_present(pte_t pte)	{ return pte_val(pte) & _PAGE_VALID; }
extern inline int pte_mapped(pte_t pte)		{ return pte_val(pte) & _PAGE_MAPPED; }

extern inline int pmd_none(pmd_t pmd)		{ return !pmd_val(pmd); }
extern inline int pmd_bad(pmd_t pmd)		{ return (pmd_val(pmd) & ~_PFN_MASK) != _PAGE_TABLE; }
extern inline int pmd_present(pmd_t pmd)	{ return pmd_val(pmd) & _PAGE_VALID; }
extern inline void pmd_clear(pmd_t * pmdp)	{ pmd_val(*pmdp) = 0; }

/*
 * The following only work if pte_present() is true.
 * Undefined behaviour if not..
 */
extern inline int pte_read(pte_t pte)		{ return pte_val(pte) & _PAGE_READ; }
extern inline int pte_write(pte_t pte)		{ return pte_val(pte) & _PAGE_WRITE; }
extern inline int pte_exec(pte_t pte)		{ return pte_val(pte) & _PAGE_EXECUTE; }
extern inline int pte_dirty(pte_t pte)		{ return pte_val(pte) & _PAGE_DIRTY; }
extern inline int pte_young(pte_t pte)		{ return pte_val(pte) & _PAGE_ACCESSED; }
extern inline int pte_file(pte_t pte)		{ return pte_val(pte) & _PAGE_FILE; }

extern inline pte_t pte_wrprotect(pte_t pte)	{ pte_val(pte) &= ~_PAGE_WRITE;	    return pte; }
extern inline pte_t pte_rdprotect(pte_t pte)	{ pte_val(pte) &= ~_PAGE_READ;	    return pte; }
extern inline pte_t pte_exprotect(pte_t pte)	{ pte_val(pte) &= ~_PAGE_EXECUTE;   return pte; }
extern inline pte_t pte_mkclean(pte_t pte)	{ pte_val(pte) &= ~_PAGE_DIRTY;	    return pte; }
extern inline pte_t pte_mkold(pte_t pte)	{ pte_val(pte) &= ~(_PAGE_ACCESSED|_PAGE_MAPPED);  return pte; }
extern inline pte_t pte_mkwrite(pte_t pte)	{ pte_val(pte) |=  _PAGE_WRITE;	    return pte; }
extern inline pte_t pte_mkread(pte_t pte)	{ pte_val(pte) |=  _PAGE_READ;	    return pte; }
extern inline pte_t pte_mkexec(pte_t pte)	{ pte_val(pte) |=  _PAGE_EXECUTE;   return pte; }
extern inline pte_t pte_mkdirty(pte_t pte)	{ pte_val(pte) |=  _PAGE_DIRTY;	    return pte; }
extern inline pte_t pte_mkyoung(pte_t pte)	{ pte_val(pte) |=  _PAGE_ACCESSED;  return pte; }
extern inline pte_t pte_mkmapped(pte_t pte)	{ pte_val(pte) |=  _PAGE_MAPPED;    return pte; }
extern inline pte_t pte_mkunmapped(pte_t pte)	{ pte_val(pte) &= ~_PAGE_MAPPED;    return pte; }

#if 0
extern inline void pte_clear(pte_t *ptep)	{
	pte_val(*(ptep)) &= ~(_PAGE_FLAGS);
}
#else
#define pte_clear(mm,addr,xp) do {		\
	set_pte_at(mm, addr, xp, __pte(0));	\
} while (0)
#endif

/* to find an entry in a kernel page-table-directory */
#define pgd_offset_k(address) pgd_offset(&init_mm, (address))

/* to find an entry in a page-table-directory. */
#define pgd_index(address)	(((address) >> PGDIR_SHIFT) & (PTRS_PER_PGD - 1))
#define pgd_offset(mm, address)	((mm)->pgd+pgd_index(address))

#define pte_index(address)	(((address) >> PAGE_SHIFT) & (PTRS_PER_PTE - 1))

/* Find an entry in the third-level page table.. */
extern inline pte_t * pte_offset_kernel(pmd_t * dir, unsigned long address)
{
	return (pte_t *) pmd_page_kernel(*dir) + pte_index(address);
}

#define pte_offset_map(dir,addr)	pte_offset_kernel((dir),(addr))
#define pte_offset_map_nested(dir,addr)	pte_offset_kernel((dir),(addr))
#define pte_unmap(pte)			do { } while (0)
#define pte_unmap_nested(pte)		do { } while (0)

extern pgd_t swapper_pg_dir[1024];

extern void update_mmu_cache(struct vm_area_struct * vma,
			     unsigned long address, pte_t *ptep, pte_t pte);
extern void tlb_modify(struct mm_struct * mm,
			     unsigned long address, pte_t *pte);

/*
 * Note: The macros below rely on the fact that MAX_SWAPFILES_SHIFT <= number of
 *	 bits in the swap-type field of the swap pte.  It would be nice to
 *	 enforce that, but we can't easily include <linux/swap.h> here.
 *	 (Of course, better still would be to define MAX_SWAPFILES_SHIFT here...).
 *
 * Format of swap pte:
 *	bit   0   : valid bit (must be zero)
 *	bit   1   : _PAGE_FILE (must be zero)
 *	bits  2- 8: swap-type
 *	bits  9-(31/63): swap offset
 *
 * Format of file pte:
 *	bit   0   : valid bit (must be zero)
 *	bit   1   : _PAGE_FILE (must be one)
 *	bits  2-(31/63): file_offset/PAGE_SIZE
 */
extern inline pte_t mk_swap_pte(unsigned long type, unsigned long offset)
{ pte_t pte; pte_val(pte) = (type << 2) | (offset << 9); return pte; }

#define __swp_type(x)		(((x).val >> 2) & 0x7f)
#define __swp_offset(x)		((x).val >> 9)
#define __swp_entry(type, off)	((swp_entry_t) { pte_val(mk_swap_pte((type), (off))) })
#define __pte_to_swp_entry(pte)	((swp_entry_t) { pte_val(pte) })
#define __swp_entry_to_pte(x)	((pte_t) { (x).val })

#define pte_to_pgoff(pte)	(pte_val(pte) >> 2)
#define pgoff_to_pte(off)	((pte_t) { ((off) << 2) | _PAGE_FILE })

#define PTE_FILE_MAX_BITS	(_PTE_SIZE - 9)

/* XXX carl - fixme */
#define kern_addr_valid(addr)	(1)

/* We need to map the physical page into linux's idea of physical memory */
#define io_remap_page_range(vma, start, busaddr, size, prot) \
    remap_page_range(vma, start, busaddr, size, prot)

#define pte_ERROR(e) \
	printk("%s:%d: bad pte %016lx.\n", __FILE__, __LINE__, pte_val(e))
#define pmd_ERROR(e) \
	printk("%s:%d: bad pmd %016lx.\n", __FILE__, __LINE__, pmd_val(e))
#define pgd_ERROR(e) \
	printk("%s:%d: bad pgd %016lx.\n", __FILE__, __LINE__, pgd_val(e))

extern void paging_init(void);

#define __HAVE_ARCH_PTEP_ESTABLISH
#define __HAVE_ARCH_PTEP_SET_ACCESS_FLAGS
#define __HAVE_ARCH_PTEP_SET_WRPROTECT

#include <asm-generic/pgtable.h>
#include INC_SYSTEM2(pgtable.h)

/*
 * No page table caches to initialise
 */
#define pgtable_cache_init()	do { } while (0)

/* We have our own get_unmapped_area to cope with ADDR_LIMIT_32BIT.  */
#define HAVE_ARCH_UNMAPPED_AREA

/*
 * remap a physical page `pfn' of size `size' with page protection `prot'
 * into virtual address `from'
 */
#define io_remap_pfn_range(vma,from,pfn,size,prot) \
		remap_pfn_range(vma, from, pfn, size, prot)

typedef pte_t *pte_addr_t;

#endif /* _L4_PGTABLE_H */
