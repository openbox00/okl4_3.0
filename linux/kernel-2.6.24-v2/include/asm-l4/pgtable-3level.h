#ifndef _L4_PGTABLE_3LEVEL_H
#define _L4_PGTABLE_3LEVEL_H


// FIXME - check how big this restricts our address space to
/* PMD_SHIFT determines the size of the area a second-level page table can map */
#define PMD_SHIFT	(PAGE_SHIFT + (PAGE_SHIFT-SIZEOF_PTR_LOG2))
#define PMD_SIZE	(1UL << PMD_SHIFT)
#define PMD_MASK	(~(PMD_SIZE-1))

/* PGDIR_SHIFT determines what a third-level page table entry can map */
#define PGDIR_SHIFT	(PAGE_SHIFT + 2*(PAGE_SHIFT-SIZEOF_PTR_LOG2))
#define PGDIR_SIZE	(1UL << PGDIR_SHIFT)
#define PGDIR_MASK	(~(PGDIR_SIZE-1))

/*
 * Entries per page directory level:  the structure is three-level, with
 * all levels having a one-page page table.
 * FIXME - Page table must map the whole of TASK_SIZE!!!!
 */
#define PTRS_PER_PTE	(1UL << (PAGE_SHIFT-SIZEOF_PTR_LOG2))
#define PTRS_PER_PMD	(1UL << (PAGE_SHIFT-SIZEOF_PTR_LOG2))
#define PTRS_PER_PGD	(1UL << (PAGE_SHIFT-SIZEOF_PTR_LOG2))


extern inline int pgd_none(pgd_t pgd)		{ return !pgd_val(pgd); }
extern inline int pgd_bad(pgd_t pgd)		{ return (pgd_val(pgd) & ~_PFN_MASK) != _PAGE_TABLE; }
extern inline int pgd_present(pgd_t pgd)	{ return pgd_val(pgd) & _PAGE_VALID; }
extern inline void pgd_clear(pgd_t * pgdp)	{ pgd_val(*pgdp) = 0; }

/* Find an entry in the second-level page table.. */
extern inline pmd_t * pmd_offset(pgd_t * dir, unsigned long address)
{
	return (pmd_t *) pgd_page(*dir) + ((address >> PMD_SHIFT) & (PTRS_PER_PMD - 1));
}

#endif
