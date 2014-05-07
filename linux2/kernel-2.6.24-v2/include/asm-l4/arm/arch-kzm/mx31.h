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
#ifndef __ASM_ARM_ARCH_MX31_H_
#define __ASM_ARM_ARCH_MX31_H_

#include <asm/arch/mx31_pins.h>

/* FIXME: need to remove to platform device config */
#ifdef CONFIG_MACH_MX3KZ /* { */
#define MXC_TIMER_CLK           53200000
#else /* }{ */
#define MXC_TIMER_CLK           66500000
#endif /* } */
#define MXC_TIMER_DIVIDER       5

/*!
 * Register an interrupt handler for the SMN as well as the SCC.  In some
 * implementations, the SMN is not connected at all, and in others, it is
 * on the same interrupt line as the SCM. Comment this line out accordingly
 */
#define USE_SMN_INTERRUPT

/* end FIXME */

/*
 * UART Chip level Configuration that a user may not have to edit. These
 * configuration vary depending on how the UART module is integrated with
 * the ARM core
 */
#define MXC_UART_NR 5

#if defined(CONFIG_CELL)
#define CLOCK_TICK_RATE     MXC_TIMER_CLK
#endif
/*!
 * This option is used to set or clear the RXDMUXSEL bit in control reg 3.
 * Certain platforms need this bit to be set in order to receive Irda data.
 */
#define MXC_UART_IR_RXDMUX      0x0004
/*!
 * This option is used to set or clear the RXDMUXSEL bit in control reg 3.
 * Certain platforms need this bit to be set in order to receive UART data.
 */
#define MXC_UART_RXDMUX         0x0004

/*
 * IRAM
 */
#define IRAM_BASE_ADDR		0x1FFC0000	/* internal ram */
//#define IRAM_BASE_ADDR_VIRT	0xD0000000
#define IRAM_SIZE		SZ_16K

/*
 * L2CC
 */
#define L2CC_BASE_ADDR		0x30000000
//#define L2CC_BASE_ADDR_VIRT	0xD1000000
#define L2CC_SIZE		SZ_1M

/*
 * AIPS 1
 */
#define AIPS1_BASE_ADDR 	0x43F00000
//#define AIPS1_BASE_ADDR_VIRT	0xD4000000
#define AIPS1_SIZE		SZ_1M

#define MAX_BASE_ADDR		(AIPS1_BASE_ADDR + 0x00004000)
#define EVTMON_BASE_ADDR	(AIPS1_BASE_ADDR + 0x00008000)
#define CLKCTL_BASE_ADDR	(AIPS1_BASE_ADDR + 0x0000C000)
#define ETB_SLOT4_BASE_ADDR	(AIPS1_BASE_ADDR + 0x00010000)
#define ETB_SLOT5_BASE_ADDR	(AIPS1_BASE_ADDR + 0x00014000)
#define ECT_CTIO_BASE_ADDR	(AIPS1_BASE_ADDR + 0x00018000)
#define I2C_BASE_ADDR		(AIPS1_BASE_ADDR + 0x00080000)
#define I2C3_BASE_ADDR		(AIPS1_BASE_ADDR + 0x00084000)
#define OTG_BASE_ADDR		(AIPS1_BASE_ADDR + 0x00088000)
#define ATA_BASE_ADDR		(AIPS1_BASE_ADDR + 0x0008C000)
#define UART1_BASE_ADDR 	(AIPS1_BASE_ADDR + 0x00090000)
#define UART2_BASE_ADDR 	(AIPS1_BASE_ADDR + 0x00094000)
#define I2C2_BASE_ADDR		(AIPS1_BASE_ADDR + 0x00098000)
#define OWIRE_BASE_ADDR 	(AIPS1_BASE_ADDR + 0x0009C000)
#define SSI1_BASE_ADDR		(AIPS1_BASE_ADDR + 0x000A0000)
#define CSPI1_BASE_ADDR 	(AIPS1_BASE_ADDR + 0x000A4000)
#define KPP_BASE_ADDR		(AIPS1_BASE_ADDR + 0x000A8000)
#define IOMUXC_BASE_ADDR	(AIPS1_BASE_ADDR + 0x000AC000)
#define UART4_BASE_ADDR 	(AIPS1_BASE_ADDR + 0x000B0000)
#define UART5_BASE_ADDR 	(AIPS1_BASE_ADDR + 0x000B4000)
#define ECT_IP1_BASE_ADDR	(AIPS1_BASE_ADDR + 0x000B8000)
#define ECT_IP2_BASE_ADDR	(AIPS1_BASE_ADDR + 0x000BC000)

/*
 * SPBA global module enabled #0
 */
#define SPBA0_BASE_ADDR 	0x50000000
//#define SPBA0_BASE_ADDR_VIRT	0xD4100000
#define SPBA0_SIZE		SZ_1M

#define MMC_SDHC1_BASE_ADDR	(SPBA0_BASE_ADDR + 0x00004000)
#define MMC_SDHC2_BASE_ADDR	(SPBA0_BASE_ADDR + 0x00008000)
#define UART3_BASE_ADDR 	(SPBA0_BASE_ADDR + 0x0000C000)
#define CSPI2_BASE_ADDR 	(SPBA0_BASE_ADDR + 0x00010000)
#define SSI2_BASE_ADDR		(SPBA0_BASE_ADDR + 0x00014000)
#define SIM_BASE_ADDR		(SPBA0_BASE_ADDR + 0x00018000)
#define IIM_BASE_ADDR		(SPBA0_BASE_ADDR + 0x0001C000)
#define ATA_DMA_BASE_ADDR	(SPBA0_BASE_ADDR + 0x00020000)
#define MSHC1_BASE_ADDR		(SPBA0_BASE_ADDR + 0x00024000)
#define MSHC2_BASE_ADDR		(SPBA0_BASE_ADDR + 0x00024000)
#define SPBA_CTRL_BASE_ADDR	(SPBA0_BASE_ADDR + 0x0003C000)

/*!
 * defines for SPBA modules
 */
#define SPBA_SDHC1	0x04
#define SPBA_SDHC2	0x08
#define SPBA_UART3	0x0C
#define SPBA_CSPI2	0x10
#define SPBA_SSI2	0x14
#define SPBA_SIM	0x18
#define SPBA_IIM	0x1C
#define SPBA_ATA	0x20

/*
 * AIPS 2
 */
#define AIPS2_BASE_ADDR		0x53F00000
//#define AIPS2_BASE_ADDR_VIRT	0xD4200000
#define AIPS2_SIZE		SZ_1M
#define CCM_BASE_ADDR		(AIPS2_BASE_ADDR + 0x00080000)
#define CSPI3_BASE_ADDR		(AIPS2_BASE_ADDR + 0x00084000)
#define FIRI_BASE_ADDR		(AIPS2_BASE_ADDR + 0x0008C000)
#define GPT1_BASE_ADDR		(AIPS2_BASE_ADDR + 0x00090000)
#define EPIT1_BASE_ADDR		(AIPS2_BASE_ADDR + 0x00094000)
#define EPIT2_BASE_ADDR		(AIPS2_BASE_ADDR + 0x00098000)
#define GPIO3_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000A4000)
#define SCC_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000AC000)
#define SCM_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000AE000)
#define SMN_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000AF000)
#define RNGA_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000B0000)
#define IPU_CTRL_BASE_ADDR	(AIPS2_BASE_ADDR + 0x000C0000)
#define AUDMUX_BASE_ADDR	(AIPS2_BASE_ADDR + 0x000C4000)
#define MPEG4_ENC_BASE_ADDR	(AIPS2_BASE_ADDR + 0x000C8000)
#define GPIO1_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000CC000)
#define GPIO2_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000D0000)
#define SDMA_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000D4000)
#define RTC_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000D8000)
#define WDOG_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000DC000)
#define PWM_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000E0000)
#define RTIC_BASE_ADDR		(AIPS2_BASE_ADDR + 0x000EC000)

/*
 * ROMP and AVIC
 */
#define ROMP_BASE_ADDR		0x60000000
//#define ROMP_BASE_ADDR_VIRT	0xD4500000
#define ROMP_SIZE		SZ_1M

#define AVIC_BASE_ADDR		0x68000000
//#define AVIC_BASE_ADDR_VIRT	0xD4400000
#define AVIC_SIZE		SZ_1M

/*
 * NAND, SDRAM, WEIM, M3IF, EMI controllers
 */
#define X_MEMC_BASE_ADDR	0xB8000000
//#define X_MEMC_BASE_ADDR_VIRT	0xD4320000
#define X_MEMC_SIZE		SZ_64K

#if defined(CONFIG_DEBUG_KZM_ETM_USE) /* { */
#define NFC_BASE_ADDR		(0xB6200000 + 0x0000)
//#define KMC_NFC_BASE_ADDR_VIRT	(0xEC200000 + 0x0000)
#else /* }{ */
#define NFC_BASE_ADDR		(X_MEMC_BASE_ADDR + 0x0000)
#endif /* } */
#define ESDCTL_BASE_ADDR	(X_MEMC_BASE_ADDR + 0x1000)
#define WEIM_BASE_ADDR		(X_MEMC_BASE_ADDR + 0x2000)
#define M3IF_BASE_ADDR		(X_MEMC_BASE_ADDR + 0x3000)
#define EMI_CTL_BASE_ADDR	(X_MEMC_BASE_ADDR + 0x4000)
#define PCMCIA_CTL_BASE_ADDR	EMI_CTL_BASE_ADDR

/*
 * Memory regions and CS
 */
/* MX31 ADS SDRAM is from 0x80000000, 64M */
#define SDRAM_BASE_ADDR         0x80000000
#define IPU_MEM_BASE_ADDR       0x70000000
#define CSD1_BASE_ADDR          0x90000000
#define CS0_BASE_ADDR           0xA0000000
#define CS1_BASE_ADDR           0xA8000000
#define CS2_BASE_ADDR           0xB0000000
#define CS3_BASE_ADDR           0xB2000000

#define CS4_BASE_ADDR           0xB4000000
//#define CS4_BASE_ADDR_VIRT      0xEB000000
#define CS4_SIZE                SZ_16M

#define CS5_BASE_ADDR           0xB6000000
#define PCMCIA_MEM_BASE_ADDR    0xBC000000

#if defined(CONFIG_MACH_MX3KZ) /* { */
//#define	CS5_BASE_ADDR_VIRT	0xEC000000
#define	CS5_SIZE		SZ_16M
#endif /* } */

#if 0
/*!
 * This macro defines the physical to virtual address mapping for all the
 * peripheral modules. It is used by passing in the physical address as x
 * and returning the virtual address. If the physical address is not mapped,
 * it returns 0xDEADBEEF
 */
#define IO_ADDRESS(x)   \
        (((x >= IRAM_BASE_ADDR) && (x < (IRAM_BASE_ADDR + IRAM_SIZE))) ? IRAM_IO_ADDRESS(x):\
        ((x >= L2CC_BASE_ADDR) && (x < (L2CC_BASE_ADDR + L2CC_SIZE))) ? L2CC_IO_ADDRESS(x):\
        ((x >= AIPS1_BASE_ADDR) && (x < (AIPS1_BASE_ADDR + AIPS1_SIZE))) ? AIPS1_IO_ADDRESS(x):\
        ((x >= SPBA0_BASE_ADDR) && (x < (SPBA0_BASE_ADDR + SPBA0_SIZE))) ? SPBA0_IO_ADDRESS(x):\
        ((x >= AIPS2_BASE_ADDR) && (x < (AIPS2_BASE_ADDR + AIPS2_SIZE))) ? AIPS2_IO_ADDRESS(x):\
        ((x >= ROMP_BASE_ADDR) && (x < (ROMP_BASE_ADDR + ROMP_SIZE))) ? ROMP_IO_ADDRESS(x):\
        ((x >= AVIC_BASE_ADDR) && (x < (AVIC_BASE_ADDR + AVIC_SIZE))) ? AVIC_IO_ADDRESS(x):\
        ((x >= CS4_BASE_ADDR) && (x < (CS4_BASE_ADDR + CS4_SIZE))) ? CS4_IO_ADDRESS(x):\
        ((x >= X_MEMC_BASE_ADDR) && (x < (X_MEMC_BASE_ADDR + X_MEMC_SIZE))) ? X_MEMC_IO_ADDRESS(x):\
        0xDEADBEEF)

/*
 * define the address mapping macros: in physical address order
 */

#define IRAM_IO_ADDRESS(x)  \
        (((x) - IRAM_BASE_ADDR) + IRAM_BASE_ADDR_VIRT)

#define L2CC_IO_ADDRESS(x)  \
        (((x) - L2CC_BASE_ADDR) + L2CC_BASE_ADDR_VIRT)

#define AIPS1_IO_ADDRESS(x)  \
        (((x) - AIPS1_BASE_ADDR) + AIPS1_BASE_ADDR_VIRT)

#define SPBA0_IO_ADDRESS(x)  \
        (((x) - SPBA0_BASE_ADDR) + SPBA0_BASE_ADDR_VIRT)

#define AIPS2_IO_ADDRESS(x)  \
        (((x) - AIPS2_BASE_ADDR) + AIPS2_BASE_ADDR_VIRT)

#define ROMP_IO_ADDRESS(x)  \
        (((x) - ROMP_BASE_ADDR) + ROMP_BASE_ADDR_VIRT)

#define AVIC_IO_ADDRESS(x)  \
        (((x) - AVIC_BASE_ADDR) + AVIC_BASE_ADDR_VIRT)

#define CS4_IO_ADDRESS(x)  \
        (((x) - CS4_BASE_ADDR) + CS4_BASE_ADDR_VIRT)

#define X_MEMC_IO_ADDRESS(x)  \
        (((x) - X_MEMC_BASE_ADDR) + X_MEMC_BASE_ADDR_VIRT)

#define PCMCIA_IO_ADDRESS(x) \
	(((x) - X_MEMC_BASE_ADDR) + X_MEMC_BASE_ADDR_VIRT)

#if defined(CONFIG_DEBUG_KZM_ETM_USE) /* { */
#define KMC_NFC_ADDRESS(x) \
	(((x) - NFC_BASE_ADDR) + KMC_NFC_BASE_ADDR_VIRT)
#endif /* } */
#endif

/*
 * DMA request assignments
 */
#define DMA_REQ_GPIO1_0    31
#define DMA_REQ_NFC        30
#define DMA_REQ_SSI1_TX1   29
#define DMA_REQ_SSI1_RX1   28
#define DMA_REQ_SSI1_TX2   27
#define DMA_REQ_SSI1_RX2   26
#define DMA_REQ_SSI2_TX1   25
#define DMA_REQ_SSI2_RX1   24
#define DMA_REQ_SSI2_TX2   23
#define DMA_REQ_SSI2_RX2   22
#define DMA_REQ_SDHC2      21
#define DMA_REQ_SDHC1      20
#define DMA_REQ_UART1_TX   19
#define DMA_REQ_UART1_RX   18
#define DMA_REQ_FIRI_RX    17
#define DMA_REQ_FIRI_TX    16
#define DMA_REQ_UART2_TX   17
#define DMA_REQ_UART2_RX   16
#define DMA_REQ_GPIO1_1    15
#define DMA_REQ_GPIO1_2    14
#define DMA_REQ_UART4_TX   13
#define DMA_REQ_UART4_RX   12
#define DMA_REQ_UART5_TX   11
#define DMA_REQ_UART5_RX   10
#define DMA_REQ_CSPI1_TX   9
#define DMA_REQ_CSPI1_RX   8
#define DMA_REQ_UART3_TX   9
#define DMA_REQ_UART3_RX   8
#define DMA_REQ_CSPI2_TX   7
#define DMA_REQ_CSPI2_RX   6
#define DMA_REQ_SIM        5
#define DMA_REQ_ATA_RX     4
#define DMA_REQ_ATA_TX     3
#define DMA_REQ_ATA_TX_END 2
#define DMA_REQ_CCM        1
#define DMA_REQ_reserved   0

/*
 * Interrupt numbers
 */
#define MXC_INT_BASE		0
#define INT_PEN_ADS7843         0
#define INT_RESV1               1
#define INT_CS8900A             2
#define INT_I2C3                3
#define INT_I2C2                4
#define INT_MPEG4_ENCODER       5
#define INT_RTIC                6
#define INT_FIRI                7
#define INT_MMC_SDHC2           8
#define INT_MMC_SDHC1           9
#define INT_I2C                 10
#define INT_SSI2                11
#define INT_SSI1                12
#define INT_CSPI2               13
#define INT_CSPI1               14
#define INT_ATA                 15
#define INT_MBX                 16
#define INT_CSPI3               17
#define INT_UART3               18
#define INT_IIM                 19
#define INT_SIM2                20
#define INT_SIM1                21
#define INT_RNGA                22
#define INT_EVTMON              23
#define INT_KPP                 24
#define INT_RTC                 25
#define INT_PWM                 26
#define INT_EPIT2               27
#define INT_EPIT1               28
#define INT_GPT                 29
#define INT_RESV30              30
#define INT_RESV31              31
#define INT_UART2               32
#define INT_NANDFC              33
#define INT_SDMA                34
#define INT_USB1                35
#define INT_USB2                36
#define INT_USB3                37
#define INT_USB4                38
#define INT_MSHC1               39
#define INT_MSHC2               40
#define INT_IPU_ERR             41
#define INT_IPU_SYN             42
#define INT_RESV43              43
#define INT_RESV44              44
#define INT_UART1               45
#define INT_UART4               46
#define INT_UART5               47
#define INT_ECT                 48
#define INT_SCC_SCM             49
#define INT_SCC_SMN             50
#define INT_GPIO2               51
#define INT_GPIO1               52
#define INT_CCM                 53
#define INT_PCMCIA              54
#define INT_WDOG                55
#define INT_GPIO3               56
#define INT_RESV57              57
#define INT_EXT_POWER           58
#define INT_EXT_TEMPER          59
#define INT_EXT_SENSOR60        60
#define INT_EXT_SENSOR61        61
#define INT_EXT_WDOG            62
#define INT_EXT_TV              63

#define MXC_MAX_INT_LINES       63

/*!
 * Interrupt Number for ARM11 PMU
 */
#define ARM11_PMU_IRQ		INT_EVTMON

#define	MXC_GPIO_BASE		(MXC_MAX_INT_LINES + 1)

/*!
 * Number of GPIO port as defined in the IC Spec
 */
#define GPIO_PORT_NUM           3
/*!
 * Number of GPIO pins per port
 */
#define GPIO_NUM_PIN            32

#define PROD_SIGNATURE        0x1	/* For MX31 */
#define CHIP_REV_1_0      0x10
#define CHIP_REV_1_1      0x11
#define CHIP_REV_2_0      0x20

#define SYSTEM_REV_MIN          CHIP_REV_1_0
#define SYSTEM_REV_NUM          3

/*
 * Used for 1-Wire
 */
#define owire_read(a) (__raw_readw(a))
#define owire_write(v,a) (__raw_writew(v,a))

#endif				/*  __ASM_ARM_ARCH_MX31_H_ */
