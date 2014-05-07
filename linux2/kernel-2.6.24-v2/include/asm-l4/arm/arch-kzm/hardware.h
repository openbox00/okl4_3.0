/*
 *  Copyright 2004-2006 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

/*!
 * @file hardware.h
 * @brief This file contains the hardware definitions of the board.
 *
 * @ingroup System
 */
#ifndef __ASM_ARCH_HARDWARE_H__
#define __ASM_ARCH_HARDWARE_H__

#include <asm/sizes.h>

/*!
 * defines PCIO_BASE (not used but needed for compilation)
 */
#define PCIO_BASE		0
#if 1
/*!
 * This macro is used to get certain bit field from a number
 */
#define MXC_GET_FIELD(val, len, sh)          ((val >> sh) & ((1 << len) - 1))

/*!
 * This macro is used to set certain bit field inside a number
 */
#define MXC_SET_FIELD(val, len, sh, nval)    ((val & ~(((1 << len) - 1) << sh)) | nval << sh)

/* This is used to turn on/off debugging */
#define MXC_TRACE
#ifdef MXC_TRACE
/*!
 * This is used for error checking in debugging mode.
 */
#define MXC_ERR_CHK(a) \
        do { \
                if ((a)) { \
                        printk("Error at line %d in function %s in file %s", \
                                __LINE__, __FUNCTION__, __FILE__); \
                        BUG(); \
                } \
        } \
        while (0)
#else
#define MXC_ERR_CHK(a)
#endif

/*
 * ---------------------------------------------------------------------------
 * Processor specific defines
 * ---------------------------------------------------------------------------
 */



#include <asm/arch/mx31.h>
#include <asm/arch/mxc.h>

#define MXC_MAX_GPIO_LINES      (GPIO_NUM_PIN * GPIO_PORT_NUM)

/*
 * ---------------------------------------------------------------------------
 * Board specific defines
 * ---------------------------------------------------------------------------
 */
#define MXC_EXP_IO_BASE         (MXC_GPIO_BASE + MXC_MAX_GPIO_LINES)






#ifdef CONFIG_KZM
#include <asm/arch/board-mx31kz.h>
#endif



#ifndef MXC_MAX_EXP_IO_LINES
#define MXC_MAX_EXP_IO_LINES 0
#endif

#define MXC_MAX_INTS            (MXC_GPIO_BASE + \
                                MXC_MAX_GPIO_LINES + \
                                MXC_MAX_EXP_IO_LINES)
#endif
#endif				/* __ASM_ARCH_HARDWARE_H__ */
