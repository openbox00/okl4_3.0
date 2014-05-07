/*
 *  From:
 *  linux/arch/l4/sys-arm/mach-pxa/gumstix.c
 *
 *  Support for the Gumstix under Wombat
 *
 *  Author:	Nicolas Pitre
 *  Created:	Jun 15, 2001
 *  Copyright:	MontaVista Software Inc.
 *
 *  Author:	David Snowdon
 *  Created:	Jun 13, 2006
 *  Copyright:	National ICT Australia
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

/*
 * This is file is not used, for gumstix ethernet device,
 * please look at drivers/net/gumstix-smc91x.c.
 * -cch
 */

#if 0

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/major.h>
#include <linux/fb.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <asm/arch/irqs.h>
#include <asm/arch/pxa-regs.h>

#include <asm/mach/flash.h>

#include <asm/sizes.h>
#include <asm/delay.h> 

#include <pxa/pxa_i2c.h>

#include <asm/hardware.h>
#include "generic.h"


#define GUMSTIX_ETH_PHYS    PXA_CS1_PHYS
#define GUMSTIX_ETH_IRQ     IRQ_GPIO(36)
#define GUMSTIX_ETH_RST     80

static struct resource smc91x_resources[] = {
		[0] = {
				.start  = (GUMSTIX_ETH_PHYS + 0x300),
				.end    = (GUMSTIX_ETH_PHYS + 0xfffff),
				.flags  = IORESOURCE_MEM,
		},
		[1] = {
				.start  = GUMSTIX_ETH_IRQ,
				.end    = GUMSTIX_ETH_IRQ,
				.flags  = IORESOURCE_IRQ,
		},
};

static struct platform_device smc91x_device = {
		.name			= "smc91x",
		.id				= 0,
		.num_resources	= ARRAY_SIZE(smc91x_resources),
		.resource		= smc91x_resources,
};

static struct platform_device *devices[] __initdata = {
	&smc91x_device
};


static int __init gumstix_init(void)
{
	printk("gumstix_init: Initialising board...\n"); 
	pxa_map_io();
	pxa_generic_init(); 

	/* Set up nPWE */
	pxa_gpio_mode(GPIO49_nPWE_MD);

	/* Set up the chip selects */ 
	pxa_gpio_mode(GPIO15_nCS_1_MD);

	(void) platform_add_devices(devices, ARRAY_SIZE(devices));
	
	pxa_gpio_func(36, GPIO_MODE, GPIO_IN);
	/* Set rising edge detect for GPIO 36 */ 
	GFER(36) &= ~GPIO_BIT(36); 
	GRER(36) |= (GPIO_BIT(36)); 

	/* We assume that the bootloader has set up the rest for us */ 

	//        set_irq_type(IBOX_ETH_IRQ, IRQT_RISING);
	

	return 0;
}

__initcall(gumstix_init);
#endif
