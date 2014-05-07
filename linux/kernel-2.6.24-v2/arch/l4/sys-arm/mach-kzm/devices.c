/*
 * Author: MontaVista Software, Inc.
 *       <source@mvista.com>
 *
 * Based on the OMAP devices.c
 *
 * 2005 (c) MontaVista Software, Inc. This file is licensed under the
 * terms of the GNU General Public License version 2. This program is
 * licensed "as is" without any warranty of any kind, whether express
 * or implied.
 *
 * Copyright 2005-2006 Freescale Semiconductor, Inc. All Rights Reserved.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>

#include <asm/hardware.h>

#include <asm/arch/sdma.h>
#include <asm/arch/sdma_script_code.h>
#include <asm/arch/sdma_script_code_pass2.h>

#if 0
int board_device_enable(u32 device_id);
int board_device_disable(u32 device_id);

int mxc_device_enable(u32 device_id)
{
	int ret = 0;

	switch (device_id) {
	default:
		ret = board_device_enable(device_id);
	}

	return ret;
}

int mxc_device_disable(u32 device_id)
{
	int ret = 0;

	switch (device_id) {
	default:
		ret = board_device_disable(device_id);
	}
	return ret;
}

EXPORT_SYMBOL(mxc_device_enable);
EXPORT_SYMBOL(mxc_device_disable);
#endif

void mxc_sdma_get_script_info(sdma_script_start_addrs * sdma_script_addr)
{
	if (1 /*system_rev == CHIP_REV_1_0*/) {
		sdma_script_addr->mxc_sdma_app_2_mcu_addr = app_2_mcu_ADDR;
		sdma_script_addr->mxc_sdma_ap_2_ap_addr = ap_2_ap_ADDR;
		sdma_script_addr->mxc_sdma_ap_2_bp_addr = -1;
		sdma_script_addr->mxc_sdma_bp_2_ap_addr = -1;
		sdma_script_addr->mxc_sdma_loopback_on_dsp_side_addr = -1;
		sdma_script_addr->mxc_sdma_mcu_2_app_addr = mcu_2_app_ADDR;
		sdma_script_addr->mxc_sdma_mcu_2_shp_addr = mcu_2_shp_ADDR;
		sdma_script_addr->mxc_sdma_mcu_interrupt_only_addr = -1;
		sdma_script_addr->mxc_sdma_shp_2_mcu_addr = shp_2_mcu_ADDR;
		sdma_script_addr->mxc_sdma_start_addr =
		    (unsigned short *)sdma_code;
		sdma_script_addr->mxc_sdma_uartsh_2_mcu_addr =
		    uartsh_2_mcu_ADDR;
		sdma_script_addr->mxc_sdma_uart_2_mcu_addr = uart_2_mcu_ADDR;
		sdma_script_addr->mxc_sdma_ram_code_size = RAM_CODE_SIZE;
		sdma_script_addr->mxc_sdma_ram_code_start_addr =
		    RAM_CODE_START_ADDR;
		sdma_script_addr->mxc_sdma_dptc_dvfs_addr = dptc_dvfs_ADDR;
		sdma_script_addr->mxc_sdma_firi_2_mcu_addr = firi_2_mcu_ADDR;
		sdma_script_addr->mxc_sdma_firi_2_per_addr = -1;
		sdma_script_addr->mxc_sdma_mshc_2_mcu_addr = mshc_2_mcu_ADDR;
		sdma_script_addr->mxc_sdma_per_2_app_addr = -1;
		sdma_script_addr->mxc_sdma_per_2_firi_addr = -1;
		sdma_script_addr->mxc_sdma_per_2_shp_addr = -1;
		sdma_script_addr->mxc_sdma_mcu_2_ata_addr = mcu_2_ata_ADDR;
		sdma_script_addr->mxc_sdma_mcu_2_firi_addr = mcu_2_firi_ADDR;
		sdma_script_addr->mxc_sdma_mcu_2_mshc_addr = mcu_2_mshc_ADDR;
		sdma_script_addr->mxc_sdma_ata_2_mcu_addr = ata_2_mcu_ADDR;
		sdma_script_addr->mxc_sdma_uartsh_2_per_addr = -1;
		sdma_script_addr->mxc_sdma_shp_2_per_addr = -1;
		sdma_script_addr->mxc_sdma_uart_2_per_addr = -1;
		sdma_script_addr->mxc_sdma_app_2_per_addr = -1;
	} else {
		sdma_script_addr->mxc_sdma_app_2_mcu_addr = app_2_mcu_ADDR_2;
		sdma_script_addr->mxc_sdma_ap_2_ap_addr = ap_2_ap_ADDR_2;
		sdma_script_addr->mxc_sdma_ap_2_bp_addr = ap_2_bp_ADDR_2;
		sdma_script_addr->mxc_sdma_bp_2_ap_addr = bp_2_ap_ADDR_2;
		sdma_script_addr->mxc_sdma_loopback_on_dsp_side_addr = -1;
		sdma_script_addr->mxc_sdma_mcu_2_app_addr = mcu_2_app_ADDR_2;
		sdma_script_addr->mxc_sdma_mcu_2_shp_addr = mcu_2_shp_ADDR_2;
		sdma_script_addr->mxc_sdma_mcu_interrupt_only_addr = -1;
		sdma_script_addr->mxc_sdma_shp_2_mcu_addr = shp_2_mcu_ADDR_2;
		sdma_script_addr->mxc_sdma_start_addr =
		    (unsigned short *)sdma_code_2;
		sdma_script_addr->mxc_sdma_uartsh_2_mcu_addr =
		    uartsh_2_mcu_ADDR_2;
		sdma_script_addr->mxc_sdma_uart_2_mcu_addr = uart_2_mcu_ADDR_2;
		sdma_script_addr->mxc_sdma_ram_code_size = RAM_CODE_SIZE_2;
		sdma_script_addr->mxc_sdma_ram_code_start_addr =
		    RAM_CODE_START_ADDR_2;
		sdma_script_addr->mxc_sdma_dptc_dvfs_addr = dptc_dvfs_ADDR_2;
		sdma_script_addr->mxc_sdma_firi_2_mcu_addr = firi_2_mcu_ADDR_2;
		sdma_script_addr->mxc_sdma_firi_2_per_addr = firi_2_per_ADDR_2;
		sdma_script_addr->mxc_sdma_mshc_2_mcu_addr = mshc_2_mcu_ADDR_2;
		sdma_script_addr->mxc_sdma_per_2_app_addr = per_2_app_ADDR_2;
		sdma_script_addr->mxc_sdma_per_2_firi_addr = per_2_firi_ADDR_2;
		sdma_script_addr->mxc_sdma_per_2_shp_addr = per_2_shp_ADDR_2;
		sdma_script_addr->mxc_sdma_mcu_2_ata_addr = mcu_2_ata_ADDR_2;
		sdma_script_addr->mxc_sdma_mcu_2_firi_addr = mcu_2_firi_ADDR_2;
		sdma_script_addr->mxc_sdma_mcu_2_mshc_addr = mcu_2_mshc_ADDR_2;
		sdma_script_addr->mxc_sdma_ata_2_mcu_addr = ata_2_mcu_ADDR_2;
		sdma_script_addr->mxc_sdma_uartsh_2_per_addr =
		    uartsh_2_per_ADDR_2;
		sdma_script_addr->mxc_sdma_shp_2_per_addr = shp_2_per_ADDR_2;
		sdma_script_addr->mxc_sdma_uart_2_per_addr = uart_2_per_ADDR_2;
		sdma_script_addr->mxc_sdma_app_2_per_addr = app_2_per_ADDR_2;
	}
}

void mxc_nop_release(struct device *dev)
{
	/* Nothing */
}
#if 0
#if defined(CONFIG_MXC_WATCHDOG) || defined(CONFIG_MXC_WATCHDOG_MODULE)

static struct resource wdt_resources[] = {
	{
	 .start = WDOG_BASE_ADDR,
	 .end = WDOG_BASE_ADDR + 0x30,
	 .flags = IORESOURCE_MEM,
	 },
};

static struct platform_device mxc_wdt_device = {
	.name = "mxc_wdt",
	.id = 0,
	.dev = {
		.release = mxc_nop_release,
		},
	.num_resources = ARRAY_SIZE(wdt_resources),
	.resource = wdt_resources,
};

static void mxc_init_wdt(void)
{
	(void)platform_device_register(&mxc_wdt_device);
}
#else
static inline void mxc_init_wdt(void)
{
}
#endif

#if defined(CONFIG_MXC_IPU) || defined(CONFIG_MXC_IPU_MODULE)
static struct mxc_ipu_config mxc_ipu_data = {
	.rev = 1,
};

static struct resource ipu_resources[] = {
	{
	 .start = IPU_CTRL_BASE_ADDR,
	 .end = IPU_CTRL_BASE_ADDR + SZ_4K,
	 .flags = IORESOURCE_MEM,
	 },
	{
	 .start = INT_IPU_SYN,
	 .flags = IORESOURCE_IRQ,
	 },
	{
	 .start = INT_IPU_ERR,
	 .flags = IORESOURCE_IRQ,
	 },
};

static struct platform_device mxc_ipu_device = {
	.name = "mxc_ipu",
	.id = -1,
	.dev = {
		.release = mxc_nop_release,
		.platform_data = &mxc_ipu_data,
		},
	.num_resources = ARRAY_SIZE(ipu_resources),
	.resource = ipu_resources,
};

static void mxc_init_ipu(void)
{
	platform_device_register(&mxc_ipu_device);
}
#else
static inline void mxc_init_ipu(void)
{
}
#endif

/* MMC device data */

#if defined(CONFIG_MMC_MXC) || defined(CONFIG_MMC_MXC_MODULE)
/*!
 * Resource definition for the SDHC1
 */
static struct resource mxcsdhc1_resources[] = {
	[0] = {
	       .start = MMC_SDHC1_BASE_ADDR,
	       .end = MMC_SDHC1_BASE_ADDR + SZ_16K - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = INT_MMC_SDHC1,
	       .end = INT_MMC_SDHC1,
	       .flags = IORESOURCE_IRQ,
	       }
};

/*!
 * Resource definition for the SDHC2
 */
static struct resource mxcsdhc2_resources[] = {
	[0] = {
	       .start = MMC_SDHC2_BASE_ADDR,
	       .end = MMC_SDHC2_BASE_ADDR + SZ_16K - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = INT_MMC_SDHC2,
	       .end = INT_MMC_SDHC2,
	       .flags = IORESOURCE_IRQ,
	       }
};

/*! Device Definition for MXC SDHC1 */
static struct platform_device mxcsdhc1_device = {
	.name = "mxcmci",
	.id = 0,
	.dev = {
		.release = mxc_nop_release,
		},
	.num_resources = ARRAY_SIZE(mxcsdhc1_resources),
	.resource = mxcsdhc1_resources,
};

/*! Device Definition for MXC SDHC2 */
static struct platform_device mxcsdhc2_device = {
	.name = "mxcmci",
	.id = 1,
	.dev = {
		.release = mxc_nop_release,
		},
	.num_resources = ARRAY_SIZE(mxcsdhc2_resources),
	.resource = mxcsdhc2_resources,
};

static inline void mxc_init_mmc(void)
{
	spba_take_ownership(SPBA_SDHC1, SPBA_MASTER_A | SPBA_MASTER_C);
	(void)platform_device_register(&mxcsdhc1_device);
	spba_take_ownership(SPBA_SDHC2, SPBA_MASTER_A | SPBA_MASTER_C);
	(void)platform_device_register(&mxcsdhc2_device);
}
#else
static inline void mxc_init_mmc(void)
{
}
#endif

struct mxc_gpio_port mxc_gpio_ports[GPIO_PORT_NUM] = {
	{
	 .num = 0,
	 .base = IO_ADDRESS(GPIO1_BASE_ADDR),
	 .irq = INT_GPIO1,
	 .virtual_irq_start = MXC_GPIO_BASE,
	 },
	{
	 .num = 1,
	 .base = IO_ADDRESS(GPIO2_BASE_ADDR),
	 .irq = INT_GPIO2,
	 .virtual_irq_start = MXC_GPIO_BASE + GPIO_NUM_PIN,
	 },
	{
	 .num = 2,
	 .base = IO_ADDRESS(GPIO3_BASE_ADDR),
	 .irq = INT_GPIO3,
	 .virtual_irq_start = MXC_GPIO_BASE + GPIO_NUM_PIN * 2,
	 },
};

#ifdef CONFIG_MXC_MC13783_LEGACY
static struct platform_device adc_ldm = {
	.name = "mc13783_adc",
	.id = 1,
};
static struct platform_device battery_ldm = {
	.name = "mc13783_battery",
	.id = 1,
};
static struct platform_device power_ldm = {
	.name = "mc13783_power",
	.id = 1,
};
static struct platform_device rtc_ldm = {
	.name = "mc13783_rtc",
	.id = 1,
};
static struct platform_device light_ldm = {
	.name = "mc13783_light",
	.id = 1,
};
static struct platform_device audio_ldm = {
	.name = "mc13783_audio",
	.id = 1,
};
static struct platform_device convity_ldm = {
	.name = "mc13783_connectivity",
	.id = 1,
};

#elif CONFIG_MXC_MC13783_PMIC

static struct platform_device adc_ldm = {
	.name = "pmic_adc",
	.id = 1,
};
static struct platform_device battery_ldm = {
	.name = "pmic_battery",
	.id = 1,
};
static struct platform_device power_ldm = {
	.name = "pmic_power",
	.id = 1,
};
static struct platform_device rtc_ldm = {
	.name = "pmic_rtc",
	.id = 1,
};
static struct platform_device light_ldm = {
	.name = "pmic_light",
	.id = 1,
};
/*
static struct platform_device audio_ldm = {
	.name = "pmic_audio",
	.id = 1,
};
static struct platform_device convity_ldm = {
	.name = "pmic_convity",
	.id = 1,
};
*/
#endif

static void mxc_init_pmic(void)
{
#if defined(CONFIG_MXC_MC13783_LEGACY) || defined(CONFIG_MXC_MC13783_PMIC)
	platform_device_register(&adc_ldm);
	platform_device_register(&battery_ldm);
	platform_device_register(&rtc_ldm);
	platform_device_register(&power_ldm);
	platform_device_register(&light_ldm);
#endif
#ifdef CONFIG_MXC_MC13783_LEGACY
	platform_device_register(&audio_ldm);
	platform_device_register(&convity_ldm);
#endif
}

#ifdef	CONFIG_MXC_FIR
/*! Resource definition for the UART & FIRI */
static struct resource mxc_irda_resources[] = {
	[0] = {
		.start  = UART2_BASE_ADDR,
		.end    = UART2_BASE_ADDR + SZ_16K - 1,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = INT_UART2,
		.end    = INT_UART2,
		.flags  = IORESOURCE_IRQ,
	},
	[2] = {
		.start  = FIRI_BASE_ADDR,
		.end    = FIRI_BASE_ADDR + SZ_16K - 1,
		.flags  = IORESOURCE_MEM,
	},
	[3] = {
		.start  = INT_FIRI,
		.end    = INT_FIRI,
		.flags  = IORESOURCE_IRQ,
	}
};

/*! Device definition for MXC IrDA */
static struct platform_device mxc_irda_device = {
	.name           = "mxc_irda",
	.id             = 0,
	.num_resources  = ARRAY_SIZE(mxc_irda_resources),
	.resource       = mxc_irda_resources,
};

static void mxc_init_irda(void)
{
	platform_device_register(&mxc_irda_device);
}
#else	/* CONFIG_MXC_FIR */
static inline void mxc_init_irda(void)
{
}
#endif /* CONFIG_MXC_FIR */ 

static int __init mxc_init_devices(void)
{
	mxc_init_wdt();
	mxc_init_ipu();
	mxc_init_mmc();
	mxc_init_irda();

	// TODO: MOVE ME
	spba_take_ownership(SPBA_CSPI2, SPBA_MASTER_A);
	/* SPBA configuration for SSI2 - SDMA and MCU are set */
	spba_take_ownership(SPBA_SSI2, SPBA_MASTER_C | SPBA_MASTER_A);
        mxc_init_pmic();
	return 0;
}

arch_initcall(mxc_init_devices);
#endif
