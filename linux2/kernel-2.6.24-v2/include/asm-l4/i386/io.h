#ifndef __L4_I386_IO_H
#define __L4_I386_IO_H

#include <linux/string.h>

#include "asm/page.h"


#ifdef IO_SPACE_LIMIT
#undef IO_SPACE_LIMIT
#define IO_SPACE_LIMIT (65536)
#endif


// XXX FIXME
#define L4_ARCH_IOSPACE_NUM(x)		0

#define L4_ARCH_IOSPACE_PORT(x)		(x & 0xfff)

/*
 * However PCI ones are not necessarily 1:1 and therefore these interfaces
 * are forbidden in portable PCI drivers.
 *
 * Allow them on x86 for legacy drivers, though.
 */
#define virt_to_bus virt_to_phys
#define bus_to_virt phys_to_virt

/*
 * readX/writeX() are used to access memory mapped devices. On some
 * architectures the memory mapped IO stuff needs to be accessed
 * differently. On the x86 architecture, we just read/write the
 * memory location directly.
 */

static inline unsigned char readb(const volatile void __iomem *addr)
{
	return *(volatile unsigned char __force *) addr;
}
static inline unsigned short readw(const volatile void __iomem *addr)
{
	return *(volatile unsigned short __force *) addr;
}
static inline unsigned int readl(const volatile void __iomem *addr)
{
	return *(volatile unsigned int __force *) addr;
}
#define readb_relaxed(addr) readb(addr)
#define readw_relaxed(addr) readw(addr)
#define readl_relaxed(addr) readl(addr)
#define __raw_readb readb
#define __raw_readw readw
#define __raw_readl readl

static inline void writeb(unsigned char b, volatile void __iomem *addr)
{
	*(volatile unsigned char __force *) addr = b;
}
static inline void writew(unsigned short b, volatile void __iomem *addr)
{
	*(volatile unsigned short __force *) addr = b;
}
static inline void writel(unsigned int b, volatile void __iomem *addr)
{
	*(volatile unsigned int __force *) addr = b;
}
#define __raw_writeb writeb
#define __raw_writew writew
#define __raw_writel writel

static inline void memset_io(volatile void __iomem *addr, unsigned char val, int count)
{
	memset((void __force *) addr, val, count);
}
static inline void memcpy_fromio(void *dst, volatile void __iomem *src, int count)
{
	memcpy(dst, (void __force *) src, count);
}
static inline void memcpy_toio(volatile void __iomem *dst, const void *src, int count)
{
	memcpy((void __force *) dst, src, count);
}


#ifdef SLOW_IO_BY_JUMPING
#define __SLOW_DOWN_IO "jmp 1f; 1: jmp 1f; 1:"
#else
#define __SLOW_DOWN_IO "outb %%al,$0x80;"
#endif

static inline void slow_down_io(void) {
	__asm__ __volatile__(
		__SLOW_DOWN_IO
#ifdef REALLY_SLOW_IO
		__SLOW_DOWN_IO __SLOW_DOWN_IO __SLOW_DOWN_IO
#endif
		: : );
}




//wombat/include/asm-i386/io.h
#define __BUILDIO(bwl,bw,type) \
static inline void out##bwl(unsigned type value, int port) { \
	out##bwl##_local(value, port); \
} \
static inline unsigned type in##bwl(int port) { \
	return in##bwl##_local(port); \
}



#define BUILDIO(bwl,bw,type) \
static inline void out##bwl##_local(unsigned type value, int port) { \
	__asm__ __volatile__("out" #bwl " %" #bw "0, %w1" : : "a"(value), "Nd"(port)); \
} \
static inline unsigned type in##bwl##_local(int port) { \
	unsigned type value; \
	__asm__ __volatile__("in" #bwl " %w1, %" #bw "0" : "=a"(value) : "Nd"(port)); \
	return value; \
} \
static inline void out##bwl##_local_p(unsigned type value, int port) { \
	out##bwl##_local(value, port); \
	slow_down_io(); \
} \
static inline unsigned type in##bwl##_local_p(int port) { \
	unsigned type value = in##bwl##_local(port); \
	slow_down_io(); \
	return value; \
} \
__BUILDIO(bwl,bw,type) \
static inline void out##bwl##_p(unsigned type value, int port) { \
	out##bwl(value, port); \
	slow_down_io(); \
} \
static inline unsigned type in##bwl##_p(int port) { \
	unsigned type value = in##bwl(port); \
	slow_down_io(); \
	return value; \
} \
static inline void outs##bwl(int port, const void *addr, unsigned long count) { \
	__asm__ __volatile__("rep; outs" #bwl : "+S"(addr), "+c"(count) : "d"(port)); \
} \
static inline void ins##bwl(int port, void *addr, unsigned long count) { \
	__asm__ __volatile__("rep; ins" #bwl : "+D"(addr), "+c"(count) : "d"(port)); \
}

BUILDIO(b,b,char)
BUILDIO(w,w,short)
BUILDIO(l,,int)


/* Used for dma */
static inline void flush_write_buffers(void)
{
        __asm__ __volatile__ ("lock; addl $0,0(%%esp)": : :"memory");
}

#endif /* __L4_I386_IO_H */
