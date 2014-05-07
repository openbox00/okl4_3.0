/*
 *  From:
 *  linux/arch/arm/mach-pxa/lubbock.c
 *
 *  Support for the Intel DBPXA250 Development Platform.
 *
 *  Author:	Nicolas Pitre
 *  Created:	Jun 15, 2001
 *  Copyright:	MontaVista Software Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/major.h>
#include <linux/fb.h>
#include <linux/interrupt.h>

#include <asm/arch/irqs.h>
#include <asm/arch/pxa-regs.h>

#include "generic.h"

#define PLEB2_ETH_PHYS PXA_CS1_PHYS
#define PLEB2_ETH_IRQ  IRQ_GPIO(2)

static struct resource smc91x_resources[] = {
	[0] = {
		.start	= (PLEB2_ETH_PHYS + 0x300),
		.end	= (PLEB2_ETH_PHYS + 0xfffff),
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= PLEB2_ETH_IRQ, // XXX IRQ number
		.end	= PLEB2_ETH_IRQ,
		.flags	= IORESOURCE_IRQ,
	}
};

static struct platform_device smc91x_device = {
	.name		= "smc91x",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(smc91x_resources),
	.resource	= smc91x_resources,
};

static struct platform_device *devices[] __initdata = {
	&smc91x_device,
};


/*
 * Handy function to set GPIO alternate functions
 */

static int __init pleb2_init(void)
{
	printk("%s: setup platform\n", __func__);

	pxa_map_io(); 
	pxa_generic_init(); 

	//pxa_set_udc_info(&udc_info);
	//set_pxa_fb_info(&sharp_lm8v31);
	(void) platform_add_devices(devices, ARRAY_SIZE(devices));

	/* This isn't the linux pxa_gpio_mode function, but could easily 
	   be replaced with it */ 
	pxa_gpio_func(2, GPIO_MODE, GPIO_IN);
	
	/* This would normally be set by the rising edge function 
	   which should be implemented in the IRQ code */ 
	GFER(2) |= GPIO_BIT(2);    /* Set falling edge detection */ 
	GRER(2) &= ~(GPIO_BIT(2)); /* Clear rising edge detection */ 

	return 0;
}


__initcall(pleb2_init);

