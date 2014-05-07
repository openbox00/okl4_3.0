#ifndef _L4_CACHEFLUSH_H_
#define _L4_CACHEFLUSH_H_

#include <asm/macros.h>

#include INC_SYSTEM2(cacheflush.h)

/* Define these properly if we are scanning the i_mmap(_shared) list */
#define flush_dcache_mmap_lock(mapping)           do { } while (0)
#define flush_dcache_mmap_unlock(mapping)           do { } while (0)
#define flush_cache_dup_mm(mm)			do { } while (0)

#endif /* _L4_CACHEFLUSH_H_ */
