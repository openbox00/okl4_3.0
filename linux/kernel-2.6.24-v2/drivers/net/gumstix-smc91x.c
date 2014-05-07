/*
 *  Gumstix SMC91C111 chip intialization driver
 *
 *  Author:     Craig Hughes
 *  Created:    December 9, 2004
 *  Copyright:  (C) 2004 Craig Hughes
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/mii.h>

#include <asm/hardware.h>
#include <asm/arch/pxa-regs.h>
#include <asm/delay.h>

#include <asm/arch/gumstix.h>

#define SMC_DEBUG               0
#include <asm/io.h>
#include "smc91x.h"

static struct resource gumstix_smc91x0_resources[] = {
	[0] = {
		.name	= "smc91x-regs",
#if defined(CONFIG_CELL)
		.start  = 0x00000300,
		.end    = 0x000fffff,
#else
		.start  = PXA_CS1_PHYS + 0x00000300,
		.end    = PXA_CS1_PHYS + 0x000fffff,
#endif
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = GUMSTIX_ETH0_IRQ,
		.end    = GUMSTIX_ETH0_IRQ,
		.flags  = IORESOURCE_IRQ,
	},
	[2] = {
		.name	= "smc91x-dma",
#if defined(CONFIG_CELL)
		.start	= 0x00000300,
		.end	= 0x000fffff,
#else
		.start  = PXA_CS1_DMA + 0x00000300,
		.end    = PXA_CS1_DMA + 0x000fffff,
#endif
		.flags	= IORESOURCE_DMA,
	},
};

static struct resource gumstix_smc91x1_resources[] = {
	[0] = {
		.name	= "smc91x-regs",
#if defined(CONFIG_CELL)
		.start  = 0x00000300,
		.end    = 0x000fffff,
#else
		.start	= PXA_CS2_PHYS + 0x00000300,
		.end	= PXA_CS2_PHYS + 0x000fffff,
#endif
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= GUMSTIX_ETH1_IRQ,
		.end	= GUMSTIX_ETH1_IRQ,
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		.name	= "smc91x-dma",
#if defined(CONFIG_CELL)
		.start  = 0x00000300,
		.end    = 0x000fffff,
#else
		.start	= PXA_CS2_DMA + 0x00000300,
		.end	= PXA_CS2_DMA + 0x000fffff,
#endif
		.flags	= IORESOURCE_DMA,
	},
};

static struct platform_device gumstix_smc91x0_device = {
	.name           = "smc91x",
	.id             = 0,
	.num_resources  = ARRAY_SIZE(gumstix_smc91x0_resources),
	.resource       = gumstix_smc91x0_resources,
};

static struct platform_device gumstix_smc91x1_device = {
	.name           = "smc91x",
	.id             = 1,
	.num_resources  = ARRAY_SIZE(gumstix_smc91x1_resources),
	.resource       = gumstix_smc91x1_resources,
};

static struct platform_device *smc91x_devices[] = {
	&gumstix_smc91x0_device,
	&gumstix_smc91x1_device,
};

/* First we're going to test if there's a 2nd SMC91C111, and if not, then we'll free up those resources and the GPIO lines
 * that it would otherwise use.  We have no choice but to probe by doing:
 * Set nCS2 to CS2 mode
 * Set the reset line to GPIO out mode, and pull it high, then drop it low (to trigger reset)
 * Read from the memory space to check for the sentinel sequence identifying a likely SMC91C111 device
 */
int __init gumstix_smc91x_init(void)
{
	unsigned int val, num_devices=ARRAY_SIZE(smc91x_devices);
	void *ioaddr;
#if defined(CONFIG_CELL)
        gumstix_smc91x0_resources[0].start += PXA_CS1_PHYS;
printk("start: %lx\n", gumstix_smc91x0_resources[0].start);
        gumstix_smc91x0_resources[0].end += PXA_CS1_PHYS;
        gumstix_smc91x1_resources[0].start += PXA_CS2_PHYS;
        gumstix_smc91x1_resources[0].end += PXA_CS2_PHYS;
		
		gumstix_smc91x0_resources[2].start += PXA_CS1_DMA;
		gumstix_smc91x0_resources[2].end += PXA_CS1_DMA;
		gumstix_smc91x1_resources[2].start += PXA_CS2_DMA;
		gumstix_smc91x1_resources[2].end += PXA_CS2_DMA;
#endif

	/* Set up nPWE */
	pxa_gpio_mode(GPIO49_nPWE_MD);

	pxa_gpio_mode(GPIO78_nCS_2_MD);
	// If either if statement fails, then we'll drop out and turn_off_eth1,
	// if both succeed, then we'll skip that and just proceed with 2 cards
	if(request_mem_region(gumstix_smc91x1_resources[0].start, SMC_IO_EXTENT, "smc91x probe"))
	{
		ioaddr = ioremap(gumstix_smc91x1_resources[0].start, SMC_IO_EXTENT);
		val = ioread16(ioaddr + BANK_SELECT);
		iounmap(ioaddr);
		release_mem_region(gumstix_smc91x1_resources[0].start, SMC_IO_EXTENT);
		if ((val & 0xFF00) == 0x3300) {
			goto proceed;
		}
	}

//turn_off_eth1:
	// This is apparently not an SMC91C111
	// So, let's decrement the number of devices to request, and reset the GPIO lines to GPIO IN mode
	num_devices--;
	smc91x_devices[1] = NULL;
	pxa_gpio_mode(78 | GPIO_IN);
	
proceed:
	pxa_gpio_mode(GPIO15_nCS_1_MD);

	if(smc91x_devices[1]) pxa_gpio_mode(GPIO_GUMSTIX_ETH1_RST_MD);
	pxa_gpio_mode(GPIO_GUMSTIX_ETH0_RST_MD);
	if(smc91x_devices[1]) GPSR(GPIO_GUMSTIX_ETH1_RST) = GPIO_bit(GPIO_GUMSTIX_ETH1_RST);
	GPSR(GPIO_GUMSTIX_ETH0_RST) = GPIO_bit(GPIO_GUMSTIX_ETH0_RST);
	udelay(1); // Hold RESET for at least 100ns
	if(smc91x_devices[1]) GPCR(GPIO_GUMSTIX_ETH1_RST) = GPIO_bit(GPIO_GUMSTIX_ETH1_RST);
	GPCR(GPIO_GUMSTIX_ETH0_RST) = GPIO_bit(GPIO_GUMSTIX_ETH0_RST);
	msleep(50);

	return platform_add_devices(smc91x_devices, num_devices);
}

void __exit gumstix_smc91x_exit(void)
{
	if(smc91x_devices[1] != NULL) platform_device_unregister(&gumstix_smc91x1_device);
	platform_device_unregister(&gumstix_smc91x0_device);
}

void gumstix_smc91x_load(void) {}
EXPORT_SYMBOL(gumstix_smc91x_load);

module_init(gumstix_smc91x_init);
module_exit(gumstix_smc91x_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Craig Hughes <craig@gumstix.com>");
MODULE_DESCRIPTION("Gumstix board SMC91C111 chip initialization driver");
MODULE_VERSION("1:0.1");
