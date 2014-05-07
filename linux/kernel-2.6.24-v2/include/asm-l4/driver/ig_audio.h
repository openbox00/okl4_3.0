/* 
 * Linux driver for Iguana virtual audio devices
 *
 * Copyright (C) 2007 Open Kernel Labs, Inc.
 *
 * Author: Carl van Schaik
 *
 * Released under GPL
 */

#ifndef __IG_AUDIO_H__
#define __IG_AUDIO_H__

#include <vaudio/audio.h>

#define CMD_OFS         0x400

struct ig_audio {
	objref_t dev;
	L4_ThreadId_t server;
	memsection_ref_t control_ms;
	struct ig_vaudio_control *control;
	struct ig_vaudio_config *config;

	int irq;

	unsigned long cmd_base;

	struct audio_info *next;
};

#endif /* __IG_AUDIO_H__ */
