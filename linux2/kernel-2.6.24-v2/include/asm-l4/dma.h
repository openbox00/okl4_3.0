#ifndef __L4_DMA_H
#define __L4_DMA_H

#include <asm/macros.h>
#include "asm/io.h"
#include INC_SYSTEM2(dma.h)

//#define MAX_DMA_ADDRESS		(0)

#endif

#ifdef CONFIG_PCI
extern int isa_dma_bridge_buggy;
#else
#define isa_dma_bridge_buggy 	(0)
#endif


