

#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/swap.h>
#include <linux/nodemask.h>
#include "wombat.h"

#if defined(CONFIG_IGUANA)
#include <iguana/memsection.h>
#endif

#define PFN_UP(x)       (((x) + PAGE_SIZE - 1) >> PAGE_SHIFT)
#define PFN_DOWN(x)       ((x) >> PAGE_SHIFT)
#define PFN_PHYS(x)      ((x) << PAGE_SHIFT)

#define STR(x)   #x
#define XSTR(x)  STR(x)

#ifdef CONFIG_DISCONTIGMEM
struct node_map_data node_data[MAX_NUMNODES];
nid_hash_list_entry_t pfn_hash[PFNNID_HASH_SIZE];
bootmem_data_t __initdata bmem_data[MAX_NUMNODES];
#endif

bootmem_area_t __initdata bootmem_area[MAX_PHYSMEM_RANGES];
int __initdata bootmem_areas;

#if defined(CONFIG_FLATMEM)
unsigned long pfn_offset;
#endif

#if defined(CONFIG_IGUANA)
// For use of mapcontrol calls
unsigned long vmalloc_ms_phys;
unsigned long vmalloc_ms_base;
unsigned long vmalloc_ms_size;
unsigned long vmalloc_ms_virt_to_phys_offset;
#endif

#define MAX_GAP (0x2000000UL >> PAGE_SHIFT)	    // 32MB for now

/* References to section boundaries */
extern char _text, _etext, _edata;
extern char __init_begin, __init_end;

/* Setup linux bootmem */
void __init
setup_bootmem(void)
{
#if defined(CONFIG_FLATMEM) && (MAX_PHYSMEM_RANGES > 1)
	bootmem_area_t mem_holes[MAX_PHYSMEM_RANGES - 1];
	int num_mem_holes;
#endif
	int i;
#if defined(CONFIG_IGUANA)
	memsection_ref_t ms;
	thread_ref_t dummy;
	size_t pagesize;
	uintptr_t paddr;
#endif

	max_low_pfn = 0ul;
	min_low_pfn = ~0ul;
	
	/* Sort the bootmem_areas
	 * This is a small list, use selection sort
	 */
	for (i = 0; i < (bootmem_areas-1); i++) {
		int r;
		for (r = i+1; r < bootmem_areas; r++) {
			bootmem_area_t tmp;
			if (bootmem_area[i].page_base >
			    bootmem_area[r].page_base)
			{
				tmp = bootmem_area[r];
				bootmem_area[r] = bootmem_area[i];
				bootmem_area[i] = tmp;
			}
		}
	}

#ifdef CONFIG_FLATMEM
	/* If there is gap that's too large, remove it from the available
	 * memory
	 */
	for (i = 1; i < bootmem_areas; i ++) {
		if ( (bootmem_area[i].page_base -
		      bootmem_area[i-1].page_base) > MAX_GAP ) {
			bootmem_areas = i;
			printk( KERN_WARNING "Large gap in memory ranges detected. Should probably use CONFIG_DISCONTIGMEM\n");
			break;
		}
	}
#endif

	printk("Kernel memory ranges:\n");
	{
		unsigned long pages = 0, last = 0;
		for (i = 0; i < bootmem_areas; i ++) {
			unsigned long start = bootmem_area[i].page_base;
			unsigned long end = start + bootmem_area[i].pages;
			if (last > start)
				panic("error, overlapping memory ranges");
			if (start == end)
				panic("error, zero size range");

			printk("  %d: 0x%08lx-0x%08lx (%ld pages)\n", i,
				start << PAGE_SHIFT, (end) << PAGE_SHIFT,
				bootmem_area[i].pages);

			if (min_low_pfn > start)
				min_low_pfn = start;
			if (max_low_pfn < end)
				max_low_pfn = end;

			pages += bootmem_area[i].pages;
			last = end;
		}
		printk("  total %ld pages\n", pages);
	}

#if defined(CONFIG_FLATMEM)
	pfn_offset = min_low_pfn + 1;
#endif
#if defined(CONFIG_FLATMEM) && (MAX_PHYSMEM_RANGES > 1)
	/* Merge ranges into bootmem_area[0] and record holes */
	{
		unsigned long end_pfn;
		unsigned long hole_pages;
		
		num_mem_holes = 0;
		end_pfn = bootmem_area[0].page_base + bootmem_area[0].pages;

		for (i = 1; i < bootmem_areas; i ++) {
			hole_pages = bootmem_area[i].page_base - end_pfn;
			if (hole_pages) {
				mem_holes[num_mem_holes].page_base = end_pfn;
				mem_holes[num_mem_holes].pages = hole_pages;
				end_pfn += hole_pages;
				num_mem_holes ++;
			}
			end_pfn += bootmem_area[i].pages;
		}

		bootmem_area[0].pages = end_pfn = bootmem_area[0].page_base;
		bootmem_areas = 1;

		BUG();
	}
#endif

#ifdef CONFIG_DISCONTIGMEM
	for (i = 0; i < MAX_NUMNODES; i++) {
		memset(NODE_DATA(i), 0, sizeof(pg_data_t));
		NODE_DATA(i)->bdata = &bmem_data[i];
	}
	for (i = 0; i < PFNNID_HASH_SIZE; i++)
	{
		pfn_hash[i].entry.match = (__typeof__(pfn_hash[i].entry.match))~0ul;
		pfn_hash[i].entry.nid = (__typeof__(pfn_hash[i].entry.nid))~0ul;
		pfn_hash[i].next = NULL;
	}
#endif

	nodes_clear(node_online_map);
	/* Merge ranges into nodes, stage 1 */
	{
#ifdef CONFIG_DISCONTIGMEM
		unsigned long start_node, end_node, last_node = ~0ul;
#endif
		int node, more_nodes, j;

		node = 0;
		for (i = 0; i < bootmem_areas; i ++) {
			unsigned long bootmap_size;
			bootmap_size = 0;
			more_nodes = 1;
#ifdef CONFIG_DISCONTIGMEM
			start_node = bootmem_area[i].page_base >> PFNNID_SHIFT;
			end_node = (bootmem_area[i].page_base + bootmem_area[i].pages - 1)
				    >> PFNNID_SHIFT;
			if (start_node == last_node)
				start_node ++;
			if (end_node >= start_node) {
				more_nodes = 1 + (end_node - start_node);
			}
			if ((node + more_nodes) >= MAX_NUMNODES)
				panic("memory ranges implied too many nodes, try increase CONFIG_L4_ZONE_SIZE");
#endif

			for (j = 0; j < more_nodes; j++) {
#ifdef CONFIG_DISCONTIGMEM
				unsigned long hash = (start_node + j) & (PFNNID_HASH_SIZE-1);
#endif
				unsigned long start, start_zone, end_zone;
				start = bootmem_area[i].page_base;
				end_zone = bootmem_area[i].page_base + bootmem_area[i].pages;

#ifdef CONFIG_DISCONTIGMEM
				nid_hash_list_entry_t *table = &pfn_hash[hash];

				if ( table->entry.nid != (__typeof__(table->entry.nid))(~0ul) ) {
				    while (table->next != NULL) {
					    table = table->next;
				    }
				    BUG();  // can't kmalloc here !!
				    table->next = kmalloc(sizeof(nid_hash_list_entry_t), GFP_KERNEL);
				    table = table->next;
				}

				table->entry.match = (start_node + j);
				table->entry.nid = node;
				table->next = NULL;

				start = start > (start_node + j) << PFNNID_SHIFT ?
					start : (start_node + j) << PFNNID_SHIFT;

				end_zone = (start_node + j + 1) << PFNNID_SHIFT;

				/* Keep zone alignment requirements */
				start_zone = (start & ~((1UL << (MAX_ORDER-1))-1));
				BUG_ON(start_zone < (start_node + j));
#else
				start_zone = start;
#endif

				bootmap_size = init_bootmem_node(NODE_DATA(node),
						start, start_zone, end_zone);

				node_set_online(node);
				node ++;
			}

			/* Free memory covered by this area, spanning nodes */
			{
				unsigned long start = bootmem_area[i].page_base;
				unsigned long end = start + bootmem_area[i].pages;
				unsigned long x;

				for (x = start; x < end;) {
					unsigned long next = end;
#ifdef CONFIG_DISCONTIGMEM
					int nid = pfn_to_nid(x);
					unsigned long node_end = ((x >> PFNNID_SHIFT)+1) << PFNNID_SHIFT;
					next = end > node_end ? node_end : end;
#endif

					free_bootmem_node(NODE_DATA(nid),
							PFN_PHYS(x), PFN_PHYS(next-x));
#ifdef CONFIG_DISCONTIGMEM
					/* Don't care that we may reserve this area multiple times */
					reserve_bootmem_node(NODE_DATA(nid), PFN_PHYS(x),
							PFN_PHYS(bootmem_bootmap_pages(1ul << PFNNID_SHIFT)));
#endif
					x = next;
				}
#ifdef CONFIG_FLATMEM
				reserve_bootmem_node(NODE_DATA(nid), PFN_PHYS(start),
						bootmap_size);

				NODE_DATA(0)->node_mem_map = PFN_PHYS(start) + PAGE_ALIGN(bootmap_size);
				{
					unsigned long size, _start, _end;

					/*
					 * The zone's endpoints aren't required to be MAX_ORDER
					 * aligned but the node_mem_map endpoints must be in order
					 * for the buddy allocator to function correctly.
					 */
					_start =  start & ~(MAX_ORDER_NR_PAGES - 1);
					_end = end;
					_end = ALIGN(_end, MAX_ORDER_NR_PAGES);
					size =  (_end - _start) * sizeof(struct page);
					reserve_bootmem_node(NODE_DATA(nid), NODE_DATA(0)->node_mem_map,
							size);
				}
#endif
			}

			//BUG_ON(node != numnodes);
		}
	}

#ifdef CONFIG_BLK_DEV_INITRD
	//reserve_bootmem_node(initrd_start, initrd_start - initrd_end);
#endif /* CONFIG_BLK_DEV_INITRD  */

	/* Calculate the virtual to physical offset for the memsection backing
	 * OK Linux for use by mapcontrol calls
	 */
#if defined(CONFIG_IGUANA)
	ms = memsection_lookup((memsection_ref_t) (bootmem_area[0].page_base << PAGE_SHIFT), &dummy);
	vmalloc_ms_base = (uintptr_t) memsection_base(ms);
	vmalloc_ms_size = (uintptr_t) memsection_size(ms);
	paddr = memsection_virt_to_phys((uintptr_t) vmalloc_ms_base, &pagesize);

	vmalloc_ms_virt_to_phys_offset = paddr - vmalloc_ms_base;
#endif

}


#define START_PFN(nid)  \
        (NODE_DATA(nid)->bdata->node_boot_start >> PAGE_SHIFT)
#define MAX_LOW_PFN(nid)        (NODE_DATA(nid)->bdata->node_low_pfn)

void __init
paging_init(void)
{
	unsigned long zones_size[MAX_NR_ZONES];
	int i, node;

	for (i = 0; i < MAX_NR_ZONES; i++)
		zones_size[i] = 0;

	setup_bootmem();

#ifdef CONFIG_DISCONTIGMEM
	{
		int nid;
		/*
		 * Insert nodes into pgdat_list backward so they appear in order.
		 */
		//pgdat_list = NULL;
		for (nid = num_online_nodes() - 1; nid >= 0; nid--) {
			int i;
			unsigned long pages = 0;
			for (i = 0 ; i < MAX_LOW_PFN(nid) - START_PFN(nid) ; i++)
				if (!test_bit(i, NODE_DATA(nid)->bdata->node_bootmem_map))
					pages++;
			/* Don't insert nearly empty zones */
			if (pages > 2) {
#if 0
				NODE_DATA(nid)->pgdat_next = pgdat_list;
				pgdat_list = NODE_DATA(nid);
#endif
			} else {
				/* Remove the current zone */
				for (i = nid; i < num_online_nodes() - 1; i++)
				{
					memcpy(&node_data[i], &node_data[i+1],
							sizeof(struct node_map_data));
				}
				//numnodes--;
				nodes_clear(node_online_map);
				for (i = 0; i < num_online_nodes() - 1; i++)
					node_set_online(i);
			}
		}

	}
#endif
	for_each_online_node(node) {
		unsigned long start = START_PFN(node);
		unsigned long end = MAX_LOW_PFN(node);

		zones_size[ZONE_NORMAL] = end - start;

		// XXX do we need holes support ?
		free_area_init_node(node, NODE_DATA(node),
				zones_size, start, NULL);
	}
}


/*
 * We have up to 8 empty zeroed pages so we can map one of the right colour
 * when needed.  This is necessary only on R4000 / R4400 SC and MC versions
 * where we have to avoid VCED / VECI exceptions for good performance at
 * any price.  Since page is never written to after the initialization we
 * don't have to care about aliases on other CPUs.
 */
unsigned long empty_zero_page, zero_page_mask;

/*
 * Not static inline because used by IP27 special magic initialization code
 */
unsigned long __init
setup_zero_pages(void)
{
	unsigned long order, size;
	struct page *page;

#if 0
	if (cpu_has_vce)
		order = 3;
	else
		order = 0;
#endif
	order = 0;

	empty_zero_page = __get_free_pages(GFP_KERNEL | __GFP_ZERO, order);
	if (!empty_zero_page)
		panic("Oh boy, that early out of memory?");

	page = virt_to_page(empty_zero_page);
	split_page(page, order);
	while (page < virt_to_page(empty_zero_page + (PAGE_SIZE << order))) {
		SetPageReserved(page);
		page++;
	}

	size = PAGE_SIZE << order;
	zero_page_mask = (size - 1) & PAGE_MASK;
	memset((void *)empty_zero_page, 0, size);

	return 1UL << order;
}

long __init reservedpages_count(void)
{
	int reservedpages, nid, i;

	reservedpages = 0;
	for_each_online_node(nid) {
		for (i = 0 ; i < MAX_LOW_PFN(nid) - START_PFN(nid) ; i++)
			if (PageReserved(NODE_DATA(nid)->node_mem_map + i))
				reservedpages++;
	}

	return reservedpages;
}

void __init
mem_init (void) 
{ 
	int nid;
	unsigned long codesize, reservedpages, datasize, initsize;

	num_physpages = 0;
	for_each_online_node(nid) {
		num_physpages += MAX_LOW_PFN(nid) - START_PFN(nid) + 1;
	}

#ifndef CONFIG_DISCONTIGMEM
	max_mapnr = num_physpages;
#endif  /* CONFIG_DISCONTIGMEM */
	high_memory = __va(max_low_pfn << PAGE_SHIFT);

	/* this will put all low memory onto the freelists */
	for_each_online_node(nid) {
		totalram_pages += free_all_bootmem_node(NODE_DATA(nid));
	}

	totalram_pages -= setup_zero_pages();	/* Setup zeroed pages.  */

	reservedpages = reservedpages_count();
	codesize = (unsigned long) &_etext - (unsigned long)&_text;
	datasize = (unsigned long) &_edata - (unsigned long)&_etext;
	initsize = (unsigned long) &__init_end - (unsigned long)&__init_begin;

	printk(KERN_INFO "Memory: %luk/%luk available (%ldk kernel code, "
		"%ldk reserved, %ldk data, %ldk init)\n",
		(unsigned long) nr_free_pages() << (PAGE_SHIFT-10),
		num_physpages << (PAGE_SHIFT-10),
		codesize >> 10,
		reservedpages << (PAGE_SHIFT-10),
		datasize >> 10,
		initsize >> 10);
} 

void __init
free_reserved_mem(void *start, void *end)
{
	void *__start = start;
	for (; __start < end; __start += PAGE_SIZE) {
		ClearPageReserved(virt_to_page(__start));
		free_page((long)__start);
		totalram_pages++;
	}
}

void __init
free_initmem(void)
{
#if 0//ndef CONFIG_ARM
	extern char __init_begin, __init_end;

	// FOR NOW - no arm
	free_reserved_mem(&__init_begin, &__init_end);
	printk ("Freeing unused kernel memory: %ldk freed\n",
		(unsigned long)(&__init_end - &__init_begin) >> 10);
#endif
}

void show_mem(void)
{
	printk("show_mem() called\n");
}
