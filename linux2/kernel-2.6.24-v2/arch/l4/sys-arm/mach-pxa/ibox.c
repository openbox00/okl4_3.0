/*
 *  From:
 *  linux/arch/l4/sys-arm/mach-pxa/ibox.c
 *
 *  Support for the NICTA iBox under Wombat
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


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/major.h>
#include <linux/fb.h>
#include <linux/interrupt.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <asm/arch/irqs.h>
#include <asm/arch/pxa-regs.h>

#include <asm/mach/flash.h>

#include <asm/sizes.h>

#include <pxa_i2c.h>

#include "generic.h"


#define IBOX_ETH_PHYS    PXA_CS1_PHYS
#define IBOX_ETH_IRQ     IRQ_GPIO(21)

static struct resource smc91x_resources[] = {
        [0] = {
                .start  = (IBOX_ETH_PHYS + 0x300),
                .end    = (IBOX_ETH_PHYS + 0xfffff),
                .flags  = IORESOURCE_MEM,
        },
        [1] = {
                .start  = IBOX_ETH_IRQ,
                .end    = IBOX_ETH_IRQ,
                .flags  = IORESOURCE_IRQ,
        }
};

static struct platform_device smc91x_device = {
        .name           = "smc91x",
        .id             = 0,
        .num_resources  = ARRAY_SIZE(smc91x_resources),
        .resource       = smc91x_resources,
};

static struct mtd_partition ibox_partitions[] = {
        {
                .name           = "BootLoader",
                .offset         = 0,
                .size           = 0x00040000,
                .mask_flags     = MTD_WRITEABLE
        }, {
                .name           = "Kernel",
                .offset         = MTDPART_OFS_APPEND,
                .size           = 0x00100000,
        }, {
                .name           = "rootfs",
                .offset         = MTDPART_OFS_APPEND,
                .size           = MTDPART_SIZ_FULL
        }
};

/* Flash memory */

static struct resource flash_resources[] = {
        [0] = {
                .start  = PXA_CS0_PHYS,
                .end    = PXA_CS0_PHYS + SZ_64M - 1,
                .flags  = IORESOURCE_MEM,
        }
};

static struct flash_platform_data ibox_flash_data = {
        .map_name       = "cfi_probe",
        .parts          = ibox_partitions,
        .nr_parts       = ARRAY_SIZE(ibox_partitions),
        .width          = 2,
        .name           = "iBox Flash",
};

static struct platform_device ibox_flash_device = {
        .name           = "pxa2xx-flash",
        .id             = 0,
        .dev = {
                .platform_data = &ibox_flash_data,
        },
        .resource = flash_resources,
        .num_resources = 1,
};

static struct platform_device *devices[] __initdata = {
	&smc91x_device,
	&ibox_flash_device
};


static int __init ibox_init(void)
{
	pxa_map_io();
	pxa_generic_init(); 

	/* initialise I2C and give a slave address */ 
	i2c_init(0xFF); 
	
	(void) platform_add_devices(devices, ARRAY_SIZE(devices));
	
	pxa_gpio_func(21, GPIO_MODE, GPIO_IN);
	/* Set rising edge detect for GPIO 21 */ 
	GFER(21) &= ~GPIO_BIT(21); 
	GRER(21) |= (GPIO_BIT(21)); 

	//        set_irq_type(IBOX_ETH_IRQ, IRQT_RISING);
	

	return 0;
}

__initcall(ibox_init);

