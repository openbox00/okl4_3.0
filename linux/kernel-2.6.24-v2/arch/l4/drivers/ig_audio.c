/*
 *  Iguana virtual soundcard
 *  Copyright (c) 2007, Open Kernel Labs Inc.
 *
 *  Based on:
 *  Dummy soundcard
 *  Copyright (c) by Jaroslav Kysela <perex@suse.cz>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <sound/driver.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/wait.h>
#include <linux/moduleparam.h>
#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm.h>
#include <sound/rawmidi.h>
#include <sound/initval.h>

#include <asm/irq.h>
#include <linux/interrupt.h>
#include <vaudio/audio.h>
#include <asm/driver/ig_audio.h>

#include <interfaces/devicecore_client.h>
#include <interfaces/vaudio_client.h>

#include <iguana/types.h>
#include <iguana/memsection.h>
#include <iguana/physmem.h>
#include <iguana/thread.h>
#include <iguana/object.h>
#include <iguana/env.h>


MODULE_AUTHOR("Carl van Schaik <carl@ok-labs.com>");
MODULE_DESCRIPTION("Iguana virtual soundcard");
MODULE_LICENSE("GPL");
MODULE_SUPPORTED_DEVICE("{{ALSA,Iguana}}");

//#define DEBUG_MSG(fmt, args...) printk(KERN_DEBUG fmt, ## args)
#define DEBUG_MSG(fmt, args...) printk(fmt, ## args)
//#define DEBUG_MSG(fmt, args...)
#define ERROR_MSG(fmt, args...) printk(KERN_NOTICE fmt, ## args)

#define MAX_PCM_DEVICES		1
#define MAX_PCM_SUBSTREAMS	1
#define MAX_MIDI_DEVICES	2

/* overrides */
#define USE_RATE		SNDRV_PCM_RATE_44100
#define USE_RATE_MIN		44100
#define USE_RATE_MAX		44100
#define USE_FORMATS 		SNDRV_PCM_FMTBIT_S16_LE
#define USE_CHANNELS_MIN 	2   // stereo only for now
#define USE_CHANNELS_MAX 	2

/* defaults */
#ifndef MAX_BUFFER_SIZE
#define MAX_BUFFER_SIZE		(64*1024)
#endif
#ifndef USE_FORMATS
#define USE_FORMATS 		(SNDRV_PCM_FMTBIT_U8 | SNDRV_PCM_FMTBIT_S16_LE)
#endif
#ifndef USE_RATE
#define USE_RATE		SNDRV_PCM_RATE_CONTINUOUS | SNDRV_PCM_RATE_8000_48000
#define USE_RATE_MIN		5500
#define USE_RATE_MAX		48000
#endif
#ifndef USE_CHANNELS_MIN
#define USE_CHANNELS_MIN 	1
#endif
#ifndef USE_CHANNELS_MAX
#define USE_CHANNELS_MAX 	2
#endif
#ifndef USE_PERIODS_MIN
#define USE_PERIODS_MIN 	1
#endif
#ifndef USE_PERIODS_MAX
#define USE_PERIODS_MAX 	1024
#endif
#ifndef add_playback_constraints
#define add_playback_constraints(x) 0
#endif
#ifndef add_capture_constraints
#define add_capture_constraints(x) 0
#endif

static int index[SNDRV_CARDS] = SNDRV_DEFAULT_IDX;	/* Index 0-MAX */
static char *id[SNDRV_CARDS] = SNDRV_DEFAULT_STR;	/* ID for this card */
static int enable[SNDRV_CARDS] = {1, [1 ... (SNDRV_CARDS - 1)] = 0};
static int pcm_devs[SNDRV_CARDS] = {[0 ... (SNDRV_CARDS - 1)] = 1};
static int pcm_substreams[SNDRV_CARDS] = {[0 ... (SNDRV_CARDS - 1)] = 1};

module_param_array(index, int, NULL, 0444);
MODULE_PARM_DESC(index, "Index value for iguana audio.");
module_param_array(id, charp, NULL, 0444);
MODULE_PARM_DESC(id, "ID string for iguana audio.");
module_param_array(enable, bool, NULL, 0444);
MODULE_PARM_DESC(enable, "Enable this iguana audio.");
module_param_array(pcm_devs, int, NULL, 0444);
MODULE_PARM_DESC(pcm_devs, "PCM devices # (0-4) for iguana audio.");
module_param_array(pcm_substreams, int, NULL, 0444);
MODULE_PARM_DESC(pcm_substreams, "PCM substreams # (1-16) for iguana audio.");
//module_param_array(midi_devs, int, NULL, 0444);
//MODULE_PARM_DESC(midi_devs, "MIDI devices # (0-2) for iguana driver.");

#define MIXER_ADDR_MASTER	0
#define MIXER_ADDR_LINE		1
#define MIXER_ADDR_MIC		2
#define MIXER_ADDR_LAST		2

typedef struct snd_card_iguana {
	snd_card_t *card;
	spinlock_t mixer_lock;
	int mixer_volume[MIXER_ADDR_LAST+1][2];
	int capture_source[MIXER_ADDR_LAST+1][2];
	struct ig_audio* vaudio;

	snd_pcm_substream_t *substreams[MAX_PCM_SUBSTREAMS];
} snd_card_iguana_t;

typedef struct snd_card_iguana_pcm {
	memsection_ref_t stream_ms;
	physmem_ref_t stream_pm;
	struct snd_dma_buffer buffer;

	spinlock_t lock;
	snd_pcm_substream_t *substream;
	struct ig_audio* vaudio;
} snd_card_iguana_pcm_t;

static snd_card_t *snd_iguana_cards[SNDRV_CARDS] = SNDRV_DEFAULT_PTR;


static void snd_card_iguana_pcm_timer_start(snd_pcm_substream_t * substream)
{
	snd_pcm_runtime_t *runtime = substream->runtime;
	snd_card_iguana_pcm_t *dpcm = runtime->private_data;
	struct ig_audio* vaudio = dpcm->vaudio;
	CORBA_Environment env;
//printk("%s:\n", __func__);
 	virtual_audio_control_stream(vaudio->server, vaudio->dev, 0,  STREAM_PLAY, &env);

}

static void snd_card_iguana_pcm_timer_stop(snd_pcm_substream_t * substream)
{
	snd_pcm_runtime_t *runtime = substream->runtime;
	snd_card_iguana_pcm_t *dpcm = runtime->private_data;
	struct ig_audio* vaudio = dpcm->vaudio;
	CORBA_Environment env;

//printk("%s:\n", __func__);
 	virtual_audio_control_stream(vaudio->server, vaudio->dev, 0,  STREAM_STOP, &env);
}

static int snd_card_iguana_playback_trigger(snd_pcm_substream_t * substream,
					   int cmd)
{
	if (cmd == SNDRV_PCM_TRIGGER_START) {
//printk("trigger: start\n");
		snd_card_iguana_pcm_timer_start(substream);
	} else if (cmd == SNDRV_PCM_TRIGGER_STOP) {
//printk("trigger: stop\n");
		snd_card_iguana_pcm_timer_stop(substream);
	} else {
		return -EINVAL;
	}
	return 0;
}

static int snd_card_iguana_capture_trigger(snd_pcm_substream_t * substream,
					  int cmd)
{
	if (cmd == SNDRV_PCM_TRIGGER_START) {
		snd_card_iguana_pcm_timer_start(substream);
	} else if (cmd == SNDRV_PCM_TRIGGER_STOP) {
		snd_card_iguana_pcm_timer_stop(substream);
	} else {
		return -EINVAL;
	}
	return 0;
}

static int snd_card_iguana_pcm_prepare(snd_pcm_substream_t * substream)
{
	snd_pcm_runtime_t *runtime = substream->runtime;
	snd_card_iguana_pcm_t *dpcm = runtime->private_data;
	struct ig_audio* vaudio = dpcm->vaudio;
	CORBA_Environment env;
	unsigned int bps;

//printk(" - rate: %d, chans: %d\n", runtime->rate, runtime->channels);
	bps = runtime->rate * runtime->channels;
	bps *= snd_pcm_format_width(runtime->format);
	bps /= 8;
	if (bps <= 0)
		return -EINVAL;
//printk(" - depth: %dbits, bps = %d\n", snd_pcm_format_width(runtime->format), bps);
//printk(" - buf size: %x, period size: %x\n", snd_pcm_lib_buffer_bytes(substream),
//		    snd_pcm_lib_period_bytes(substream));

//printk("%s:  dma buf = %p\n", __func__, runtime->dma_area);

	vaudio->config->frequency = runtime->rate;
	vaudio->config->channels = runtime->channels;
	vaudio->config->format = runtime->format;
	vaudio->config->period_bytes = snd_pcm_lib_period_bytes(substream);
	vaudio->config->buf_size = snd_pcm_lib_buffer_bytes(substream);	// override hw_param provided value

	virtual_audio_config_stream(vaudio->server, vaudio->dev, 0, 0, CMD_OFS, &env);
	return 0;
}

static int snd_card_iguana_playback_prepare(snd_pcm_substream_t * substream)
{
	return snd_card_iguana_pcm_prepare(substream);
}

static int snd_card_iguana_capture_prepare(snd_pcm_substream_t * substream)
{
	return snd_card_iguana_pcm_prepare(substream);
}

static snd_pcm_uframes_t snd_card_iguana_playback_pointer(snd_pcm_substream_t * substream)
{
	snd_pcm_runtime_t *runtime = substream->runtime;
	snd_card_iguana_pcm_t *dpcm = runtime->private_data;
	struct ig_audio* vaudio = dpcm->vaudio;
	CORBA_Environment env;
	uintptr_t pos;

	virtual_audio_get_buffer_pos(vaudio->server, vaudio->dev, 0, &pos, &env);
//printk("%s: %ld\n", __func__, pos);

	return bytes_to_frames(runtime, pos);
}

static snd_pcm_uframes_t snd_card_iguana_capture_pointer(snd_pcm_substream_t * substream)
{
	snd_pcm_runtime_t *runtime = substream->runtime;
	//snd_card_iguana_pcm_t *dpcm = runtime->private_data;

BUG();	// unimplemented
	return bytes_to_frames(runtime, 0);
}

static snd_pcm_hardware_t snd_card_iguana_playback =
{
	.info =			(SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_INTERLEAVED |
				 SNDRV_PCM_INFO_MMAP_VALID),
	.formats =		USE_FORMATS,
	.rates =		USE_RATE,
	.rate_min =		USE_RATE_MIN,
	.rate_max =		USE_RATE_MAX,
	.channels_min =		USE_CHANNELS_MIN,
	.channels_max =		USE_CHANNELS_MAX,
	.buffer_bytes_max =	MAX_BUFFER_SIZE,
	.period_bytes_min =	64,
	.period_bytes_max =	MAX_BUFFER_SIZE,
	.periods_min =		USE_PERIODS_MIN,
	.periods_max =		USE_PERIODS_MAX,
	.fifo_size =		0,
};

static snd_pcm_hardware_t snd_card_iguana_capture =
{
	.info =			(SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_INTERLEAVED |
				 SNDRV_PCM_INFO_MMAP_VALID),
	.formats =		USE_FORMATS,
	.rates =		USE_RATE,
	.rate_min =		USE_RATE_MIN,
	.rate_max =		USE_RATE_MAX,
	.channels_min =		USE_CHANNELS_MIN,
	.channels_max =		USE_CHANNELS_MAX,
	.buffer_bytes_max =	MAX_BUFFER_SIZE,
	.period_bytes_min =	64,
	.period_bytes_max =	MAX_BUFFER_SIZE,
	.periods_min =		USE_PERIODS_MIN,
	.periods_max =		USE_PERIODS_MAX,
	.fifo_size =		0,
};

static void snd_card_iguana_runtime_free(snd_pcm_runtime_t *runtime)
{
	snd_card_iguana_pcm_t *dpcm = runtime->private_data;
	kfree(dpcm);
}

static int snd_card_iguana_hw_params(snd_pcm_substream_t * substream,
				    snd_pcm_hw_params_t * hw_params)
{
	snd_pcm_runtime_t *runtime = substream->runtime;
	snd_card_iguana_pcm_t *dpcm = runtime->private_data;
	struct ig_audio* vaudio = dpcm->vaudio;
	CORBA_Environment env;

	int size = params_buffer_bytes(hw_params);
	uintptr_t vbase, pbase, psize;

//printk("%s:\n", __func__);
	if (dpcm->stream_ms != 0) {
		memsection_delete_dma(dpcm->stream_ms, dpcm->stream_pm);
		dpcm->stream_ms = 0;
	}

	dpcm->stream_ms = memsection_create_dma(size, &vbase, &dpcm->stream_pm, L4_WriteThroughMemory);

	if (dpcm->stream_ms == 0) {
		printk("%s: cannot alloc dma buffer: size %d\n", __func__, size);
		return -ENOMEM;
	}
	virtual_audio_add_memsection(vaudio->server, vaudio->dev, dpcm->stream_ms, 1, &env);
	physmem_info(dpcm->stream_pm, &pbase, &psize);

	vaudio->config->buf_memsect_idx = 1;
	vaudio->config->buf_pbase = pbase;
	vaudio->config->buf_size = psize;

	dpcm->buffer.area = (char*)vbase;
	dpcm->buffer.addr = pbase;
	dpcm->buffer.bytes = psize;
//printk(" - got dma buffer: %lx, phys: %lx\n", vbase, pbase);
//printk(" - req size = %d, buf size: %ld\n", params_buffer_bytes(hw_params), psize);
	snd_pcm_set_runtime_buffer(substream, &dpcm->buffer);
	return 1;
//	return snd_pcm_lib_malloc_pages(substream, params_buffer_bytes(hw_params));
}

static int snd_card_iguana_hw_free(snd_pcm_substream_t * substream)
{
	snd_pcm_runtime_t *runtime = substream->runtime;
	snd_card_iguana_pcm_t *dpcm = runtime->private_data;
//printk("%s:\n", __func__);
	if (dpcm->stream_ms != 0)
		memsection_delete_dma(dpcm->stream_ms, dpcm->stream_pm);
	dpcm->stream_ms = 0;
//	return snd_pcm_lib_free_pages(substream);
        return 0;
}

static int snd_card_iguana_playback_open(snd_pcm_substream_t * substream)
{
	snd_pcm_runtime_t *runtime = substream->runtime;
	snd_card_iguana_pcm_t *dpcm;
	struct ig_audio* vaudio;
	CORBA_Environment env;
	struct snd_card_iguana *snd_card;
	int err;
//printk("%s:\n", __func__);

	dpcm = kcalloc(1, sizeof(*dpcm), GFP_KERNEL);
	if (dpcm == NULL)
		return -ENOMEM;
	spin_lock_init(&dpcm->lock);
	dpcm->substream = substream;
	runtime->private_data = dpcm;
	runtime->private_free = snd_card_iguana_runtime_free;
	runtime->hw = snd_card_iguana_playback;
	if (substream->pcm->device & 1) {
		runtime->hw.info &= ~SNDRV_PCM_INFO_INTERLEAVED;
		runtime->hw.info |= SNDRV_PCM_INFO_NONINTERLEAVED;
	}
	if (substream->pcm->device & 2)
		runtime->hw.info &= ~(SNDRV_PCM_INFO_MMAP|SNDRV_PCM_INFO_MMAP_VALID);
	if ((err = add_playback_constraints(runtime)) < 0) {
		kfree(dpcm);
		return err;
	}

	snd_card = ((struct snd_card_iguana *)substream->pcm->private_data);
	snd_card->substreams[0] = substream;
	vaudio = snd_card->vaudio;
	dpcm->vaudio = vaudio;

	err = virtual_audio_open_stream(vaudio->server, vaudio->dev, 0, 0, CMD_OFS, &env);
	if (err < 0) {
		//kfree(dpcm);  -- done by private_free
		return -EBUSY;
	}

	return 0;
}

static int snd_card_iguana_capture_open(snd_pcm_substream_t * substream)
{
	snd_pcm_runtime_t *runtime = substream->runtime;
	snd_card_iguana_pcm_t *dpcm;
	int err;

	dpcm = kcalloc(1, sizeof(*dpcm), GFP_KERNEL);
	if (dpcm == NULL)
		return -ENOMEM;
	spin_lock_init(&dpcm->lock);
	dpcm->substream = substream;
	runtime->private_data = dpcm;
	runtime->private_free = snd_card_iguana_runtime_free;
	runtime->hw = snd_card_iguana_capture;
	if (substream->pcm->device == 1) {
		runtime->hw.info &= ~SNDRV_PCM_INFO_INTERLEAVED;
		runtime->hw.info |= SNDRV_PCM_INFO_NONINTERLEAVED;
	}
	if (substream->pcm->device & 2)
		runtime->hw.info &= ~(SNDRV_PCM_INFO_MMAP|SNDRV_PCM_INFO_MMAP_VALID);
	if ((err = add_capture_constraints(runtime)) < 0) {
		kfree(dpcm);
		return err;
	}

	return 0;
}

static int snd_card_iguana_playback_close(snd_pcm_substream_t * substream)
{
	snd_pcm_runtime_t *runtime = substream->runtime;
	snd_card_iguana_pcm_t *dpcm = runtime->private_data;
	struct snd_card_iguana *snd_card;
	struct ig_audio* vaudio = dpcm->vaudio;
	CORBA_Environment env;
//printk("%s:\n", __func__);

	virtual_audio_close_stream(vaudio->server, vaudio->dev, 0, &env);

	snd_card = ((struct snd_card_iguana *)substream->pcm->private_data);
	snd_card->substreams[0] = NULL;

	return 0;
}

static int snd_card_iguana_capture_close(snd_pcm_substream_t * substream)
{
	return 0;
}

static snd_pcm_ops_t snd_card_iguana_playback_ops = {
	.open =			snd_card_iguana_playback_open,
	.close =		snd_card_iguana_playback_close,
	.ioctl =		snd_pcm_lib_ioctl,
	.hw_params =		snd_card_iguana_hw_params,
	.hw_free =		snd_card_iguana_hw_free,
	.prepare =		snd_card_iguana_playback_prepare,
	.trigger =		snd_card_iguana_playback_trigger,
	.pointer =		snd_card_iguana_playback_pointer,
};

static snd_pcm_ops_t snd_card_iguana_capture_ops = {
	.open =			snd_card_iguana_capture_open,
	.close =		snd_card_iguana_capture_close,
	.ioctl =		snd_pcm_lib_ioctl,
	.hw_params =		snd_card_iguana_hw_params,
	.hw_free =		snd_card_iguana_hw_free,
	.prepare =		snd_card_iguana_capture_prepare,
	.trigger =		snd_card_iguana_capture_trigger,
	.pointer =		snd_card_iguana_capture_pointer,
};

static int __init snd_card_iguana_pcm(snd_card_iguana_t *iguana, int device, int substreams)
{
	snd_pcm_t *pcm;
	int err;

	if ((err = snd_pcm_new(iguana->card, "vPCM", device, substreams, substreams, &pcm)) < 0)
		return err;
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK, &snd_card_iguana_playback_ops);
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_CAPTURE, &snd_card_iguana_capture_ops);
	pcm->private_data = iguana;
	pcm->info_flags = 0;
	strcpy(pcm->name, "vPCM");
//	snd_pcm_lib_preallocate_pages_for_all(pcm, SNDRV_DMA_TYPE_CONTINUOUS,
//					      snd_dma_continuous_data(GFP_KERNEL),
//					      0, 64*1024);
	return 0;
}

#define DUMMY_VOLUME(xname, xindex, addr) \
{ .iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, .index = xindex, \
  .info = snd_iguana_volume_info, \
  .get = snd_iguana_volume_get, .put = snd_iguana_volume_put, \
  .private_value = addr }

static int snd_iguana_volume_info(snd_kcontrol_t * kcontrol, snd_ctl_elem_info_t * uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 2;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 100;
	return 0;
}
 
static int snd_iguana_volume_get(snd_kcontrol_t * kcontrol, snd_ctl_elem_value_t * ucontrol)
{
	snd_card_iguana_t *iguana = snd_kcontrol_chip(kcontrol);
	unsigned long flags;
	int addr = kcontrol->private_value & 0xff;

	spin_lock_irqsave(&iguana->mixer_lock, flags);
	ucontrol->value.integer.value[0] = iguana->mixer_volume[addr][0];
	ucontrol->value.integer.value[1] = iguana->mixer_volume[addr][1];
	spin_unlock_irqrestore(&iguana->mixer_lock, flags);
	return 0;
}

static int snd_iguana_volume_put(snd_kcontrol_t * kcontrol, snd_ctl_elem_value_t * ucontrol)
{
	snd_card_iguana_t *iguana = snd_kcontrol_chip(kcontrol);
	unsigned long flags;
	int l_change, r_change, addr = kcontrol->private_value;
	int left, right;
    uint32_t control;

    int index = kcontrol->private_value & 0xff;

	left = ucontrol->value.integer.value[0];
	right = ucontrol->value.integer.value[1];

	spin_lock_irqsave(&iguana->mixer_lock, flags);
	l_change = iguana->mixer_volume[addr][0] != left; 
	r_change = iguana->mixer_volume[addr][1] != right;
	iguana->mixer_volume[addr][0] = left;
	iguana->mixer_volume[addr][1] = right;
	spin_unlock_irqrestore(&iguana->mixer_lock, flags);



    if (l_change){
        control = 0;
        control = set_volume_left(&control, left+47);
        virtual_audio_mixer_ctrl_write(iguana->vaudio->server, iguana->vaudio->dev,0,control, NULL);
    }
    if (r_change){
        control = 0;
        control = set_volume_right(&control, right+47);
        virtual_audio_mixer_ctrl_write(iguana->vaudio->server, iguana->vaudio->dev,0,control, NULL);
    }


	return l_change || r_change;
}

#define DUMMY_CAPSRC(xname, xindex, addr) \
{ .iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, .index = xindex, \
  .info = snd_iguana_capsrc_info, \
  .get = snd_iguana_capsrc_get, .put = snd_iguana_capsrc_put, \
  .private_value = addr }

static int snd_iguana_capsrc_info(snd_kcontrol_t * kcontrol, snd_ctl_elem_info_t * uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
	uinfo->count = 2;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	return 0;
}
 
static int snd_iguana_capsrc_get(snd_kcontrol_t * kcontrol, snd_ctl_elem_value_t * ucontrol)
{
	snd_card_iguana_t *iguana = snd_kcontrol_chip(kcontrol);
	unsigned long flags;
	int addr = kcontrol->private_value;

	spin_lock_irqsave(&iguana->mixer_lock, flags);
	ucontrol->value.integer.value[0] = iguana->capture_source[addr][0];
	ucontrol->value.integer.value[1] = iguana->capture_source[addr][1];
	spin_unlock_irqrestore(&iguana->mixer_lock, flags);
	return 0;
}

static int snd_iguana_capsrc_put(snd_kcontrol_t * kcontrol, snd_ctl_elem_value_t * ucontrol)
{
	snd_card_iguana_t *iguana = snd_kcontrol_chip(kcontrol);
	unsigned long flags;
	int change, addr = kcontrol->private_value;
	int left, right;

	left = ucontrol->value.integer.value[0] & 1;
	right = ucontrol->value.integer.value[1] & 1;
	spin_lock_irqsave(&iguana->mixer_lock, flags);
	change = iguana->capture_source[addr][0] != left &&
	         iguana->capture_source[addr][1] != right;
	iguana->capture_source[addr][0] = left;
	iguana->capture_source[addr][1] = right;
	spin_unlock_irqrestore(&iguana->mixer_lock, flags);
	return change;
}

static snd_kcontrol_new_t snd_iguana_controls[] = {
DUMMY_VOLUME("Master Volume", 0, MIXER_ADDR_MASTER),
DUMMY_CAPSRC("Master Capture Switch", 0, MIXER_ADDR_MASTER),
DUMMY_VOLUME("Line Volume", 0, MIXER_ADDR_LINE),
DUMMY_CAPSRC("Line Capture Switch", 0, MIXER_ADDR_MASTER),
DUMMY_VOLUME("Mic Volume", 0, MIXER_ADDR_MIC),
DUMMY_CAPSRC("Mic Capture Switch", 0, MIXER_ADDR_MASTER),
};

static int __init snd_card_iguana_new_mixer(snd_card_iguana_t * iguana)
{
	snd_card_t *card = iguana->card;
	unsigned int idx;
	int err;

	snd_assert(iguana != NULL, return -EINVAL);
	spin_lock_init(&iguana->mixer_lock);
	strcpy(card->mixername, "vMixer");

	for (idx = 0; idx < ARRAY_SIZE(snd_iguana_controls); idx++) {
		if ((err = snd_ctl_add(card, snd_ctl_new1(&snd_iguana_controls[idx], iguana))) < 0)
			return err;
	}
	return 0;
}

/* Handle interrupts */
static irqreturn_t
ig_audio_interrupt(int irq, void *dev_id)
{
	struct snd_card_iguana * iguana = (struct snd_card_iguana *)dev_id;
	int i;

	//printk("ig_audio: interrupt!!\n");

	for (i = 0; i < MAX_PCM_SUBSTREAMS; i++)
	{
		snd_pcm_substream_t *substream = iguana->substreams[i];
		if (substream) {
			snd_pcm_runtime_t *runtime = substream->runtime;
			snd_card_iguana_pcm_t *dpcm = runtime->private_data;

			spin_lock_irq(&dpcm->lock);
			snd_pcm_period_elapsed(substream);
			spin_unlock_irq(&dpcm->lock);	
		}
	}

	return 0;
}

// XXX put in a header
extern int iguana_alloc_irq(void);
extern L4_ThreadId_t timer_thread;

static int __init snd_card_iguana_probe(int dev)
{
	snd_card_t *card;
	struct snd_card_iguana *iguana;
	int idx, err;
	struct ig_audio* vaudio;

	if (!enable[dev])
		return -ENODEV;

	printk("Iguana virtual AUDIO driver v1\n");

	// Scan Iguana for devices
	{
		thread_ref_t server_;
		L4_ThreadId_t server;
		CORBA_Environment env;

		memsection_lookup(env_memsection_base(iguana_getenv("OKL4_CORE_DEVICE_SERVER")),
                                  &server_);

		vaudio = kmalloc(sizeof(struct ig_audio), GFP_KERNEL);
		if (!vaudio) {
			ERROR_MSG("ig_audio: Cannot allocate new AUDIO device.\n");
			return -ENOMEM;
		}
		memset(vaudio, 0, sizeof(struct ig_audio));

		server = thread_l4tid(server_);

		/* Allocate an IRQ number */
		vaudio->irq = iguana_alloc_irq();

		/* Attach to the Audio server */
		vaudio->dev = device_core_get_audio(server, &vaudio->server,
				&timer_thread, IGUANA_IRQ_NOTIFY_MASK(vaudio->irq), &env);

		vaudio->control_ms = memsection_create(4096, &(vaudio->cmd_base));
		virtual_audio_add_memsection(vaudio->server, vaudio->dev, vaudio->control_ms, 0, &env);
		virtual_audio_register_control_block(vaudio->server, vaudio->dev, 0, 0, &env);

		vaudio->control = (struct ig_vaudio_control*)vaudio->cmd_base;
		vaudio->config  = (struct ig_vaudio_config*)(vaudio->cmd_base + CMD_OFS);
        }

	/* setup audio capabilities */
	snd_card_iguana_playback.rate_min = vaudio->control->min_freq;
	snd_card_iguana_playback.rate_min = vaudio->control->max_freq;
	snd_card_iguana_playback.rates = vaudio->control->freq_bitmask;
	snd_card_iguana_playback.channels_min = vaudio->control->min_channels;
	snd_card_iguana_playback.channels_max = vaudio->control->max_channels;
	snd_card_iguana_playback.formats = vaudio->control->format_bitmask;
	snd_card_iguana_playback.buffer_bytes_max = vaudio->control->buf_bytes_max;
	snd_card_iguana_playback.period_bytes_min = vaudio->control->period_bytes_min;
	snd_card_iguana_playback.period_bytes_max = vaudio->control->period_bytes_max;
#if 0
    uint32_t    streams;    /* eg: hardware mixed channels */
#endif

	card = snd_card_new(index[dev], id[dev], THIS_MODULE,
			    sizeof(struct snd_card_iguana));
	if (card == NULL)
		return -ENOMEM;
	iguana = (struct snd_card_iguana *)card->private_data;
	iguana->card = card;
	iguana->vaudio = vaudio;

	{
		printk("IG_vAUDIO on IRQ %d\n", vaudio->irq);
		request_irq(vaudio->irq, ig_audio_interrupt, 0, "IG_vAUDIO", iguana);
	}

	for (idx = 0; idx < MAX_PCM_DEVICES && idx < pcm_devs[dev]; idx++) {
		if (pcm_substreams[dev] < 1)
			pcm_substreams[dev] = 1;
		if (pcm_substreams[dev] > MAX_PCM_SUBSTREAMS)
			pcm_substreams[dev] = MAX_PCM_SUBSTREAMS;
		if ((err = snd_card_iguana_pcm(iguana, idx, pcm_substreams[dev])) < 0)
			goto __nodev;
	}
	if ((err = snd_card_iguana_new_mixer(iguana)) < 0)
		goto __nodev;
	strcpy(card->driver, "ig_audio");
	strcpy(card->shortname, "Iguana");
	sprintf(card->longname, "Iguana vaudio %i", dev + 1);
	if ((err = snd_card_register(card)) == 0) {
		snd_iguana_cards[dev] = card;
		return 0;
	}
      __nodev:
	kfree(vaudio);
	snd_card_free(card);
	return err;
}

static int __init alsa_card_iguana_init(void)
{
	int dev, cards;

	for (dev = cards = 0; dev < SNDRV_CARDS && enable[dev]; dev++) {
		if (snd_card_iguana_probe(dev) < 0) {
#ifdef MODULE
			printk(KERN_ERR "Iguana vaudio #%i not found or device busy\n", dev + 1);
#endif
			break;
		}
		cards++;
	}
	if (!cards) {
#ifdef MODULE
		printk(KERN_ERR "Iguanan vaudio not found or device busy\n");
#endif
		return -ENODEV;
	}
	return 0;
}

static void __exit alsa_card_iguana_exit(void)
{
	int idx;

	for (idx = 0; idx < SNDRV_CARDS; idx++)
		snd_card_free(snd_iguana_cards[idx]);
}

module_init(alsa_card_iguana_init)
module_exit(alsa_card_iguana_exit)
