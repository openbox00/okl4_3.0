
/*
 * PXA i2Sound code for iBox
 * It uses PXA2xx i2Sound and CS4270 modules
 *
 * Copyright (c) 2006 David Snowdon, National ICT Australia. 
 * Copyright (c) 2005 Todd Blumer, SDG Systems, LLC
 *
 * Based on code:
 *   Copyright (c) 2005 Giorgio Padrin giorgio@mandarinlogiq.org
 *   Copyright (c) 2004,2005 Matthew Reimer
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * History:
 * 
 * 2006         David Snowdon           Forked for iBox audio glue. 
 * 2005-06	Todd Blumer		Initial release
 */

#include <linux/module.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <asm/hardware.h>
#include <linux/pm.h>
#include <asm/arch/pxa-regs.h>

#include "pxa2xx-i2sound.h"
#ifdef CONFIG_SND_IBOX
#include <sound/cs4270.h>
#endif
#ifdef CONFIG_SND_IBOX_WOMBAT
#include <sound/cs4270_l4i2c.h>
#endif


static int
snd_ibox_audio_activate(void)
{
	snd_pxa2xx_i2sound_i2slink_get(); /* I2SLINK clocks required always on
					   * by AK4641 module */ 
	
		printk("Requesting I2C module\n"); 
		request_module("i2c-pxa");
	
	printk("Activating ibox audio\n"); 

	if (snd_cs4270_activate() == 0) { 
		// General setup here
		// Perhaps setting up preferred volume, serial mode, etc 
		return 0;
	}
	else {
		// General teardown here, since our initialisation failed
		return 1;
	}
}

static void
snd_ibox_audio_deactivate(void)
{
  // Do power, volume, etc, activation here. 
  snd_cs4270_deactivate();
	
  snd_pxa2xx_i2sound_i2slink_free();
}

static int
snd_ibox_audio_open_stream(int stream)
{
#ifdef DEBUG_VERBOSE
	if (stream == SNDRV_PCM_STREAM_PLAYBACK) {
	  printk("ibox_audio: opening output stream\n"); 
	}
	else
	  printk("ibox_audio: opening input stream\n"); 
#endif
	return 0;
}

static void
snd_ibox_audio_close_stream(int stream)
{
#ifdef DEBUG_VERBOSE
	if (stream == SNDRV_PCM_STREAM_PLAYBACK) {
	  printk("ibox_audio: closing output stream\n"); 
	}
	else
	  printk("ibox_audio: closing input stream\n"); 
#endif
}		

int
snd_cs4270_mixer_create( snd_card_t *_card )
{
	return -1; 
}

void
snd_cs4270_mixer_free( void )
{
	return;
}

		

#ifdef CONFIG_PM

static int
snd_ibox_audio_suspend(pm_message_t state)
{
#ifdef DEBUG_VERBOSE
	printk("ibox_audio: suspend\n"); 
#endif

  //	ak4641_command(ak4641_i2c_get_client(), I2C_AK4641_CLOSE, NULL);
	return 0;
}

static int
snd_ibox_audio_resume(void)
{
#ifdef DEBUG_VERBOSE
        printk("ibox_audio: resume\n"); 
#endif
  //	ak4641_command(ak4641_i2c_get_client(), I2C_AK4641_OPEN, NULL);
	return 0;
}

#endif

static struct snd_pxa2xx_i2sound_board ibox_audio = {
	.name			= "iBox Audio",
	.desc			= "iBox Audio via CS4270 ADC",
	.acard_id		= "ibox-audio",
	.info			= SND_PXA2xx_I2SOUND_INFO_CAN_CAPTURE | 
	                          SND_PXA2xx_I2SOUND_INFO_CLOCK_FROM_PXA,
	.activate		= snd_ibox_audio_activate,
	.deactivate		= snd_ibox_audio_deactivate,
	.open_stream		= snd_ibox_audio_open_stream,
	.close_stream		= snd_ibox_audio_close_stream,
	.add_mixer_controls	= snd_cs4270_mixer_create,
#ifdef CONFIG_PM	
	.suspend		= snd_ibox_audio_suspend,
	.resume			= snd_ibox_audio_resume
#endif
};


static int __init
snd_pxa2xx_i2sound_ibox_init(void)
{
	int ret; 
	printk( KERN_NOTICE "ibox_audio: iBox i2Sound Module\n" );
	ret = snd_pxa2xx_i2sound_card_activate( &ibox_audio );
	return ret; 
}

static void __exit
snd_pxa2xx_i2sound_ibox_free(void)
{
	printk( KERN_NOTICE "iBox i2Sound module deactivating\n"); 
	snd_pxa2xx_i2sound_card_deactivate();
}

module_init(snd_pxa2xx_i2sound_ibox_init);
module_exit(snd_pxa2xx_i2sound_ibox_free);

MODULE_AUTHOR("David Snowdon, National ICT Australia Pty. Ltd.");
MODULE_DESCRIPTION("Audio Support for iBox");
MODULE_LICENSE("GPL");
