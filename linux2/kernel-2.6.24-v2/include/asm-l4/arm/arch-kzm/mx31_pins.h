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
#ifndef _ASM_ARCH_MX31_PINS_H
#define _ASM_ARCH_MX31_PINS_H

#ifndef __ASSEMBLY__

/*!
 * @name IOMUX/PAD Bit field definitions
 */

/*! @{ */

/*!
 * In order to identify pins more effectively, each mux-controlled pin's
 * enumerated value is constructed in the following way:
 *
 * -------------------------------------------------------------------
 * 31-29 | 28 - 24 |23 - 21| 20  | 19 - 18 | 17 - 10| 9 - 8 | 7 - 0
 * -------------------------------------------------------------------
 * IO_P  |  IO_I   |     RSVD    |  PAD_F  |  PAD_I | MUX_F | MUX_I
 * -------------------------------------------------------------------
 *
 * Bit 0 to 7 contains MUX_I used to identify the register
 * offset (0-based. base is IOMUX_module_base + 0xC) defined in the Section
 * "sw_pad_ctl & sw_mux_ctl details" of the IC Spec. Bit 8 to 9 is MUX_F which
 * contains the offset value defined WITHIN the same register (each IOMUX
 * control register contains four 8-bit fields for four different pins). The
 * similar field definitions are used for the pad control register.
 * For example, the MX31_PIN_A0 is defined in the enumeration:
 *    ( 73 << MUX_I) | (0 << MUX_F)|( 98 << PAD_I) | (0 << PAD_F)
 * It means the mux control register is at register offset 73. So the absolute
 * address is: 0xC+73*4=0x130   0 << MUX_F means the control bits are at the
 * least significant bits within the register. The pad control register offset
 * is: 0x154+98*4=0x2DC and also occupy the least significant bits within the
 * register.
 */

/*!
 * Starting bit position within each entry of \b iomux_pins to represent the
 * gpio port number (0-based) for that pin. For non-gpio pins, the bits will
 * be all 1's for error checking in the functions. (gpio port 7 is invalid)
 */
#define MUX_IO_P	29

/*!
 * Starting bit position within each entry of \b iomux_pins to represent the
 * gpio offset bit (0-based) for that pin. For non-gpio pins, the bits will
 * be all 0's since they are don't cares. So for port 2 pin 21, bit 31-24
 * will be (1 << MUX_IO_P) | (21 << MUX_IO_I).
 */
#define MUX_IO_I	24

/*!
 * Starting bit position within each entry of \b iomux_pins to represent the
 * MUX control register index (0-based)
 */
#define MUX_I		0

/*!
 * Starting bit position within each entry of \b iomux_pins to represent the
 * field within IOMUX control register for control bits
 * (legal values are 0, 1, 2, 3)
 */
#define MUX_F		8

/*!
 * Starting bit position within each entry of \b iomux_pins to represent the
 * PAD control register index (0-based)
 */
#define PAD_I		10

/*!
 * Starting bit position within each entry of \b iomux_pins to represent the
 * field within PAD control register for control bits
 * (legal values are 0, 1, 2)
 */
#define PAD_F		18

#define _MXC_BUILD_PIN(gp,gi,mi,mf,pi,pf) \
	((gp) << MUX_IO_P) | ((gi) << MUX_IO_I) | ((mi) << MUX_I) | \
	((mf) << MUX_F) | ((pi) << PAD_I) | ((pf) << PAD_F)

#define _MXC_BUILD_GPIO_PIN(gp,gi,mi,mf,pi,pf) \
		_MXC_BUILD_PIN(gp,gi,mi,mf,pi,pf)
#define _MXC_BUILD_NON_GPIO_PIN(mi,mf,pi,pf) \
		_MXC_BUILD_PIN(7,0,mi,mf,pi,pf)

/*! @} End IOMUX/PAD Bit field definitions */

/*!
 * This enumeration is constructed based on the Section
 * "sw_pad_ctl & sw_mux_ctl details" of the MX31 IC Spec. Each enumerated
 * value is constructed based on the rules described above.
 */
typedef enum iomux_pins {
	MX31_PIN_CSPI3_MISO = _MXC_BUILD_NON_GPIO_PIN(0, 3, 1, 2),
	MX31_PIN_CSPI3_SCLK = _MXC_BUILD_NON_GPIO_PIN(0, 2, 1, 1),
	MX31_PIN_CSPI3_SPI_RDY = _MXC_BUILD_NON_GPIO_PIN(0, 1, 1, 0),
	MX31_PIN_TTM_PAD = _MXC_BUILD_NON_GPIO_PIN(0, 0, 0, 2),
	MX31_PIN_ATA_RESET_B = _MXC_BUILD_GPIO_PIN(2, 31, 1, 3, 3, 0),
	MX31_PIN_CE_CONTROL = _MXC_BUILD_NON_GPIO_PIN(1, 2, 2, 2),
	MX31_PIN_CLKSS = _MXC_BUILD_NON_GPIO_PIN(1, 1, 2, 1),
	MX31_PIN_CSPI3_MOSI = _MXC_BUILD_NON_GPIO_PIN(1, 0, 2, 0),
	MX31_PIN_ATA_CS1 = _MXC_BUILD_GPIO_PIN(2, 27, 2, 3, 4, 1),
	MX31_PIN_ATA_DIOR = _MXC_BUILD_GPIO_PIN(2, 28, 2, 2, 4, 0),
	MX31_PIN_ATA_DIOW = _MXC_BUILD_GPIO_PIN(2, 29, 2, 1, 3, 2),
	MX31_PIN_ATA_DMACK = _MXC_BUILD_GPIO_PIN(2, 30, 2, 0, 3, 1),
	MX31_PIN_SD1_DATA1 = _MXC_BUILD_GPIO_PIN(1, 29, 3, 3, 5, 2),
	MX31_PIN_SD1_DATA2 = _MXC_BUILD_GPIO_PIN(1, 30, 3, 2, 5, 1),
	MX31_PIN_SD1_DATA3 = _MXC_BUILD_GPIO_PIN(1, 31, 3, 1, 5, 0),
	MX31_PIN_ATA_CS0 = _MXC_BUILD_GPIO_PIN(2, 26, 3, 0, 4, 2),
	MX31_PIN_D3_SPL = _MXC_BUILD_NON_GPIO_PIN(4, 3, 7, 0),
	MX31_PIN_SD1_CMD = _MXC_BUILD_GPIO_PIN(1, 26, 4, 2, 6, 2),
	MX31_PIN_SD1_CLK = _MXC_BUILD_GPIO_PIN(1, 27, 4, 1, 6, 1),
	MX31_PIN_SD1_DATA0 = _MXC_BUILD_GPIO_PIN(1, 28, 4, 0, 6, 0),
	MX31_PIN_VSYNC3 = _MXC_BUILD_NON_GPIO_PIN(5, 3, 8, 1),
	MX31_PIN_CONTRAST = _MXC_BUILD_NON_GPIO_PIN(5, 2, 8, 0),
	MX31_PIN_D3_REV = _MXC_BUILD_NON_GPIO_PIN(5, 1, 7, 2),
	MX31_PIN_D3_CLS = _MXC_BUILD_NON_GPIO_PIN(5, 0, 7, 1),
	MX31_PIN_SER_RS = _MXC_BUILD_GPIO_PIN(2, 25, 6, 3, 9, 2),
	MX31_PIN_PAR_RS = _MXC_BUILD_NON_GPIO_PIN(6, 2, 9, 1),
	MX31_PIN_WRITE = _MXC_BUILD_NON_GPIO_PIN(6, 1, 9, 0),
	MX31_PIN_READ = _MXC_BUILD_NON_GPIO_PIN(6, 0, 8, 2),
	MX31_PIN_SD_D_IO = _MXC_BUILD_GPIO_PIN(2, 21, 7, 3, 11, 0),
	MX31_PIN_SD_D_CLK = _MXC_BUILD_GPIO_PIN(2, 22, 7, 2, 10, 2),
	MX31_PIN_LCS0 = _MXC_BUILD_GPIO_PIN(2, 23, 7, 1, 10, 1),
	MX31_PIN_LCS1 = _MXC_BUILD_GPIO_PIN(2, 24, 7, 0, 10, 0),
	MX31_PIN_HSYNC = _MXC_BUILD_NON_GPIO_PIN(8, 3, 12, 1),
	MX31_PIN_FPSHIFT = _MXC_BUILD_NON_GPIO_PIN(8, 2, 12, 0),
	MX31_PIN_DRDY0 = _MXC_BUILD_NON_GPIO_PIN(8, 1, 11, 2),
	MX31_PIN_SD_D_I = _MXC_BUILD_GPIO_PIN(2, 20, 8, 0, 11, 1),
	MX31_PIN_LD15 = _MXC_BUILD_NON_GPIO_PIN(9, 3, 13, 2),
	MX31_PIN_LD16 = _MXC_BUILD_NON_GPIO_PIN(9, 2, 13, 1),
	MX31_PIN_LD17 = _MXC_BUILD_NON_GPIO_PIN(9, 1, 13, 0),
	MX31_PIN_VSYNC0 = _MXC_BUILD_NON_GPIO_PIN(9, 0, 12, 2),
	MX31_PIN_LD11 = _MXC_BUILD_NON_GPIO_PIN(10, 3, 15, 0),
	MX31_PIN_LD12 = _MXC_BUILD_NON_GPIO_PIN(10, 2, 14, 2),
	MX31_PIN_LD13 = _MXC_BUILD_NON_GPIO_PIN(10, 1, 14, 1),
	MX31_PIN_LD14 = _MXC_BUILD_NON_GPIO_PIN(10, 0, 14, 0),
	MX31_PIN_LD7 = _MXC_BUILD_NON_GPIO_PIN(11, 3, 16, 1),
	MX31_PIN_LD8 = _MXC_BUILD_NON_GPIO_PIN(11, 2, 16, 0),
	MX31_PIN_LD9 = _MXC_BUILD_NON_GPIO_PIN(11, 1, 15, 2),
	MX31_PIN_LD10 = _MXC_BUILD_NON_GPIO_PIN(11, 0, 15, 1),
	MX31_PIN_LD3 = _MXC_BUILD_NON_GPIO_PIN(12, 3, 17, 2),
	MX31_PIN_LD4 = _MXC_BUILD_NON_GPIO_PIN(12, 2, 17, 1),
	MX31_PIN_LD5 = _MXC_BUILD_NON_GPIO_PIN(12, 1, 17, 0),
	MX31_PIN_LD6 = _MXC_BUILD_NON_GPIO_PIN(12, 0, 16, 2),
	MX31_PIN_USBH2_DATA1 = _MXC_BUILD_NON_GPIO_PIN(13, 3, 19, 0),
	MX31_PIN_LD0 = _MXC_BUILD_NON_GPIO_PIN(13, 2, 18, 2),
	MX31_PIN_LD1 = _MXC_BUILD_NON_GPIO_PIN(13, 1, 18, 1),
	MX31_PIN_LD2 = _MXC_BUILD_NON_GPIO_PIN(13, 0, 18, 0),
	MX31_PIN_USBH2_DIR = _MXC_BUILD_NON_GPIO_PIN(14, 3, 20, 1),
	MX31_PIN_USBH2_STP = _MXC_BUILD_NON_GPIO_PIN(14, 2, 20, 0),
	MX31_PIN_USBH2_NXT = _MXC_BUILD_NON_GPIO_PIN(14, 1, 19, 2),
	MX31_PIN_USBH2_DATA0 = _MXC_BUILD_NON_GPIO_PIN(14, 0, 19, 1),
	MX31_PIN_USBOTG_DATA5 = _MXC_BUILD_NON_GPIO_PIN(15, 3, 21, 2),
	MX31_PIN_USBOTG_DATA6 = _MXC_BUILD_NON_GPIO_PIN(15, 2, 21, 1),
	MX31_PIN_USBOTG_DATA7 = _MXC_BUILD_NON_GPIO_PIN(15, 1, 21, 0),
	MX31_PIN_USBH2_CLK = _MXC_BUILD_NON_GPIO_PIN(15, 0, 20, 2),
	MX31_PIN_USBOTG_DATA1 = _MXC_BUILD_NON_GPIO_PIN(16, 3, 23, 0),
	MX31_PIN_USBOTG_DATA2 = _MXC_BUILD_NON_GPIO_PIN(16, 2, 22, 2),
	MX31_PIN_USBOTG_DATA3 = _MXC_BUILD_NON_GPIO_PIN(16, 1, 22, 1),
	MX31_PIN_USBOTG_DATA4 = _MXC_BUILD_NON_GPIO_PIN(16, 0, 22, 0),
	MX31_PIN_USBOTG_DIR = _MXC_BUILD_NON_GPIO_PIN(17, 3, 24, 1),
	MX31_PIN_USBOTG_STP = _MXC_BUILD_NON_GPIO_PIN(17, 2, 24, 0),
	MX31_PIN_USBOTG_NXT = _MXC_BUILD_NON_GPIO_PIN(17, 1, 23, 2),
	MX31_PIN_USBOTG_DATA0 = _MXC_BUILD_NON_GPIO_PIN(17, 0, 23, 1),
	MX31_PIN_USB_PWR = _MXC_BUILD_GPIO_PIN(0, 29, 18, 3, 25, 2),
	MX31_PIN_USB_OC = _MXC_BUILD_GPIO_PIN(0, 30, 18, 2, 25, 1),
	MX31_PIN_USB_BYP = _MXC_BUILD_GPIO_PIN(0, 31, 18, 1, 25, 0),
	MX31_PIN_USBOTG_CLK = _MXC_BUILD_NON_GPIO_PIN(18, 0, 24, 2),
	MX31_PIN_TDO = _MXC_BUILD_NON_GPIO_PIN(19, 3, 27, 0),
	MX31_PIN_TRSTB = _MXC_BUILD_NON_GPIO_PIN(19, 2, 26, 2),
	MX31_PIN_DE_B = _MXC_BUILD_NON_GPIO_PIN(19, 1, 26, 1),
	MX31_PIN_SJC_MOD = _MXC_BUILD_NON_GPIO_PIN(19, 0, 26, 0),
	MX31_PIN_RTCK = _MXC_BUILD_NON_GPIO_PIN(20, 3, 28, 1),
	MX31_PIN_TCK = _MXC_BUILD_NON_GPIO_PIN(20, 2, 28, 0),
	MX31_PIN_TMS = _MXC_BUILD_NON_GPIO_PIN(20, 1, 27, 2),
	MX31_PIN_TDI = _MXC_BUILD_NON_GPIO_PIN(20, 0, 27, 1),
	MX31_PIN_KEY_COL4 = _MXC_BUILD_GPIO_PIN(1, 22, 21, 3, 29, 2),
	MX31_PIN_KEY_COL5 = _MXC_BUILD_GPIO_PIN(1, 23, 21, 2, 29, 1),
	MX31_PIN_KEY_COL6 = _MXC_BUILD_GPIO_PIN(1, 24, 21, 1, 29, 0),
	MX31_PIN_KEY_COL7 = _MXC_BUILD_GPIO_PIN(1, 25, 21, 0, 28, 2),
	MX31_PIN_KEY_COL0 = _MXC_BUILD_NON_GPIO_PIN(22, 3, 31, 0),
	MX31_PIN_KEY_COL1 = _MXC_BUILD_NON_GPIO_PIN(22, 2, 30, 2),
	MX31_PIN_KEY_COL2 = _MXC_BUILD_NON_GPIO_PIN(22, 1, 30, 1),
	MX31_PIN_KEY_COL3 = _MXC_BUILD_NON_GPIO_PIN(22, 0, 30, 0),
	MX31_PIN_KEY_ROW4 = _MXC_BUILD_GPIO_PIN(1, 18, 23, 3, 32, 1),
	MX31_PIN_KEY_ROW5 = _MXC_BUILD_GPIO_PIN(1, 19, 23, 2, 32, 0),
	MX31_PIN_KEY_ROW6 = _MXC_BUILD_GPIO_PIN(1, 20, 23, 1, 31, 2),
	MX31_PIN_KEY_ROW7 = _MXC_BUILD_GPIO_PIN(1, 21, 23, 0, 31, 1),
	MX31_PIN_KEY_ROW0 = _MXC_BUILD_NON_GPIO_PIN(24, 3, 33, 2),
	MX31_PIN_KEY_ROW1 = _MXC_BUILD_NON_GPIO_PIN(24, 2, 33, 1),
	MX31_PIN_KEY_ROW2 = _MXC_BUILD_NON_GPIO_PIN(24, 1, 33, 0),
	MX31_PIN_KEY_ROW3 = _MXC_BUILD_NON_GPIO_PIN(24, 0, 32, 2),
	MX31_PIN_TXD2 = _MXC_BUILD_GPIO_PIN(0, 28, 25, 3, 35, 0),
	MX31_PIN_RTS2 = _MXC_BUILD_NON_GPIO_PIN(25, 2, 34, 2),
	MX31_PIN_CTS2 = _MXC_BUILD_NON_GPIO_PIN(25, 1, 34, 1),
	MX31_PIN_BATT_LINE = _MXC_BUILD_GPIO_PIN(1, 17, 25, 0, 34, 0),
	MX31_PIN_RI_DTE1 = _MXC_BUILD_GPIO_PIN(1, 14, 26, 3, 36, 1),
	MX31_PIN_DCD_DTE1 = _MXC_BUILD_GPIO_PIN(1, 15, 26, 2, 36, 0),
	MX31_PIN_DTR_DCE2 = _MXC_BUILD_GPIO_PIN(1, 16, 26, 1, 35, 2),
	MX31_PIN_RXD2 = _MXC_BUILD_GPIO_PIN(0, 27, 26, 0, 35, 1),
	MX31_PIN_RI_DCE1 = _MXC_BUILD_GPIO_PIN(1, 10, 27, 3, 37, 2),
	MX31_PIN_DCD_DCE1 = _MXC_BUILD_GPIO_PIN(1, 11, 27, 2, 37, 1),
	MX31_PIN_DTR_DTE1 = _MXC_BUILD_GPIO_PIN(1, 12, 27, 1, 37, 0),
	MX31_PIN_DSR_DTE1 = _MXC_BUILD_GPIO_PIN(1, 13, 27, 0, 36, 2),
	MX31_PIN_RTS1 = _MXC_BUILD_GPIO_PIN(1, 6, 28, 3, 39, 0),
	MX31_PIN_CTS1 = _MXC_BUILD_GPIO_PIN(1, 7, 28, 2, 38, 2),
	MX31_PIN_DTR_DCE1 = _MXC_BUILD_GPIO_PIN(1, 8, 28, 1, 38, 1),
	MX31_PIN_DSR_DCE1 = _MXC_BUILD_GPIO_PIN(1, 9, 28, 0, 38, 0),
	MX31_PIN_CSPI2_SCLK = _MXC_BUILD_NON_GPIO_PIN(29, 3, 40, 1),
	MX31_PIN_CSPI2_SPI_RDY = _MXC_BUILD_NON_GPIO_PIN(29, 2, 40, 0),
	MX31_PIN_RXD1 = _MXC_BUILD_GPIO_PIN(1, 4, 29, 1, 39, 2),
	MX31_PIN_TXD1 = _MXC_BUILD_GPIO_PIN(1, 5, 29, 0, 39, 1),
	MX31_PIN_CSPI2_MISO = _MXC_BUILD_NON_GPIO_PIN(30, 3, 41, 2),
	MX31_PIN_CSPI2_SS0 = _MXC_BUILD_NON_GPIO_PIN(30, 2, 41, 1),
	MX31_PIN_CSPI2_SS1 = _MXC_BUILD_NON_GPIO_PIN(30, 1, 41, 0),
	MX31_PIN_CSPI2_SS2 = _MXC_BUILD_NON_GPIO_PIN(30, 0, 40, 2),
	MX31_PIN_CSPI1_SS2 = _MXC_BUILD_NON_GPIO_PIN(31, 3, 43, 0),
	MX31_PIN_CSPI1_SCLK = _MXC_BUILD_NON_GPIO_PIN(31, 2, 42, 2),
	MX31_PIN_CSPI1_SPI_RDY = _MXC_BUILD_NON_GPIO_PIN(31, 1, 42, 1),
	MX31_PIN_CSPI2_MOSI = _MXC_BUILD_NON_GPIO_PIN(31, 0, 42, 0),
	MX31_PIN_CSPI1_MOSI = _MXC_BUILD_NON_GPIO_PIN(32, 3, 44, 1),
	MX31_PIN_CSPI1_MISO = _MXC_BUILD_NON_GPIO_PIN(32, 2, 44, 0),
	MX31_PIN_CSPI1_SS0 = _MXC_BUILD_NON_GPIO_PIN(32, 1, 43, 2),
	MX31_PIN_CSPI1_SS1 = _MXC_BUILD_NON_GPIO_PIN(32, 0, 43, 1),
	MX31_PIN_STXD6 = _MXC_BUILD_GPIO_PIN(0, 23, 33, 3, 45, 2),
	MX31_PIN_SRXD6 = _MXC_BUILD_GPIO_PIN(0, 24, 33, 2, 45, 1),
	MX31_PIN_SCK6 = _MXC_BUILD_GPIO_PIN(0, 25, 33, 1, 45, 0),
	MX31_PIN_SFS6 = _MXC_BUILD_GPIO_PIN(0, 26, 33, 0, 44, 2),
	MX31_PIN_STXD5 = _MXC_BUILD_GPIO_PIN(0, 21, 34, 3, 47, 0),
	MX31_PIN_SRXD5 = _MXC_BUILD_GPIO_PIN(0, 22, 34, 2, 46, 2),
	MX31_PIN_SCK5 = _MXC_BUILD_NON_GPIO_PIN(34, 1, 46, 1),
	MX31_PIN_SFS5 = _MXC_BUILD_NON_GPIO_PIN(34, 0, 46, 0),
	MX31_PIN_STXD4 = _MXC_BUILD_GPIO_PIN(0, 19, 35, 3, 48, 1),
	MX31_PIN_SRXD4 = _MXC_BUILD_GPIO_PIN(0, 20, 35, 2, 48, 0),
	MX31_PIN_SCK4 = _MXC_BUILD_NON_GPIO_PIN(35, 1, 47, 2),
	MX31_PIN_SFS4 = _MXC_BUILD_NON_GPIO_PIN(35, 0, 47, 1),
	MX31_PIN_STXD3 = _MXC_BUILD_GPIO_PIN(0, 17, 36, 3, 49, 2),
	MX31_PIN_SRXD3 = _MXC_BUILD_GPIO_PIN(0, 18, 36, 2, 49, 1),
	MX31_PIN_SCK3 = _MXC_BUILD_NON_GPIO_PIN(36, 1, 49, 0),
	MX31_PIN_SFS3 = _MXC_BUILD_NON_GPIO_PIN(36, 0, 48, 2),
	MX31_PIN_CSI_HSYNC = _MXC_BUILD_GPIO_PIN(2, 18, 37, 3, 51, 0),
	MX31_PIN_CSI_PIXCLK = _MXC_BUILD_GPIO_PIN(2, 19, 37, 2, 50, 2),
	MX31_PIN_I2C_CLK = _MXC_BUILD_NON_GPIO_PIN(37, 1, 50, 1),
	MX31_PIN_I2C_DAT = _MXC_BUILD_NON_GPIO_PIN(37, 0, 50, 0),
	MX31_PIN_CSI_D14 = _MXC_BUILD_GPIO_PIN(2, 14, 38, 3, 52, 1),
	MX31_PIN_CSI_D15 = _MXC_BUILD_GPIO_PIN(2, 15, 38, 2, 52, 0),
	MX31_PIN_CSI_MCLK = _MXC_BUILD_GPIO_PIN(2, 16, 38, 1, 51, 2),
	MX31_PIN_CSI_VSYNC = _MXC_BUILD_GPIO_PIN(2, 17, 38, 0, 51, 1),
	MX31_PIN_CSI_D10 = _MXC_BUILD_GPIO_PIN(2, 10, 39, 3, 53, 2),
	MX31_PIN_CSI_D11 = _MXC_BUILD_GPIO_PIN(2, 11, 39, 2, 53, 1),
	MX31_PIN_CSI_D12 = _MXC_BUILD_GPIO_PIN(2, 12, 39, 1, 53, 0),
	MX31_PIN_CSI_D13 = _MXC_BUILD_GPIO_PIN(2, 13, 39, 0, 52, 2),
	MX31_PIN_CSI_D6 = _MXC_BUILD_GPIO_PIN(2, 6, 40, 3, 55, 0),
	MX31_PIN_CSI_D7 = _MXC_BUILD_GPIO_PIN(2, 7, 40, 2, 54, 2),
	MX31_PIN_CSI_D8 = _MXC_BUILD_GPIO_PIN(2, 8, 40, 1, 54, 1),
	MX31_PIN_CSI_D9 = _MXC_BUILD_GPIO_PIN(2, 9, 40, 0, 54, 0),
	MX31_PIN_M_REQUEST = _MXC_BUILD_NON_GPIO_PIN(41, 3, 56, 1),
	MX31_PIN_M_GRANT = _MXC_BUILD_NON_GPIO_PIN(41, 2, 56, 0),
	MX31_PIN_CSI_D4 = _MXC_BUILD_GPIO_PIN(2, 4, 41, 1, 55, 2),
	MX31_PIN_CSI_D5 = _MXC_BUILD_GPIO_PIN(2, 5, 41, 0, 55, 1),
	MX31_PIN_PC_RST = _MXC_BUILD_NON_GPIO_PIN(42, 3, 57, 2),
	MX31_PIN_IOIS16 = _MXC_BUILD_NON_GPIO_PIN(42, 2, 57, 1),
	MX31_PIN_PC_RW_B = _MXC_BUILD_NON_GPIO_PIN(42, 1, 57, 0),
	MX31_PIN_PC_POE = _MXC_BUILD_NON_GPIO_PIN(42, 0, 56, 2),
	MX31_PIN_PC_VS1 = _MXC_BUILD_NON_GPIO_PIN(43, 3, 59, 0),
	MX31_PIN_PC_VS2 = _MXC_BUILD_NON_GPIO_PIN(43, 2, 58, 2),
	MX31_PIN_PC_BVD1 = _MXC_BUILD_NON_GPIO_PIN(43, 1, 58, 1),
	MX31_PIN_PC_BVD2 = _MXC_BUILD_NON_GPIO_PIN(43, 0, 58, 0),
	MX31_PIN_PC_CD2_B = _MXC_BUILD_NON_GPIO_PIN(44, 3, 60, 1),
	MX31_PIN_PC_WAIT_B = _MXC_BUILD_NON_GPIO_PIN(44, 2, 60, 0),
	MX31_PIN_PC_READY = _MXC_BUILD_NON_GPIO_PIN(44, 1, 59, 2),
	MX31_PIN_PC_PWRON = _MXC_BUILD_NON_GPIO_PIN(44, 0, 59, 1),
	MX31_PIN_D2 = _MXC_BUILD_NON_GPIO_PIN(45, 3, 61, 2),
	MX31_PIN_D1 = _MXC_BUILD_NON_GPIO_PIN(45, 2, 61, 1),
	MX31_PIN_D0 = _MXC_BUILD_NON_GPIO_PIN(45, 1, 61, 0),
	MX31_PIN_PC_CD1_B = _MXC_BUILD_NON_GPIO_PIN(45, 0, 60, 2),
	MX31_PIN_D6 = _MXC_BUILD_NON_GPIO_PIN(46, 3, 63, 0),
	MX31_PIN_D5 = _MXC_BUILD_NON_GPIO_PIN(46, 2, 62, 2),
	MX31_PIN_D4 = _MXC_BUILD_NON_GPIO_PIN(46, 1, 62, 1),
	MX31_PIN_D3 = _MXC_BUILD_NON_GPIO_PIN(46, 0, 62, 0),
	MX31_PIN_D10 = _MXC_BUILD_NON_GPIO_PIN(47, 3, 64, 1),
	MX31_PIN_D9 = _MXC_BUILD_NON_GPIO_PIN(47, 2, 64, 0),
	MX31_PIN_D8 = _MXC_BUILD_NON_GPIO_PIN(47, 1, 63, 2),
	MX31_PIN_D7 = _MXC_BUILD_NON_GPIO_PIN(47, 0, 63, 1),
	MX31_PIN_D14 = _MXC_BUILD_NON_GPIO_PIN(48, 3, 65, 2),
	MX31_PIN_D13 = _MXC_BUILD_NON_GPIO_PIN(48, 2, 65, 1),
	MX31_PIN_D12 = _MXC_BUILD_NON_GPIO_PIN(48, 1, 65, 0),
	MX31_PIN_D11 = _MXC_BUILD_NON_GPIO_PIN(48, 0, 64, 2),
	MX31_PIN_NFWP_B = _MXC_BUILD_GPIO_PIN(0, 14, 49, 3, 67, 0),
	MX31_PIN_NFCE_B = _MXC_BUILD_GPIO_PIN(0, 15, 49, 2, 66, 2),
	MX31_PIN_NFRB = _MXC_BUILD_GPIO_PIN(0, 16, 49, 1, 66, 1),
	MX31_PIN_D15 = _MXC_BUILD_NON_GPIO_PIN(49, 0, 66, 0),
	MX31_PIN_NFWE_B = _MXC_BUILD_GPIO_PIN(0, 10, 50, 3, 68, 1),
	MX31_PIN_NFRE_B = _MXC_BUILD_GPIO_PIN(0, 11, 50, 2, 68, 0),
	MX31_PIN_NFALE = _MXC_BUILD_GPIO_PIN(0, 12, 50, 1, 67, 2),
	MX31_PIN_NFCLE = _MXC_BUILD_GPIO_PIN(0, 13, 50, 0, 67, 1),
	MX31_PIN_SDQS0 = _MXC_BUILD_NON_GPIO_PIN(51, 3, 69, 2),
	MX31_PIN_SDQS1 = _MXC_BUILD_NON_GPIO_PIN(51, 2, 69, 1),
	MX31_PIN_SDQS2 = _MXC_BUILD_NON_GPIO_PIN(51, 1, 69, 0),
	MX31_PIN_SDQS3 = _MXC_BUILD_NON_GPIO_PIN(51, 0, 68, 2),
	MX31_PIN_SDCKE0 = _MXC_BUILD_NON_GPIO_PIN(52, 3, 71, 0),
	MX31_PIN_SDCKE1 = _MXC_BUILD_NON_GPIO_PIN(52, 2, 70, 2),
	MX31_PIN_SDCLK = _MXC_BUILD_NON_GPIO_PIN(52, 1, 70, 1),
	MX31_PIN_SDCLK_B = _MXC_BUILD_NON_GPIO_PIN(52, 0, 70, 0),
	MX31_PIN_RW = _MXC_BUILD_NON_GPIO_PIN(53, 3, 72, 1),
	MX31_PIN_RAS = _MXC_BUILD_NON_GPIO_PIN(53, 2, 72, 0),
	MX31_PIN_CAS = _MXC_BUILD_NON_GPIO_PIN(53, 1, 71, 2),
	MX31_PIN_SDWE = _MXC_BUILD_NON_GPIO_PIN(53, 0, 71, 1),
	MX31_PIN_CS5 = _MXC_BUILD_NON_GPIO_PIN(54, 3, 73, 2),
	MX31_PIN_ECB = _MXC_BUILD_NON_GPIO_PIN(54, 2, 73, 1),
	MX31_PIN_LBA = _MXC_BUILD_NON_GPIO_PIN(54, 1, 73, 0),
	MX31_PIN_BCLK = _MXC_BUILD_NON_GPIO_PIN(54, 0, 72, 2),
	MX31_PIN_CS1 = _MXC_BUILD_NON_GPIO_PIN(55, 3, 75, 0),
	MX31_PIN_CS2 = _MXC_BUILD_NON_GPIO_PIN(55, 2, 74, 2),
	MX31_PIN_CS3 = _MXC_BUILD_NON_GPIO_PIN(55, 1, 74, 1),
	MX31_PIN_CS4 = _MXC_BUILD_NON_GPIO_PIN(55, 0, 74, 0),
	MX31_PIN_EB0 = _MXC_BUILD_NON_GPIO_PIN(56, 3, 76, 1),
	MX31_PIN_EB1 = _MXC_BUILD_NON_GPIO_PIN(56, 2, 76, 0),
	MX31_PIN_OE = _MXC_BUILD_NON_GPIO_PIN(56, 1, 75, 2),
	MX31_PIN_CS0 = _MXC_BUILD_NON_GPIO_PIN(56, 0, 75, 1),
	MX31_PIN_DQM0 = _MXC_BUILD_NON_GPIO_PIN(57, 3, 77, 2),
	MX31_PIN_DQM1 = _MXC_BUILD_NON_GPIO_PIN(57, 2, 77, 1),
	MX31_PIN_DQM2 = _MXC_BUILD_NON_GPIO_PIN(57, 1, 77, 0),
	MX31_PIN_DQM3 = _MXC_BUILD_NON_GPIO_PIN(57, 0, 76, 2),
	MX31_PIN_SD28 = _MXC_BUILD_NON_GPIO_PIN(58, 3, 79, 0),
	MX31_PIN_SD29 = _MXC_BUILD_NON_GPIO_PIN(58, 2, 78, 2),
	MX31_PIN_SD30 = _MXC_BUILD_NON_GPIO_PIN(58, 1, 78, 1),
	MX31_PIN_SD31 = _MXC_BUILD_NON_GPIO_PIN(58, 0, 78, 0),
	MX31_PIN_SD24 = _MXC_BUILD_NON_GPIO_PIN(59, 3, 80, 1),
	MX31_PIN_SD25 = _MXC_BUILD_NON_GPIO_PIN(59, 2, 80, 0),
	MX31_PIN_SD26 = _MXC_BUILD_NON_GPIO_PIN(59, 1, 79, 2),
	MX31_PIN_SD27 = _MXC_BUILD_NON_GPIO_PIN(59, 0, 79, 1),
	MX31_PIN_SD20 = _MXC_BUILD_NON_GPIO_PIN(60, 3, 81, 2),
	MX31_PIN_SD21 = _MXC_BUILD_NON_GPIO_PIN(60, 2, 81, 1),
	MX31_PIN_SD22 = _MXC_BUILD_NON_GPIO_PIN(60, 1, 81, 0),
	MX31_PIN_SD23 = _MXC_BUILD_NON_GPIO_PIN(60, 0, 80, 2),
	MX31_PIN_SD16 = _MXC_BUILD_NON_GPIO_PIN(61, 3, 83, 0),
	MX31_PIN_SD17 = _MXC_BUILD_NON_GPIO_PIN(61, 2, 82, 2),
	MX31_PIN_SD18 = _MXC_BUILD_NON_GPIO_PIN(61, 1, 82, 1),
	MX31_PIN_SD19 = _MXC_BUILD_NON_GPIO_PIN(61, 0, 82, 0),
	MX31_PIN_SD12 = _MXC_BUILD_NON_GPIO_PIN(62, 3, 84, 1),
	MX31_PIN_SD13 = _MXC_BUILD_NON_GPIO_PIN(62, 2, 84, 0),
	MX31_PIN_SD14 = _MXC_BUILD_NON_GPIO_PIN(62, 1, 83, 2),
	MX31_PIN_SD15 = _MXC_BUILD_NON_GPIO_PIN(62, 0, 83, 1),
	MX31_PIN_SD8 = _MXC_BUILD_NON_GPIO_PIN(63, 3, 85, 2),
	MX31_PIN_SD9 = _MXC_BUILD_NON_GPIO_PIN(63, 2, 85, 1),
	MX31_PIN_SD10 = _MXC_BUILD_NON_GPIO_PIN(63, 1, 85, 0),
	MX31_PIN_SD11 = _MXC_BUILD_NON_GPIO_PIN(63, 0, 84, 2),
	MX31_PIN_SD4 = _MXC_BUILD_NON_GPIO_PIN(64, 3, 87, 0),
	MX31_PIN_SD5 = _MXC_BUILD_NON_GPIO_PIN(64, 2, 86, 2),
	MX31_PIN_SD6 = _MXC_BUILD_NON_GPIO_PIN(64, 1, 86, 1),
	MX31_PIN_SD7 = _MXC_BUILD_NON_GPIO_PIN(64, 0, 86, 0),
	MX31_PIN_SD0 = _MXC_BUILD_NON_GPIO_PIN(65, 3, 88, 1),
	MX31_PIN_SD1 = _MXC_BUILD_NON_GPIO_PIN(65, 2, 88, 0),
	MX31_PIN_SD2 = _MXC_BUILD_NON_GPIO_PIN(65, 1, 87, 2),
	MX31_PIN_SD3 = _MXC_BUILD_NON_GPIO_PIN(65, 0, 87, 1),
	MX31_PIN_A24 = _MXC_BUILD_NON_GPIO_PIN(66, 3, 89, 2),
	MX31_PIN_A25 = _MXC_BUILD_NON_GPIO_PIN(66, 2, 89, 1),
	MX31_PIN_SDBA1 = _MXC_BUILD_NON_GPIO_PIN(66, 1, 89, 0),
	MX31_PIN_SDBA0 = _MXC_BUILD_NON_GPIO_PIN(66, 0, 88, 2),
	MX31_PIN_A20 = _MXC_BUILD_NON_GPIO_PIN(67, 3, 91, 0),
	MX31_PIN_A21 = _MXC_BUILD_NON_GPIO_PIN(67, 2, 90, 2),
	MX31_PIN_A22 = _MXC_BUILD_NON_GPIO_PIN(67, 1, 90, 1),
	MX31_PIN_A23 = _MXC_BUILD_NON_GPIO_PIN(67, 0, 90, 0),
	MX31_PIN_A16 = _MXC_BUILD_NON_GPIO_PIN(68, 3, 92, 1),
	MX31_PIN_A17 = _MXC_BUILD_NON_GPIO_PIN(68, 2, 92, 0),
	MX31_PIN_A18 = _MXC_BUILD_NON_GPIO_PIN(68, 1, 91, 2),
	MX31_PIN_A19 = _MXC_BUILD_NON_GPIO_PIN(68, 0, 91, 1),
	MX31_PIN_A12 = _MXC_BUILD_NON_GPIO_PIN(69, 3, 93, 2),
	MX31_PIN_A13 = _MXC_BUILD_NON_GPIO_PIN(69, 2, 93, 1),
	MX31_PIN_A14 = _MXC_BUILD_NON_GPIO_PIN(69, 1, 93, 0),
	MX31_PIN_A15 = _MXC_BUILD_NON_GPIO_PIN(69, 0, 92, 2),
	MX31_PIN_A9 = _MXC_BUILD_NON_GPIO_PIN(70, 3, 95, 0),
	MX31_PIN_A10 = _MXC_BUILD_NON_GPIO_PIN(70, 2, 94, 2),
	MX31_PIN_MA10 = _MXC_BUILD_NON_GPIO_PIN(70, 1, 94, 1),
	MX31_PIN_A11 = _MXC_BUILD_NON_GPIO_PIN(70, 0, 94, 0),
	MX31_PIN_A5 = _MXC_BUILD_NON_GPIO_PIN(71, 3, 96, 1),
	MX31_PIN_A6 = _MXC_BUILD_NON_GPIO_PIN(71, 2, 96, 0),
	MX31_PIN_A7 = _MXC_BUILD_NON_GPIO_PIN(71, 1, 95, 2),
	MX31_PIN_A8 = _MXC_BUILD_NON_GPIO_PIN(71, 0, 95, 1),
	MX31_PIN_A1 = _MXC_BUILD_NON_GPIO_PIN(72, 3, 97, 2),
	MX31_PIN_A2 = _MXC_BUILD_NON_GPIO_PIN(72, 2, 97, 1),
	MX31_PIN_A3 = _MXC_BUILD_NON_GPIO_PIN(72, 1, 97, 0),
	MX31_PIN_A4 = _MXC_BUILD_NON_GPIO_PIN(72, 0, 96, 2),
	MX31_PIN_DVFS1 = _MXC_BUILD_NON_GPIO_PIN(73, 3, 99, 0),
	MX31_PIN_VPG0 = _MXC_BUILD_NON_GPIO_PIN(73, 2, 98, 2),
	MX31_PIN_VPG1 = _MXC_BUILD_NON_GPIO_PIN(73, 1, 98, 1),
	MX31_PIN_A0 = _MXC_BUILD_NON_GPIO_PIN(73, 0, 98, 0),
	MX31_PIN_CKIL = _MXC_BUILD_NON_GPIO_PIN(74, 3, 100, 1),
	MX31_PIN_POWER_FAIL = _MXC_BUILD_NON_GPIO_PIN(74, 2, 100, 0),
	MX31_PIN_VSTBY = _MXC_BUILD_NON_GPIO_PIN(74, 1, 99, 2),
	MX31_PIN_DVFS0 = _MXC_BUILD_NON_GPIO_PIN(74, 0, 99, 1),
	MX31_PIN_BOOT_MODE1 = _MXC_BUILD_NON_GPIO_PIN(75, 3, 101, 2),
	MX31_PIN_BOOT_MODE2 = _MXC_BUILD_NON_GPIO_PIN(75, 2, 101, 1),
	MX31_PIN_BOOT_MODE3 = _MXC_BUILD_NON_GPIO_PIN(75, 1, 101, 0),
	MX31_PIN_BOOT_MODE4 = _MXC_BUILD_NON_GPIO_PIN(75, 0, 100, 2),
	MX31_PIN_RESET_IN_B = _MXC_BUILD_NON_GPIO_PIN(76, 3, 103, 0),
	MX31_PIN_POR_B = _MXC_BUILD_NON_GPIO_PIN(76, 2, 102, 2),
	MX31_PIN_CLKO = _MXC_BUILD_NON_GPIO_PIN(76, 1, 102, 1),
	MX31_PIN_BOOT_MODE0 = _MXC_BUILD_NON_GPIO_PIN(76, 0, 102, 0),
	MX31_PIN_STX0 = _MXC_BUILD_GPIO_PIN(1, 1, 77, 3, 104, 1),
	MX31_PIN_SRX0 = _MXC_BUILD_GPIO_PIN(1, 2, 77, 2, 104, 0),
	MX31_PIN_SIMPD0 = _MXC_BUILD_GPIO_PIN(1, 3, 77, 1, 103, 2),
	MX31_PIN_CKIH = _MXC_BUILD_NON_GPIO_PIN(77, 0, 103, 1),
	MX31_PIN_GPIO3_1 = _MXC_BUILD_GPIO_PIN(2, 1, 78, 3, 105, 2),
	MX31_PIN_SCLK0 = _MXC_BUILD_GPIO_PIN(2, 2, 78, 2, 105, 1),
	MX31_PIN_SRST0 = _MXC_BUILD_GPIO_PIN(2, 3, 78, 1, 105, 0),
	MX31_PIN_SVEN0 = _MXC_BUILD_GPIO_PIN(1, 0, 78, 0, 104, 2),
	MX31_PIN_GPIO1_4 = _MXC_BUILD_GPIO_PIN(0, 4, 79, 3, 107, 0),
	MX31_PIN_GPIO1_5 = _MXC_BUILD_GPIO_PIN(0, 5, 79, 2, 106, 2),
	MX31_PIN_GPIO1_6 = _MXC_BUILD_GPIO_PIN(0, 6, 79, 1, 106, 1),
	MX31_PIN_GPIO3_0 = _MXC_BUILD_GPIO_PIN(2, 0, 79, 0, 106, 0),
	MX31_PIN_GPIO1_0 = _MXC_BUILD_GPIO_PIN(0, 0, 80, 3, 108, 1),
	MX31_PIN_GPIO1_1 = _MXC_BUILD_GPIO_PIN(0, 1, 80, 2, 108, 0),
	MX31_PIN_GPIO1_2 = _MXC_BUILD_GPIO_PIN(0, 2, 80, 1, 107, 2),
	MX31_PIN_GPIO1_3 = _MXC_BUILD_GPIO_PIN(0, 3, 80, 0, 107, 1),
	MX31_PIN_CAPTURE = _MXC_BUILD_GPIO_PIN(0, 7, 81, 3, 109, 2),
	MX31_PIN_COMPARE = _MXC_BUILD_GPIO_PIN(0, 8, 81, 2, 109, 1),
	MX31_PIN_WATCHDOG_RST = _MXC_BUILD_NON_GPIO_PIN(81, 1, 109, 0),
	MX31_PIN_PWMO = _MXC_BUILD_GPIO_PIN(0, 9, 81, 0, 108, 2),
} iomux_pin_name_t;

#endif
#endif
