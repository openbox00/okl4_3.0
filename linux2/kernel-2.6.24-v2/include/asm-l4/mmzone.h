/* Bits taken from Parisc, m32r
 * Carl van Schaik, NICTA
 */

#ifndef _L4_MMZONE_H
#define _L4_MMZONE_H

#ifdef CONFIG_DISCONTIGMEM

#include <asm/macros.h>

/* Get our architecture page info */
#include INC_SYSTEM2(page.h)

#define MAX_PHYSMEM_RANGES  4 /* Fix the size for now */

struct node_map_data {
	struct pglist_data pg_data;
};

extern struct node_map_data node_data[];

#define NODE_DATA(nid)          (&node_data[nid].pg_data)

/*
 * Given a kernel address, find the home node of the underlying memory.
 */
//#define kvaddr_to_nid(kaddr)	pfn_to_nid(__pa(kaddr) >> PAGE_SHIFT)

#define node_mem_map(nid)	(NODE_DATA(nid)->node_mem_map)
#define node_start_pfn(nid)	(NODE_DATA(nid)->node_start_pfn)
#define node_end_pfn(nid)						\
({									\
	pg_data_t *__pgdat = NODE_DATA(nid);				\
	__pgdat->node_start_pfn + __pgdat->node_spanned_pages;		\
})
#define node_localnr(pfn, nid)		((pfn) - node_start_pfn(nid))

#define local_mapnr(kvaddr)						\
({									\
	unsigned long __pfn = __pa(kvaddr) >> PAGE_SHIFT;		\
	(__pfn - node_start_pfn(pfn_to_nid(__pfn)));			\
})

#define pmd_page(pmd)		(pfn_to_page(pmd_val(pmd) >> PTE_PFN_OFFSET))

/* Define such that zones have contiguous aligned pfn numbers with each
 * zone allowed to map to an unaligned area. NB. we need to sort the
 * zonelist before creating zones in arch memory setup to avoid overlap
 */

#define PFNNID_SHIFT		(CONFIG_L4_ZONE_SIZE - PAGE_SHIFT)
#define PFNNID_HASH_SIZE	(1ul << CONFIG_NODES_SHIFT)

/* We use a hash to lookup the node id
 * Hash uses pfn such that the maximum addressable size of memory is
 * 2^(CONFIG_L4_ZONE_SIZE + (wordsize/2)) which with defaults gives
 * 2^(54) for 64bit and 2^(38) for 32bit
 */
typedef struct {
#if (_L4_WORD == 64)
	u32 match;
	u32 nid;
#else
	u16 match;
	u16 nid;
#endif
} nid_hash_entry_t;

typedef struct {
	nid_hash_entry_t entry;
	void *next;
} nid_hash_list_entry_t;

#define PFN_MAX		(1ul << (PFNNID_SHIFT + 8/2*(sizeof(nid_hash_entry_t))))

extern nid_hash_list_entry_t pfn_hash[PFNNID_HASH_SIZE];

static inline int pfn_to_nid(unsigned long pfn)
{
	unsigned long hash, match;
	nid_hash_list_entry_t *r;

	BUG_ON(pfn >= PFN_MAX);

	match = pfn >> PFNNID_SHIFT;
	hash = match & (PFNNID_HASH_SIZE-1ul);
	r = &pfn_hash[hash];

	while (unlikely(r->entry.match != match)) {
		if (r->next == NULL)
			return -1;

		r = (nid_hash_list_entry_t *)r->next;
	}

	return (int)r->entry.nid;
}

static inline int pfn_valid(unsigned long pfn)
{
	int nid = pfn_to_nid(pfn);

	return ((nid >= 0) && (pfn < node_end_pfn(nid)));
}

#else /* !CONFIG_DISCONTIGMEM */
#define MAX_PHYSMEM_RANGES 	1 
#endif
#endif /* _L4_MMZONE_H */
