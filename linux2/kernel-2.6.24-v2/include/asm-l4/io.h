#ifndef __L4_IO_H
#define __L4_IO_H

#include <l4/types.h>

#include "asm/page.h"
#include <asm/macros.h>

#include INC_SYSTEM2(io.h)

// XXX not for ia32, which defined it above
#ifndef IO_SPACE_LIMIT
/* We don't really have an IO space, so don't restrict it */
#define IO_SPACE_LIMIT (-1ul)

#endif

/*
 * Convert a physical pointer to a virtual kernel pointer for /dev/mem
 * access
 */
#define xlate_dev_mem_ptr(p)    __va(p)
  
/* 
 * Convert a virtual cached pointer to an uncached pointer
 */
#define xlate_dev_kmem_ptr(p)   p

/*
 * Change virtual addresses to physical addresses and vv.
 * These are pretty trivial
 */
static inline unsigned long virt_to_phys(volatile void * address)
{
	return __pa((void *) address);
}

static inline void * phys_to_virt(unsigned long address)
{
	return __va(address);
}

extern void __iomem *
ioremap_nocache(unsigned long offset, unsigned long size);

extern void __iomem *
__ioremap(unsigned long offset, unsigned long size, unsigned long flags);

/**
 *	ioremap		-	map bus memory into CPU space
 *	@offset:	bus address of the memory
 *	@size:		size of the resource to map
 *
 *	ioremap performs a platform specific sequence of operations to
 *	make bus memory CPU accessible via the readb/readw/readl/writeb/
 *	writew/writel functions and the other mmio helpers. The returned
 *	address is not guaranteed to be usable directly as a virtual
 *	address.
 */

static inline void * ioremap(unsigned long offset, unsigned long size)
{
	return __ioremap(offset, size, 0);
}

extern void
iounmap (volatile void __iomem *addr);

typedef void (*_io_barrier)(void);

/* IO routines */
struct io_space {
	unsigned long mmio_base;	/* base in MMIO space */
	unsigned long (*io_encode)(unsigned long port);
	_io_barrier io_barrier;
};

extern void l4_init_io_space(unsigned int num);

extern struct io_space io_space[];
extern unsigned int num_io_spaces;


static inline void*
__l4_get_mmio_addr(unsigned long port, _io_barrier *barrier)
{
	struct io_space *space;
	unsigned long offset;

	port = port & IO_SPACE_LIMIT;
	space = &io_space[L4_ARCH_IOSPACE_NUM(port)];
	offset = L4_ARCH_IOSPACE_PORT(port);

	offset = space->io_encode(offset);
	*barrier = space->io_barrier;

	if (space->mmio_base == (-1ul)){
		//l4_init_io_space(L4_ARCH_IOSPACE_NUM(port));
		space->mmio_base =0;
		//hacky hacky FIXME
	}

	return (void *)(space->mmio_base + offset);
}

#if !defined(CONFIG_PC99)
static inline L4_Word8_t
_inb (unsigned long port)
{
	volatile L4_Word8_t *addr, res;
	_io_barrier barrier = NULL;

	addr = __l4_get_mmio_addr(port, &barrier);

	res = *addr;
	barrier();
	return res;
}

static inline L4_Word16_t
_inw (unsigned long port)
{
	volatile L4_Word16_t *addr, res;
	_io_barrier barrier = NULL;

	addr = __l4_get_mmio_addr(port, &barrier);

	res = *addr;
	barrier();
	return res;
}

static inline L4_Word32_t
_inl (unsigned long port)
{
	volatile L4_Word32_t *addr, res;
	_io_barrier barrier = NULL;

	addr = __l4_get_mmio_addr(port, &barrier);

	res = *addr;
	barrier();
	return res;
}

static inline void
_outb (L4_Word8_t val, unsigned long port)
{
	volatile L4_Word8_t *addr;
	_io_barrier barrier = NULL;

	addr = __l4_get_mmio_addr(port, &barrier);

	*addr = val;
	barrier();
}

static inline void
_outw (L4_Word16_t val, unsigned long port)
{
	volatile L4_Word16_t *addr;
	_io_barrier barrier = NULL;

	addr = __l4_get_mmio_addr(port, &barrier);

	*addr = val;
	barrier();
}

static inline void
_outl (L4_Word32_t val, unsigned long port)
{
	volatile L4_Word32_t *addr;
	_io_barrier barrier = NULL;

	addr = __l4_get_mmio_addr(port, &barrier);

	*addr = val;
	barrier();
}

static inline void
_outsb (unsigned long port, void *src, unsigned long count)
{
	volatile L4_Word8_t *addr, *s = src;
	_io_barrier barrier = NULL;

	addr = __l4_get_mmio_addr(port, &barrier);

	while (count--)
	{
		*addr = *s++;
		barrier();
	}
}

static inline void
_outsw (unsigned long port, void *src, unsigned long count)
{
	volatile L4_Word16_t *addr, *s = src;
	_io_barrier barrier = NULL;

	addr = __l4_get_mmio_addr(port, &barrier);

	while (count--)
	{
		*addr = *s++;
		barrier();
	}
}

static inline void
_outsl (unsigned long port, void *src, unsigned long count)
{
	volatile L4_Word32_t *addr, *s = src;
	_io_barrier barrier = NULL;

	addr = __l4_get_mmio_addr(port, &barrier);

	while (count--)
	{
		*addr = *s++;
		barrier();
	}
}

static inline void
_insb (unsigned long port, void *dst, unsigned long count)
{
	volatile L4_Word8_t *addr, *d = dst;
	_io_barrier barrier = NULL;

	addr = __l4_get_mmio_addr(port, &barrier);

	while (count--)
	{
		*d++ = *addr;
		barrier();
	}
}

static inline void
_insw (unsigned long port, void *dst, unsigned long count)
{
	volatile L4_Word16_t *addr, *d = dst;
	_io_barrier barrier = NULL;

	addr = __l4_get_mmio_addr(port, &barrier);

	while (count--)
	{
		*d++ = *addr;
		barrier();
	}
}

static inline void
_insl (unsigned long port, void *dst, unsigned long count)
{
	volatile L4_Word32_t *addr, *d = dst;
	_io_barrier barrier = NULL;

	addr = __l4_get_mmio_addr(port, &barrier);

	while (count--)
	{
		*d++ = *addr;
		barrier();
	}
}

#define inb	_inb
#define inw	_inw
#define inl	_inl
#define outb	_outb
#define outw	_outw
#define outl	_outl

#define insb	_insb
#define insw	_insw
#define insl	_insl
#define outsb	_outsb
#define outsw	_outsw
#define outsl	_outsl

#endif /* !defined (CONFIG_PC99) */

#define readb(a)	({				\
	unsigned int __read;				\
	/*printk("readb @ %p\n", (void*)a);*/		\
	__read = (*(volatile u8  *)(a));		\
	__read;	    })

#define readw(a)	({				\
	unsigned int __read;				\
	/* printk("readw @ %p\n", (void*)a);*/		\
	__read = (*(volatile u16 __force *)(a));	\
	__read;	    })

#define readl(a)	({				\
	unsigned int __read;				\
	/*printk("readl @ %p\n", (void*)a);*/		\
	__read = (*(volatile u32 __force *)(a));		\
	__read;	    })

#define writeb(v, a)	({				\
	/*printk("writeb @ %p\n", (void*)a);*/		\
	(*(volatile u8 *)(a)) = v;			\
	})

#define writew(v, a)	({				\
	/*printk("writew @ %p\n", (void*)a);*/		\
	(*(volatile u16 *)(a)) = v;			\
	})

#define writel(v, a)	({				\
	/*printk("writel @ %p\n", (void*)a);*/		\
	(*(volatile u32 *)(a)) = v;			\
	})

#ifdef CONFIG_ARCH_ARM

/*
 * Generic IO read/write.  These perform native-endian accesses.  Note
 * that some architectures will want to re-define __raw_{read,write}w.
 */
extern void __raw_writesb(unsigned int addr, const void *data, int bytelen);
extern void __raw_writesw(unsigned int addr, const void *data, int wordlen);
extern void __raw_writesl(unsigned int addr, const void *data, int longlen);

extern void __raw_readsb(unsigned int addr, void *data, int bytelen);
extern void __raw_readsw(unsigned int addr, void *data, int wordlen);
extern void __raw_readsl(unsigned int addr, void *data, int longlen);

#define readsb(p,d,l)		__raw_readsb((unsigned int)(p),d,l)
#define readsw(p,d,l)		__raw_readsw((unsigned int)(p),d,l)
#define readsl(p,d,l)		__raw_readsl((unsigned int)(p),d,l)

#define writesb(p,d,l)		__raw_writesb((unsigned int)(p),d,l)
#define writesw(p,d,l)		__raw_writesw((unsigned int)(p),d,l)
#define writesl(p,d,l)		__raw_writesl((unsigned int)(p),d,l)

#endif

#endif
