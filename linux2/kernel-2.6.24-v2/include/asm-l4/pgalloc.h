#ifndef _L4_PGALLOC_H
#define _L4_PGALLOC_H


#include <linux/mm.h>
#include <linux/mmzone.h>

/*
 * Allocate and free page tables. The xxx_kernel() versions are
 * used to allocate a kernel page table - this turns on ASN bits
 * if any.
 */

#if PTRS_PER_PMD == 1

#define pmd_alloc_one(mm,addr)		({ BUG(); ((pmd_t *)2); })
#define pmd_free(pmd)			do { } while (0)
#define pgd_populate(mm,pmd,pte)	BUG()

#else /* 3-level page table */

static inline pmd_t *
pmd_alloc_one(struct mm_struct *mm, unsigned long address)
{
	pmd_t *ret = (pmd_t *)__get_free_page(GFP_KERNEL|__GFP_REPEAT|__GFP_ZERO);
	return ret;
}

static inline void
pmd_free(pmd_t *pmd)
{
	free_page((unsigned long)pmd);
}

static inline void
pgd_populate(struct mm_struct *mm, pgd_t *pgd, pmd_t *pmd)
{
	pgd_set(pgd, pmd);
}

#endif

static inline void
pmd_populate(struct mm_struct *mm, pmd_t *pmd, struct page *pte)
{
	pmd_set(pmd, (pte_t *)(page_to_phys(pte) + PAGE_OFFSET));
}

static inline void
pmd_populate_kernel(struct mm_struct *mm, pmd_t *pmd, pte_t *pte)
{
	pmd_set(pmd, pte);
}

static inline pgd_t *
pgd_alloc (struct mm_struct *mm) {
	pgd_t *pgd = (pgd_t *)__get_free_page(GFP_KERNEL|__GFP_ZERO);

	return pgd;
}

static inline void
pgd_free(pgd_t *pgd)
{
	free_page((unsigned long)pgd);
}

static inline pte_t *
pte_alloc_one_kernel(struct mm_struct *mm, unsigned long address)
{
	pte_t *pte = (pte_t *)__get_free_page(GFP_KERNEL|__GFP_REPEAT|__GFP_ZERO);
	return pte;
}

static inline void
pte_free_kernel(pte_t *pte)
{
	free_page((unsigned long)pte);
}

static inline struct page *
pte_alloc_one(struct mm_struct *mm, unsigned long addr)
{
	struct page *pte;

	pte = alloc_pages(GFP_KERNEL|__GFP_REPEAT|__GFP_ZERO, 0);
	return pte;
	
}

static inline void
pte_free(struct page *page)
{
	__free_page(page);
}

#define check_pgt_cache()	do { } while (0)

#endif /* _L4_PGALLOC_H */
