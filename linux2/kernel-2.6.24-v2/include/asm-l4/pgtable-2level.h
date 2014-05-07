#ifndef _L4_PGTABLE_2LEVEL_H
#define _L4_PGTABLE_2LEVEL_H

#define PMD_SHIFT	(22)
#define PMD_SIZE	(1UL << PMD_SHIFT)
#define PMD_MASK	(~(PMD_SIZE-1))

/* PGDIR_SHIFT determines what a third-level page table entry can map */
#define PGDIR_SHIFT	(22)
#define PGDIR_SIZE	(1UL << PGDIR_SHIFT)
#define PGDIR_MASK	(~(PGDIR_SIZE-1))

/*
 * Entries per page directory level:  the structure is three-level, with
 * all levels having a one-page page table.
 * FIXME - Page table must map the whole of TASK_SIZE!!!!
 */
#define PTRS_PER_PTE	(1UL << (PAGE_SHIFT-SIZEOF_PTR_LOG2))
#define PTRS_PER_PMD	(1)
#define PTRS_PER_PGD	(1UL << (PAGE_SHIFT-SIZEOF_PTR_LOG2))

/*
 * The "pgd_xxx()" functions here are trivial for a folded two-level
 * setup: the pgd is never bad, and a pmd always exists (as it's folded
 * into the pgd entry)
 */
extern inline int pgd_none(pgd_t pgd)		{ return 0; }
extern inline int pgd_bad(pgd_t pgd)		{ return 0; }
extern inline int pgd_present(pgd_t pgd)	{ return 1; }
#define pgd_clear(__X)				do { } while (0)

/* Find an entry in the second-level page table.. */
extern inline pmd_t * pmd_offset(pgd_t * dir, unsigned long address)
{
	return (pmd_t *) dir;
}

#endif
