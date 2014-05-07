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

/*  Front-end to the DMA handling.  This handles the allocation/freeing
 *  of DMA channels, and provides a unified interface to the machines
 *  DMA facilities.
 */

/*!
 * @file dma.c
 * @brief This file contains functions for Smart DMA  API
 *
 * SDMA (Smart DMA) is used for transferring data between MCU and peripherals
 *
 * @ingroup SDMA
 */

#include <linux/init.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <asm/dma.h>
#include <asm/scatterlist.h>
#include INC_SYSTEM2(mach/dma.h)
#include <linux/interrupt.h>
#include <asm/arch/hardware.h>

#include <asm/semaphore.h>
#include <linux/spinlock.h>

#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>

#ifdef CONFIG_MXC_SDMA_API

static int bd_index[MAX_DMA_CHANNELS];

/*!
 * Request dma channel. Standard DMA API handler.
 *
 * @param   channel           channel number
 * @param   dma               dma structure
 */
static int dma_api_request(dmach_t channel, dma_t * dma)
{
	int res;

	if (channel == 0) {
		res = -EBUSY;
	} else {
		res = mxc_request_dma(&channel, "DMA_API");
		if (res == 0) {
			bd_index[channel] = 0;
		}
	}
	return res;
}

/*!
 * Free dma channel. Standard DMA API handler.
 *
 * @param   channel           channel number
 * @param   dma               dma structure
 */
static void dma_api_free(dmach_t channel, dma_t * dma)
{
	mxc_free_dma(channel);
}

/*!
 * Enable dma channel. Standard DMA API handler.
 *
 * @param   channel           channel number
 * @param   dma               dma structure
 */
static void dma_api_enable(dmach_t channel, dma_t * dma)
{
	dma_request_t r_params;

	mxc_dma_get_config(channel, &r_params, bd_index[channel]);
	r_params.count = dma->buf.length;

	if (dma->dma_mode == DMA_MODE_READ) {
		r_params.destAddr = dma->addr;
	} else if (dma->dma_mode == DMA_MODE_WRITE) {
		r_params.sourceAddr = dma->addr;
	}

	mxc_dma_set_config(channel, &r_params, bd_index[channel]);
	bd_index[channel] = (bd_index[channel] + 1) % MAX_BD_NUMBER;

	mxc_dma_start(channel);
}

/*!
 * Disable dma channel. Standard DMA API handler.
 *
 * @param   channel           channel number
 * @param   dma               dma structure
 */
static void dma_api_disable(dmach_t channel, dma_t * dma)
{
	mxc_dma_stop(channel);
}

/*!
 * This structure contains the pointers to the control functions that are
 * invoked by the standard dma api driver.
 * The structure is passed to dma.c file during initialization.
 */
struct dma_ops dma_operations = {
	.request = dma_api_request,
	.free = dma_api_free,
	.enable = dma_api_enable,
	.disable = dma_api_disable,
	.type = "sdma",
};

/*!
 * Initializes dma structure with dma_operations
 *
 * @param   dma           dma structure
 */
void __init arch_dma_init(dma_t * dma)
{
	int i;

	for (i = 0; i < MAX_DMA_CHANNELS; i++) {
		dma[i].d_ops = &dma_operations;
	}
}

module_init(sdma_init);

#else
int mxc_request_dma(int *channel, const char *devicename)
{
	return -ENODEV;
}

int mxc_dma_setup_channel(int channel, dma_channel_params * p)
{
	return -ENODEV;
}

int mxc_dma_set_config(int channel, dma_request_t * p, int bd_index)
{
	return -ENODEV;
}

int mxc_dma_get_config(int channel, dma_request_t * p, int bd_index)
{
	return -ENODEV;
}

int mxc_dma_start(int channel)
{
	return -ENODEV;
}

int mxc_dma_stop(int channel)
{
	return -ENODEV;
}

void mxc_free_dma(int channel)
{
}

void mxc_dma_set_callback(int channel, dma_callback_t callback, void *arg)
{
}

void *sdma_malloc(size_t size)
{
	return 0;
}

void sdma_free(void *buf)
{
}

void *sdma_phys_to_virt(unsigned long buf)
{
	return 0;
}

unsigned long sdma_virt_to_phys(void *buf)
{
	return 0;
}

EXPORT_SYMBOL(mxc_request_dma);
EXPORT_SYMBOL(mxc_dma_setup_channel);
EXPORT_SYMBOL(mxc_dma_set_config);
EXPORT_SYMBOL(mxc_dma_get_config);
EXPORT_SYMBOL(mxc_dma_start);
EXPORT_SYMBOL(mxc_dma_stop);
EXPORT_SYMBOL(mxc_free_dma);
EXPORT_SYMBOL(mxc_dma_set_callback);
EXPORT_SYMBOL(sdma_malloc);
EXPORT_SYMBOL(sdma_free);
EXPORT_SYMBOL(sdma_phys_to_virt);
EXPORT_SYMBOL(sdma_virt_to_phys);

#endif

MODULE_AUTHOR("Freescale Semiconductor, Inc.");
MODULE_DESCRIPTION("MXC Linux SDMA API");
MODULE_LICENSE("GPL");
