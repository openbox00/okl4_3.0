/*
 *  Copyright 2004-2007 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */
#ifndef __ASM_ARCH_MXC_IRQS_H__
#define __ASM_ARCH_MXC_IRQS_H__

#include <asm/arch/hardware.h>

/*!
 * @defgroup Interrupt Interrupt Controller (AVIC)
 */

/*!
 * @file arch-mxc/irqs.h
 * @brief This file defines the number of normal interrupts and fast interrupts
 *
 * @ingroup Interrupt
 */

#define MXC_IRQ_TO_EDIO(nr)     ((nr) - MXC_EXT_BASE)
#define MXC_IRQ_IS_EDIO(nr)  	((nr) >= MXC_EXT_BASE)

#define MXC_IRQ_TO_EXPIO(irq)	(irq - MXC_EXP_IO_BASE)

#define MXC_IRQ_TO_GPIO(irq)	(irq - MXC_GPIO_BASE)
#define MXC_GPIO_TO_IRQ(x)	(MXC_GPIO_BASE + x)

/*!
 * REVISIT: document me
 */
#define ARCH_TIMER_IRQ	INT_GPT

/*!
 * Number of normal interrupts
 */
#define NR_IRQS         (MXC_MAX_INTS)

/*!
 * Number of fast interrupts
 */
#define NR_FIQS		(MXC_MAX_INTS)

/*
 * This function is used to get the AVIC Lo and Hi interrupts
 * that are enabled as wake up sources to wake up the core from suspend
 */
void mxc_get_wake_irq(u32 * wake_src[]);

#endif				/* __ASM_ARCH_MXC_IRQS_H__ */
