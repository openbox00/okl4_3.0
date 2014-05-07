/*
 * arch/l4/sys-arm/mach-ixp4xx/common.c
 *
 * Generic code shared across all IXP4XX platforms
 * on Wombat systems
 *
 * Maintainer: David Snowdon <David.Snowdon@nicta.com.au>
 *
 * Copyright 2002 (c) Intel Corporation
 * Copyright 2003-2004 (c) MontaVista, Software, Inc. 
 * Copyright 2006 (c) National ICT Australia, Inc. 
 * 
 * This file is licensed under  the terms of the GNU General Public 
 * License version 2. This program is licensed "as is" without any 
 * warranty of any kind, whether express or implied.
 */


#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/serial.h>
#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/serial_core.h>
#include <linux/bootmem.h>
#include <linux/interrupt.h>
#include <linux/bitops.h>
#include <linux/time.h>
#include <linux/timex.h>

#include <asm/hardware.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/pgtable.h>
#include <asm/page.h>
#include <asm/irq.h>

/*************************************************************************
 * GPIO acces functions
 *************************************************************************/

/*
 * Configure GPIO line for input, interrupt, or output operation
 *
 * TODO: Enable/disable the irq_desc based on interrupt or output mode.
 * TODO: Should these be named ixp4xx_gpio_?
 */
void gpio_line_config(u8 line, u32 style)
{
	u32 enable;
	volatile u32 *int_reg;
	u32 int_style;

	enable = *IXP4XX_GPIO_GPOER;

	if (style & IXP4XX_GPIO_OUT) {
		enable &= ~((1) << line);
	} else if (style & IXP4XX_GPIO_IN) {
		enable |= ((1) << line);

		switch (style & IXP4XX_GPIO_INTSTYLE_MASK)
		{
		case (IXP4XX_GPIO_ACTIVE_HIGH):
			int_style = IXP4XX_GPIO_STYLE_ACTIVE_HIGH;
			break;
		case (IXP4XX_GPIO_ACTIVE_LOW):
			int_style = IXP4XX_GPIO_STYLE_ACTIVE_LOW;
			break;
		case (IXP4XX_GPIO_RISING_EDGE):
			int_style = IXP4XX_GPIO_STYLE_RISING_EDGE;
			break;
		case (IXP4XX_GPIO_FALLING_EDGE):
			int_style = IXP4XX_GPIO_STYLE_FALLING_EDGE;
			break;
		case (IXP4XX_GPIO_TRANSITIONAL):
			int_style = IXP4XX_GPIO_STYLE_TRANSITIONAL;
			break;
		default:
			int_style = IXP4XX_GPIO_STYLE_ACTIVE_HIGH;
			break;
		}

		if (line >= 8) {	/* pins 8-15 */ 
			line -= 8;
			int_reg = IXP4XX_GPIO_GPIT2R;
		}
		else {			/* pins 0-7 */
			int_reg = IXP4XX_GPIO_GPIT1R;
		}

		/* Clear the style for the appropriate pin */
		*int_reg &= ~(IXP4XX_GPIO_STYLE_CLEAR << 
		    		(line * IXP4XX_GPIO_STYLE_SIZE));

		/* Set the new style */
		*int_reg |= (int_style << (line * IXP4XX_GPIO_STYLE_SIZE));
	}

	*IXP4XX_GPIO_GPOER = enable;
}

EXPORT_SYMBOL(gpio_line_config);

