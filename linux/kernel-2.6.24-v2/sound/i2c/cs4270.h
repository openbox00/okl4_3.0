/*
 *
 * CS4270 codec driver
 *
 * Copyright (c) 2006 National ICT Australia
 * 
 * Copyright (c) 2002 Hewlett-Packard Company
 *
 * Copyright (c) 2000 Nicolas Pitre <nico@cam.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License.
 */

#define CS4270_NAME "ak4535"

struct cs4270 {
	struct i2c_client client;
};

int     cs4270_command(struct i2c_client *clnt, unsigned int cmd, void *arg);
int     cs4270_mute(struct i2c_client *clnt, int mute);
uint8_t cs4270_read_reg (struct i2c_client *clnt, int regaddr);
void    cs4270_write_reg(struct i2c_client *clnt, int regaddr, int data);
