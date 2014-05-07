/*
 * CS4270 device driver
 *
 * Copyright (c) 2006 David Snowdon, National ICT Australia
 *
 * Copyright (c) 2002 Hewlett-Packard Company
 *
 * Copyright (c) 2000 Nicolas Pitre <nico@cam.org>
 *
 * Portions are Copyright (C) 2000 Lernout & Hauspie Speech Products, N.V.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/soundcard.h>
#include <linux/delay.h>

#include <asm/uaccess.h>
//#include <asm/mach-types.h>

#include <sound/driver.h>
#include <sound/core.h>
#include <sound/control.h>
#include <sound/initval.h>
#include <sound/info.h>

#include <sound/cs4270_l4i2c.h>

#include <pxa.h>
#include <pxa_i2c.h>

#define ADDRESS        0x48

#define REG_MAP        0x00
#define REG_ID         0x01
#define REG_PWRCNT     0x02
#define REG_FUNCMODE   0x03
#define REG_SFORMAT    0x04
#define REG_TC         0x05
#define REG_MUTE       0x06
#define REG_VOLA       0x07
#define REG_VOLB       0x08
#define REG_MAX        (REG_VOLB+1)

#define BIT(x)         (1<<(x))

#define REG_MAP_INCR   BIT(7)

#define REG_ID_ID(x)   (((x) >> 4) & 0x0F)
#define REG_ID_REV(x)  ((x) & 0x0F)

#define REG_PC_FREEZE  BIT(7)
#define REG_PC_PDN_ADC BIT(5)
#define REG_PC_PDN_DAC BIT(1)
#define REG_PC_PDN     BIT(0)

#define REG_FM_MODE1   BIT(5)
#define REG_FM_MODE0   BIT(4)
#define REG_FM_MCLK2   BIT(3)
#define REG_FM_MCLK1   BIT(2)
#define REG_FM_MCLK0   BIT(1)
#define REG_FM_PGDIS   BIT(0)

#define REG_SF_FREEZEA BIT(7)
#define REG_SF_FREEZEB BIT(6)
#define REG_SF_DIGLOOP BIT(5)
#define REG_SF_DACDIF1 BIT(4)
#define REG_SF_DACDIF0 BIT(3)
#define REG_SF_ADCDIF0 BIT(0)

#define REG_TC_DAC_SINGLEVOL BIT(7)
#define REG_TC_SOFTDAC       BIT(6)
#define REG_TC_ZCDAC         BIT(5)
#define REG_TC_ADCINVB       BIT(4)
#define REG_TC_ADCINVA       BIT(3)
#define REG_TC_DACINVB       BIT(2)
#define REG_TC_DACINVA       BIT(1)
#define REG_TC_DEEMPH        BIT(0)

#define REG_MUTE_AUTOMUTE    BIT(5)
#define REG_MUTE_ADCMUTEB    BIT(4)
#define REG_MUTE_ADCMUTEA    BIT(3)
#define REG_MUTE_POL         BIT(2)
#define REG_MUTE_DACMUTEB    BIT(1)
#define REG_MUTE_DACMUTEA    BIT(0)

struct cs4270 {
        unsigned char   regs[REG_MAX];
};

int     cs4270_command(unsigned int cmd, void *arg);
int     cs4270_mute(int mute);
uint8_t cs4270_read_reg (int regaddr);
void    cs4270_write_reg(int regaddr, int data);

static struct cs4270_reg_info {
	unsigned char num;
	unsigned char default_value;
} cs4270_reg_info[] = {
  { REG_PWRCNT,       0x00},  /* Nothing powered down */ 
  { REG_FUNCMODE,     REG_FM_MODE1 | REG_FM_MODE0},  
                              /* Slave mode, Single-speed */
                              /* Divide-by-1 MCLK */
                              /* Let Popguard to whatever it does */ 
  { REG_SFORMAT,      REG_SF_DACDIF0 },  /* I2S serial format */
  { REG_TC,           REG_TC_SOFTDAC | REG_TC_ZCDAC },
  { REG_MUTE,         REG_MUTE_AUTOMUTE }, 
  { REG_VOLA,         0x00}, /* Full volume for A and B */
  { REG_VOLB,         0x00}
};

struct cs4270 *mydata = NULL; 

uint8_t cs4270_read_reg (int regaddr)
{
	char buffer[2];
	buffer[0] = regaddr;  /* Bottom 4 bits of the MAP are register address */ 

	/* Too many problems getting the PXA25X i2c driver working, 
	   so I use our iguana PXA library one instead */ 
	i2c_write(ADDRESS, buffer, 1); 
	i2c_read(ADDRESS, buffer, 1);

	return buffer[0];
}

void cs4270_write_reg(int regaddr, int data)
{
	char buffer[2];

	buffer[0] = regaddr;
	buffer[1] = data;

#ifdef DEBUG_VERBOSE
	printk("cs4270: write %x %x\n", regaddr, data);
#endif

	/* Too many problems getting the PXA25X i2c driver working, 
	   so I use our iguana PXA library one instead */ 
	i2c_write(ADDRESS, buffer, 2); 
}

static void
cs4270_sync(void)
{
	int i;
#ifdef DEBUG_VERBOSE
	int addr; 
#endif

#ifdef DEBUG_VERBOSE
	for (i = 0; i < ARRAY_SIZE(cs4270_reg_info); i++) {
		addr = cs4270_reg_info[i].num; 
		printk("mydata->regs[%d] == %x\n", 
		       addr, mydata->regs[addr]);
	}
	for (i = 0; i < REG_MAX; i++) {
		int v = cs4270_read_reg(i);
		printk("register %x: old value %x\n", i, v);
	}
#endif

	for (i = 0; i < ARRAY_SIZE(cs4270_reg_info); i++){
		int addr = cs4270_reg_info[i].num; 
		cs4270_write_reg(addr, mydata->regs[addr]);
	}

#ifdef DEBUG_VERBOSE
	for (i = 0; i < ARRAY_SIZE(cs4270_reg_info); i++) {
		addr = cs4270_reg_info[i].num; 
		printk("mydata->regs[%d] == %x\n", 
		       addr, mydata->regs[addr]);
	}

	for (i = 0; i < REG_MAX; i++) {
		int v = cs4270_read_reg(i);
		printk("register %x: new value %x\n", i, v);
    }
#endif
}


/*
static void cs4270_power_down (void)
{
        cs4270_write_reg(REG_PWRCNT, REG_PC_PDN_ADC | REG_PC_PDN_DAC | REG_PC_PDN);
	}*/

#if 0 
static void cs4270_power_up (void)
{
        cs4270_write_reg(REG_PWRCNT, cs4270_reg_info[REG_PWRCNT].default_value);
}
#endif


static int cs4270_attach(void)		
{
	int i;

	mydata = kmalloc(sizeof(*mydata), GFP_KERNEL);
	if (!mydata)
		return -ENOMEM;

	memset(mydata, 0, sizeof(*mydata));

	/* set default values of registers */
	for (i = 0; i < ARRAY_SIZE(cs4270_reg_info); i++) {
	  int addr = cs4270_reg_info[i].num;
	  mydata->regs[addr] = cs4270_reg_info[i].default_value;
	}

	return 0;
}

static int cs4270_open(void)
{
	cs4270_attach();
	cs4270_sync(); 
	return 0;
}


static void cs4270_close(void)
{
	//	cs4270_power_down ();
}


int
snd_cs4270_activate( void )
{
    int ret;

#ifndef CONFIG_IBOX
    if (machine_is_ibox()) {
#endif
        ret = cs4270_open( );

	return ret;
#ifndef CONFIG_IBOX
    }
    else
        return -ENODEV;
#endif
}

void
snd_cs4270_deactivate( void )
{
#ifndef CONFIG_IBOX
    if (machine_is_ibox()) {
#endif
     cs4270_close();
#ifndef CONFIG_IBOX
    }
#endif
}

static int __init cs4270_init(void) { return 0; }
static void __exit cs4270_exit(void) {}

module_init(cs4270_init);
module_exit(cs4270_exit);

MODULE_AUTHOR("David Snowdon <David.Snowdon@nicta.com.au>");
MODULE_DESCRIPTION("CS4270 driver");
MODULE_LICENSE( "GPL" );

EXPORT_SYMBOL(snd_cs4270_activate);
EXPORT_SYMBOL(snd_cs4270_deactivate);
