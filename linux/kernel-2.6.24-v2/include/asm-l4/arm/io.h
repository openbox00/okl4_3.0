#ifndef __L4_ARM_IO_H
#define __L4_ARM_IO_H

#include <asm/hardware.h>

#define L4_ARCH_IOSPACE_NUM(x)		0
#define L4_ARCH_IOSPACE_PORT(x)		(x & 0xfff)

/*
 * String version of IO memory access ops:
 */
extern void _memcpy_fromio(void *, unsigned long, size_t);
extern void _memcpy_toio(unsigned long, const void *, size_t);
extern void _memset_io(unsigned long, int, size_t);

#define memset_io(c,v,l)	_memset_io(__mem_pci(c),(v),(l))
#define memcpy_fromio(a,c,l)	_memcpy_fromio((a),__mem_pci(c),(l))
#define memcpy_toio(c,a,l)	_memcpy_toio(__mem_pci(c),(a),(l))

#define __raw_writeb(v,a)	(__chk_io_ptr(a), *(volatile unsigned char __force  *)(a) = (v))
#define __raw_writew(v,a)	(__chk_io_ptr(a), *(volatile unsigned short __force *)(a) = (v))                        
#define __raw_writel(v,a)	(__chk_io_ptr(a), *(volatile unsigned int __force   *)(a) = (v))      

#define __raw_readb(a)		(__chk_io_ptr(a), *(volatile unsigned char __force  *)(a))
#define __raw_readw(a)		(__chk_io_ptr(a), *(volatile unsigned short __force *)(a))
#define __raw_readl(a)		(__chk_io_ptr(a), *(volatile unsigned int __force   *)(a))

/*
 * io{read,write}{8,16,32} macros
 */
#ifndef ioread8
#define ioread8(p)	({ unsigned int __v = __raw_readb(p); __v; })
#define ioread16(p)	({ unsigned int __v = le16_to_cpu((__force __le16)__raw_readw(p)); __v; })
#define ioread32(p)	({ unsigned int __v = le32_to_cpu((__force __le32)__raw_readl(p)); __v; })

#define iowrite8(v,p)	__raw_writeb(v, p)
#define iowrite16(v,p)	__raw_writew((__force __u16)cpu_to_le16(v), p)
#define iowrite32(v,p)	__raw_writel((__force __u32)cpu_to_le32(v), p)

#define ioread8_rep(p,d,c)	__raw_readsb(p,d,c)
#define ioread16_rep(p,d,c)	__raw_readsw(p,d,c)
#define ioread32_rep(p,d,c)	__raw_readsl(p,d,c)

#define iowrite8_rep(p,s,c)	__raw_writesb(p,s,c)
#define iowrite16_rep(p,s,c)	__raw_writesw(p,s,c)
#define iowrite32_rep(p,s,c)	__raw_writesl(p,s,c)

extern void __iomem *ioport_map(unsigned long port, unsigned int nr);
extern void ioport_unmap(void __iomem *addr);
#endif

/*
 * (For now) We don't actually have real ISA nor PCI buses, but there is so many
 * drivers out there that might just work if we fake them... -cvs
 */
#define __io(a)			((void __iomem *)(a))
#define __mem_pci(a)		(a)


#endif /* __L4_ARM_IO_H */
