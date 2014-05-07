#ifndef __L4_TLB_H
#define __L4_TLB_H

/*
 * L4 needs special per-pte or per-vma handling..
 */

#define tlb_flush(tlb)			\
do {	if ((tlb)->fullmm)		\
		flush_tlb_mm((tlb)->mm);\
} while (0)

#define tlb_start_vma(tlb, vma) \
do {	if (!(tlb)->fullmm)	\
		flush_cache_range(vma, vma->vm_start, vma->vm_end); \
} while (0)

#define tlb_end_vma(tlb, vma)	\
do {	if (!(tlb)->fullmm)	\
		flush_tlb_range(vma, vma->vm_start, vma->vm_end); \
} while (0)

void remove_tlb_pte(pte_t *ptep, unsigned long address);

#define __tlb_remove_tlb_entry(tlb, ptep, address)	\
	do { if (!(tlb)->fullmm)			\
		remove_tlb_pte(ptep, address);		\
	} while (0)

#include <asm-generic/tlb.h>

#define __pte_free_tlb(tlb,pte)			tlb_remove_page((tlb),(pte))
#define __pmd_free_tlb(tlb,pmd)			pmd_free(pmd)

#endif /* __L4_TLB_H */
