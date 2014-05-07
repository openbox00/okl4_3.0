/*****************************************
 * pci.c for l4 arch
 *
 * init_pci()
 * calls pci_scan_bus with correct params NULL first
 *
 *
 * struct pci_ops (?) bus->ops->read & bus->ops->write()
 *
 *
 * call init_pci() somewhere in wombat startup
 * arch_initcall(init_pci)
 ******************************************/


#include <linux/list.h>
#include <linux/pci.h>
#include <linux/mm.h>
#include <asm/pci-bridge.h>
#include <asm/setup.h>
#include <asm/types.h>
#include <asm/io.h>
#include "pci.h"

/*
 *	PCI BIOS function numbering for conventional PCI BIOS 
 *	systems
 */

#define PCIBIOS_PCI_FUNCTION_ID 	0xb1XX
#define PCIBIOS_PCI_BIOS_PRESENT 	0xb101
#define PCIBIOS_FIND_PCI_DEVICE		0xb102
#define PCIBIOS_FIND_PCI_CLASS_CODE	0xb103
#define PCIBIOS_GENERATE_SPECIAL_CYCLE	0xb106
#define PCIBIOS_READ_CONFIG_BYTE	0xb108
#define PCIBIOS_READ_CONFIG_WORD	0xb109
#define PCIBIOS_READ_CONFIG_DWORD	0xb10a
#define PCIBIOS_WRITE_CONFIG_BYTE	0xb10b
#define PCIBIOS_WRITE_CONFIG_WORD	0xb10c
#define PCIBIOS_WRITE_CONFIG_DWORD	0xb10d
#define PCIBIOS_GET_ROUTING_OPTIONS	0xb10e
#define PCIBIOS_SET_PCI_HW_INT		0xb10f


#define GDT_ENTRY_KERNEL_BASE	12
#define GDT_ENTRY_KERNEL_CS		(GDT_ENTRY_KERNEL_BASE + 0)
#define __KERNEL_CS (GDT_ENTRY_KERNEL_CS * 8)

//from arch/i386/pci/common.c
unsigned int pci_probe = PCI_PROBE_BIOS | PCI_PROBE_CONF1 | PCI_PROBE_CONF2 |
				PCI_PROBE_MMCONF;

void __devinit pcibios_sort(void);

int pcibios_last_bus = -1;

//from arch/i386/pci/irq.c
unsigned int pcibios_irq_mask = 0xfff8;
struct pci_bus *pci_root_bus = NULL;
struct pci_raw_ops *raw_pci_ops;

extern u8 pci_cache_line_size; 

// arch/i386/kernel/setup.c
unsigned long pci_mem_start =0x10000000;

// arch/i386/kernel/pci/direct.c


#define PCI_CONF1_ADDRESS(bus, devfn, reg) \
	(0x80000000 | (bus << 16) | (devfn << 8) | (reg & ~3))

static int pci_conf1_read (int seg, int bus, int devfn, int reg, int len, u32 *value)
{
	unsigned long flags;

	if (!value || (bus > 255) || (devfn > 255) || (reg > 255))
		return -EINVAL;

	spin_lock_irqsave(&pci_config_lock, flags);

	outl(PCI_CONF1_ADDRESS(bus, devfn, reg), 0xCF8);

	switch (len) {
	case 1:
		*value = inb(0xCFC + (reg & 3));
		break;
	case 2:
		*value = inw(0xCFC + (reg & 2));
		break;
	case 4:
		*value = inl(0xCFC);
		break;
	}

	spin_unlock_irqrestore(&pci_config_lock, flags);

	return 0;
}

static int pci_conf1_write (int seg, int bus, int devfn, int reg, int len, u32 value)
{
	unsigned long flags;

	if ((bus > 255) || (devfn > 255) || (reg > 255)) 
		return -EINVAL;

	spin_lock_irqsave(&pci_config_lock, flags);

	outl(PCI_CONF1_ADDRESS(bus, devfn, reg), 0xCF8);

	switch (len) {
	case 1:
		outb((u8)value, 0xCFC + (reg & 3));
		break;
	case 2:
		outw((u16)value, 0xCFC + (reg & 2));
		break;
	case 4:
		outl((u32)value, 0xCFC);
		break;
	}

	spin_unlock_irqrestore(&pci_config_lock, flags);

	return 0;
}

#undef PCI_CONF1_ADDRESS

static int __init pci_sanity_check(struct pci_raw_ops *o)
{
	u32 x = 0;
	int devfn;

	if (pci_probe & PCI_NO_CHECKS)
		return 1;

	for (devfn = 0; devfn < 0x100; devfn++) {
		if (o->read(0, 0, devfn, PCI_CLASS_DEVICE, 2, &x))
			continue;
		if (x == PCI_CLASS_BRIDGE_HOST || x == PCI_CLASS_DISPLAY_VGA)
			return 1;

		if (o->read(0, 0, devfn, PCI_VENDOR_ID, 2, &x))
			continue;
		if (x == PCI_VENDOR_ID_INTEL || x == PCI_VENDOR_ID_COMPAQ)
			return 1;
	}

	DBG("PCI: Sanity check failed\n");
	return 0;
}

struct pci_raw_ops pci_direct_conf1 = {
	.read =		pci_conf1_read,
	.write =	pci_conf1_write,
};

static int __init pci_check_type1(void)
{
	unsigned long flags;
	unsigned int tmp;
	int works = 0;

	local_irq_save(flags);

	outb(0x01, 0xCFB);
	tmp = inl(0xCF8);
	outl(0x80000000, 0xCF8);
	if (inl(0xCF8) == 0x80000000 && pci_sanity_check(&pci_direct_conf1)) {
		works = 1;
	}
	outl(tmp, 0xCF8);
	local_irq_restore(flags);

	return works;
}

/*
 * Functions for accessing PCI configuration space with type 2 accesses
 */

#define PCI_CONF2_ADDRESS(dev, reg)	(u16)(0xC000 | (dev << 8) | reg)

static int pci_conf2_read(int seg, int bus, int devfn, int reg, int len, u32 *value)
{
	unsigned long flags;
	int dev, fn;

	if (!value || (bus > 255) || (devfn > 255) || (reg > 255))
		return -EINVAL;

	dev = PCI_SLOT(devfn);
	fn = PCI_FUNC(devfn);

	if (dev & 0x10) 
		return PCIBIOS_DEVICE_NOT_FOUND;

	spin_lock_irqsave(&pci_config_lock, flags);

	outb((u8)(0xF0 | (fn << 1)), 0xCF8);
	outb((u8)bus, 0xCFA);

	switch (len) {
	case 1:
		*value = inb(PCI_CONF2_ADDRESS(dev, reg));
		break;
	case 2:
		*value = inw(PCI_CONF2_ADDRESS(dev, reg));
		break;
	case 4:
		*value = inl(PCI_CONF2_ADDRESS(dev, reg));
		break;
	}

	outb(0, 0xCF8);

	spin_unlock_irqrestore(&pci_config_lock, flags);

	return 0;
}

static int pci_conf2_write (int seg, int bus, int devfn, int reg, int len, u32 value)
{
	unsigned long flags;
	int dev, fn;

	if ((bus > 255) || (devfn > 255) || (reg > 255)) 
		return -EINVAL;

	dev = PCI_SLOT(devfn);
	fn = PCI_FUNC(devfn);

	if (dev & 0x10) 
		return PCIBIOS_DEVICE_NOT_FOUND;

	spin_lock_irqsave(&pci_config_lock, flags);

	outb((u8)(0xF0 | (fn << 1)), 0xCF8);
	outb((u8)bus, 0xCFA);

	switch (len) {
	case 1:
		outb((u8)value, PCI_CONF2_ADDRESS(dev, reg));
		break;
	case 2:
		outw((u16)value, PCI_CONF2_ADDRESS(dev, reg));
		break;
	case 4:
		outl((u32)value, PCI_CONF2_ADDRESS(dev, reg));
		break;
	}

	outb(0, 0xCF8);    

	spin_unlock_irqrestore(&pci_config_lock, flags);

	return 0;
}

static struct pci_raw_ops pci_direct_conf2 = {
	.read =		pci_conf2_read,
	.write =	pci_conf2_write,
};


static int __init pci_check_type2(void)
{
	unsigned long flags;
	int works = 0;

	local_irq_save(flags);

	outb(0x00, 0xCFB);
	outb(0x00, 0xCF8);
	outb(0x00, 0xCFA);
	if (inb(0xCF8) == 0x00 && inb(0xCFA) == 0x00 &&
	    pci_sanity_check(&pci_direct_conf2)) {
		works = 1;
	}

	local_irq_restore(flags);

	return works;
}

static int __init pci_direct_init(void)
{
	struct resource *region, *region2;

	printk(KERN_ALERT "PCI: direct init\n");
	if ((pci_probe & PCI_PROBE_CONF1) == 0)
		goto type2;
	printk(KERN_ALERT "%s: calling request_region(0xCF8, 8, \"PCI conf1\")\n", __FUNCTION__);
	region = request_region(0xCF8, 8, "PCI conf1");
	printk(KERN_ALERT "%s: request_region returned 0x%08lx\n", __FUNCTION__, (unsigned long int)region);
	if (!region)
		goto type2;

	printk(KERN_ALERT "calling pci_check_type1()\n");
	if (pci_check_type1()) {
		//printk(KERN_INFO "PCI: Using configuration type 1\n");
		printk(KERN_ALERT "PCI: Using configuration type 1\n");
		raw_pci_ops = &pci_direct_conf1;
		return 0;
	}
	printk(KERN_ALERT "*PCI: configuration type 1 FAILED*\n");
	release_resource(region);

 type2:
	printk(KERN_INFO "PCI: can we use type 2?\n");
	if ((pci_probe & PCI_PROBE_CONF2) == 0)
		goto out;
	printk(KERN_INFO "PCI: Using configuration type 2\n");
	region = request_region(0xCF8, 4, "PCI conf2");
	if (!region)
		goto out;
	region2 = request_region(0xC000, 0x1000, "PCI conf2");
	if (!region2)
		goto fail2;

	if (pci_check_type2()) {
		printk(KERN_INFO "PCI: Using configuration type 2\n");
		raw_pci_ops = &pci_direct_conf2;
		return 0;
	}

	release_resource(region2);
 fail2:
	release_resource(region);
	printk(KERN_ALERT "PCI: direct_init failed\n");

 out:
	printk(KERN_INFO "PCI: all messed up :-(\n");
	return 0;
}

arch_initcall(pci_direct_init);

int pcibios_enable_device(struct pci_dev *dev, int mask)
{
	u16 cmd, oldcmd;
	int i;



	pci_read_config_word(dev, PCI_COMMAND, &cmd);
	oldcmd = cmd;

	for (i = 0; i < PCI_NUM_RESOURCES; i++) {
		struct resource *res = &dev->resource[i];

		/* Only set up the requested stuff */
		if (!(mask & (1<<i)))
			continue;
		if (res->flags & IORESOURCE_IO)
			cmd |= PCI_COMMAND_IO;
		if (res->flags & IORESOURCE_MEM)
			cmd |= PCI_COMMAND_MEMORY;
	}

	if (cmd != oldcmd) {
		printk(KERN_ALERT "PCI: Enabling device: (%s), cmd %x\n",
		       pci_name(dev), cmd);
                /* Enable the appropriate bits in the PCI command register.  */
		pci_write_config_word(dev, PCI_COMMAND, cmd);
	}
	return 0;
}

unsigned int pcibios_max_latency = 255;

// from pci/i386.c

void pcibios_set_master(struct pci_dev *dev)
{
	u8 lat;
	printk(KERN_ALERT "PCI: %s :\n", __FUNCTION__);
	pci_read_config_byte(dev, PCI_LATENCY_TIMER, &lat);
	if (lat < 16)
		lat = (64 <= pcibios_max_latency) ? 64 : pcibios_max_latency;
	else if (lat > pcibios_max_latency)
		lat = pcibios_max_latency;
	else
		return;
	printk(KERN_DEBUG "PCI: Setting latency timer of device %s to %d\n", pci_name(dev), lat);
	pci_write_config_byte(dev, PCI_LATENCY_TIMER, lat);
}



// from pci/i386.c
int pci_mmap_page_range(struct pci_dev *dev, struct vm_area_struct *vma,
			enum pci_mmap_state mmap_state, int write_combine)
{
	unsigned long prot;

	printk(KERN_ALERT "PCI: %s :\n", __FUNCTION__);

	/* I/O space cannot be accessed via normal processor loads and
	 * stores on this platform.
	 */
	if (mmap_state == pci_mmap_io)
		return -EINVAL;

	/* Leave vm_pgoff as-is, the PCI space address is the physical
	 * address on this platform.
	 */
	prot = pgprot_val(vma->vm_page_prot);
	
//FIXME understand how this relateds to L4
//	if (boot_cpu_data.x86 > 3)
//		prot |= _PAGE_PCD | _PAGE_PWT;
	vma->vm_page_prot = __pgprot(prot);

	/* Write-combine setting is ignored, it is changed via the mtrr
	 * interfaces on this platform.
	 */
	/*
	 * Remap_page_range() function is not yet implemented.  :-(
	 */
#if 0
	if (remap_page_range(vma, vma->vm_start, vma->vm_pgoff << PAGE_SHIFT,
			     vma->vm_end - vma->vm_start,
			     vma->vm_page_prot))
		return -EAGAIN;
#endif

	return 0;
}





// arch/i386/pci/i386.c
void
pcibios_align_resource(void *data, struct resource *res,
		       resource_size_t size, resource_size_t align)
{
	printk(KERN_ALERT "PCI: %s :\n", __FUNCTION__);
	if (res->flags & IORESOURCE_IO) {
		resource_size_t start = res->start;

		if (start & 0x300) {
			start = (start + 0x3ff) & ~0x3ff;
			res->start = start;
		}
	}
}

unsigned int pcibios_assign_all_busses(void)
{
	printk(KERN_ALERT "PCI: %s :\n", __FUNCTION__);
	return (pci_probe & PCI_ASSIGN_ALL_BUSSES) ? 1 : 0;
}

void __devinit  pcibios_fixup_bus(struct pci_bus *b)
{
//	pcibios_fixup_ghosts(b);
	printk(KERN_ALERT "PCI: %s :\n", __FUNCTION__);

	pci_read_bridge_bases(b);
}

//arch/i386/pci/common.c
char * __devinit  pcibios_setup(char *str)
{
	printk(KERN_ALERT "PCI: %s :\n", __FUNCTION__);
	if (!strcmp(str, "off")) {
		pci_probe = 0;
		return NULL;
	}
#ifdef CONFIG_PCI_BIOS
	else if (!strcmp(str, "bios")) {
		pci_probe = PCI_PROBE_BIOS;
		return NULL;
	} else if (!strcmp(str, "nobios")) {
		pci_probe &= ~PCI_PROBE_BIOS;
		return NULL;
	} else if (!strcmp(str, "nosort")) {
		pci_probe |= PCI_NO_SORT;
		return NULL;
	} else if (!strcmp(str, "biosirq")) {
		pci_probe |= PCI_BIOS_IRQ_SCAN;
		return NULL;
	}
#endif
#ifdef CONFIG_PCI_DIRECT
	else if (!strcmp(str, "conf1")) {
		pci_probe = PCI_PROBE_CONF1 | PCI_NO_CHECKS;
		return NULL;
	}
	else if (!strcmp(str, "conf2")) {
		pci_probe = PCI_PROBE_CONF2 | PCI_NO_CHECKS;
		return NULL;
	}
#endif
#ifdef CONFIG_PCI_MMCONFIG
	else if (!strcmp(str, "nommconf")) {
		pci_probe &= ~PCI_PROBE_MMCONF;
		return NULL;
	}
#endif
	else if (!strcmp(str, "noacpi")) {
		//acpi_noirq_set();
		return NULL;
	}
#ifndef CONFIG_X86_VISWS
	else if (!strcmp(str, "usepirqmask")) {
		pci_probe |= PCI_USE_PIRQ_MASK;
		return NULL;
	} else if (!strncmp(str, "irqmask=", 8)) {
		pcibios_irq_mask = simple_strtol(str+8, NULL, 0);
		return NULL;
	} else if (!strncmp(str, "lastbus=", 8)) {
		pcibios_last_bus = simple_strtol(str+8, NULL, 0);
		return NULL;
	}
#endif
	else if (!strcmp(str, "rom")) {
		pci_probe |= PCI_ASSIGN_ROMS;
		return NULL;
	} else if (!strcmp(str, "assign-busses")) {
		pci_probe |= PCI_ASSIGN_ALL_BUSSES;
		return NULL;
	}
	return str;
}

