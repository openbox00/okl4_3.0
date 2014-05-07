/*
 * Copyright 2004-2006 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#ifndef __ASM_ARM_ARCH_MXC_BOARD_H_
#define __ASM_ARM_ARCH_MXC_BOARD_H_

/*
 * Include Files
 */

#ifndef __ASSEMBLY__

struct mxc_ipu_config {
	int rev;
};

#endif

/*
 * The modes of the UART ports
 */
#define MODE_DTE                0
#define MODE_DCE                1
/*
 * Is the UART configured to be a IR port
 */
#define IRDA                    0
#define NO_IRDA                 1

#endif				/* __ASM_ARM_ARCH_MXC_BOARD_H_ */
