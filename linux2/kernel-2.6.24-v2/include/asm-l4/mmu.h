#ifndef __L4_MMU_H_
#define __L4_MMU_H_

#include "l4.h"

#if defined(CONFIG_IGUANA)
#include <iguana/eas.h>
#endif

#ifdef ARM_PID_RELOC
#include <asm/range_fl.h>
#endif

#include <asm/semaphore.h>
#include INC_SYSTEM2(mmu.h)

typedef struct {
#if defined(CONFIG_IGUANA)
	eas_ref_t eas;
#endif
	L4_SpaceId_t space_id;
#ifdef ARM_PID_RELOC	/* XXX should move to asm-l4/arm/mmu.h -gl */
	int pid;
	unsigned largevm:1;
	rfl_t mmap_free_list;
#endif
	L4_ARCH_MMU_CONTEXT
} mm_context_t;

void l4_map_page(mm_context_t *context, L4_Fpage_t fpage,
		unsigned long address, unsigned long attrib);

#endif /* __L4_MMU_H_ */
