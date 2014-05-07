#ifndef __L4_DMA_MAPPING_H
#define __L4_DMA_MAPPING_H

#include <asm/macros.h>
#include "asm/io.h"

/* arch/l4/kernel/main.c */
extern uintptr_t dma_heap_base, dma_heap_phys_base, dma_region_start;

/*
 * Only i386 implemented so far.
 * Bring other architectures in as needed.
 */
#if defined(__i386__) || defined(__arm__)
#include INC_SYSTEM2(dma-mapping.h)
#else
#include <asm-generic/dma-mapping-broken.h>
#endif

#endif
