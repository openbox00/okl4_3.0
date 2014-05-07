#ifndef _CS4270_L4I2C_H
#define _CS4270_L4I2C_H

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
int  cs4270_command(unsigned int cmd, void *arg);
struct i2c_client * cs4270_i2c_get_client( void );

#endif

