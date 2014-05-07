/*
 *
 * CS4270 codec driver (from AK4641)
 *
 * Copyright (c) 2006 National ICT Australia Pty, Ltd. 
 *
 * Based on code:
 *   Copyright (c) 2005 SDG Systems, LLC
 *   Copyright (c) 2002 Hewlett-Packard Company
 *   Copyright (c) 2000 Nicolas Pitre <nico@cam.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License.
 */

#ifndef _CS4270_H
#define _CS4270_H

#include <linux/i2c.h>

#define CS4270_NAME "cs4270"

#define I2C_CS4270_CONFIGURE    0x13800001
#define I2C_CS4270_OPEN         0x13800002
#define I2C_CS4270_CLOSE        0x13800003

struct cs4270_cfg {
    unsigned int dac_format:2;
    unsigned int adc_format:2;
};

int  snd_cs4270_activate(void);
void snd_cs4270_deactivate(void);
int  cs4270_command(struct i2c_client *clnt, unsigned int cmd, void *arg);
struct i2c_client * cs4270_i2c_get_client( void );

#endif

