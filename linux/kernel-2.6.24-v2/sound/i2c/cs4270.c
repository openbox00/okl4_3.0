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
#include <linux/i2c.h>
#include <linux/delay.h>

#include <asm/uaccess.h>
//#include <asm/mach-types.h>

#include <sound/driver.h>
#include <sound/core.h>
#include <sound/control.h>
#include <sound/initval.h>
#include <sound/info.h>

#include <sound/cs4270.h>

#ifdef CONFIG_L4
#ifdef CONFIG_IBOX
#define ADDRESS        0x48
#endif
#endif

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
	struct i2c_client client;
};

int     cs4270_command(struct i2c_client *clnt, unsigned int cmd, void *arg);
int     cs4270_mute(struct i2c_client *clnt, int mute);
uint8_t cs4270_read_reg (struct i2c_client *clnt, int regaddr);
void    cs4270_write_reg(struct i2c_client *clnt, int regaddr, int data);

//@@@ - i2c_get_client is defined but not declared in i2c.h
static struct i2c_client *cs4270_i2c_client=NULL; //@@@
struct i2c_client *cs4270_get_i2c_client(void) { return cs4270_i2c_client; }

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

uint8_t cs4270_read_reg (struct i2c_client *clnt, int regaddr)
{
	char buffer[2];
	int r;
	buffer[0] = regaddr;  /* Bottom 4 bits of the MAP are register address */ 

#ifdef CONFIG_L4
#ifdef CONFIG_IBOX
	/* Too many problems getting the PXA25X i2c driver working, 
	   so I use our iguana PXA library one instead */ 
	i2c_write(ADDRESS, buffer, 1); 
	i2c_read(ADDRESS, buffer, 1);
#else
	/*	r = i2c_master_send(clnt, buffer, 1);
	if (r != 1) {
		printk(KERN_ERR "cs4270: write failed, status %d\n", r);
		return -1;
		}*/
       


	r = i2c_master_recv(clnt, buffer, 1);
	if (r != 1) {
		printk(KERN_ERR "cs4270: read failed, status %d\n", r);
		return -1;
	}
#endif
#endif
	return buffer[0];
}

void cs4270_write_reg(struct i2c_client *clnt, int regaddr, int data)
{
	char buffer[2];
	int r;

	buffer[0] = regaddr;
	buffer[1] = data;

#ifdef DEBUG_VERBOSE
	printk("cs4270: write %x %x\n", regaddr, data);
#endif

#ifdef CONFIG_L4
#ifdef CONFIG_IBOX
	/* Too many problems getting the PXA25X i2c driver working, 
	   so I use our iguana PXA library one instead */ 
	i2c_write(ADDRESS, buffer, 2); 
#else
	r = i2c_master_send(clnt, buffer, 2);
	if (r != 2) {
		printk(KERN_ERR "cs4270: write failed, status %d\n", r);
	}
#endif
}

static void
cs4270_sync( struct i2c_client *clnt )
{
	struct cs4270 *mydata = NULL; 
	int i;
#ifdef DEBUG_VERBOSE
	int addr; 
#endif

	return; 

      	printk("cs4270: Synchronising"); 

       	if((mydata = i2c_get_clientdata(clnt)) == NULL)
       		printk(KERN_ERR "cs4270: i2c_get_clientdata returned NULL\n"); 


	printk("Got client data\n"); 

#ifdef DEBUG_VERBOSE
	for (i = 0; i < ARRAY_SIZE(cs4270_reg_info); i++) {
		addr = cs4270_reg_info[i].num; 
		printk("mydata->regs[%d] == %x\n", 
		       addr, mydata->regs[addr]);
	}
	for (i = 0; i < REG_MAX; i++) {
		int v = cs4270_read_reg(clnt, i);
		printk("register %x: old value %x\n", i, v);
	}
#endif

	printk("About to write registers\n"); 

	for (i = 0; i < ARRAY_SIZE(cs4270_reg_info); i++){
		int addr = cs4270_reg_info[i].num; 
		cs4270_write_reg(clnt, addr, mydata->regs[addr]);
	}

	printk("Wrote registers\n"); 


#ifdef DEBUG_VERBOSE
	for (i = 0; i < ARRAY_SIZE(cs4270_reg_info); i++) {
		addr = cs4270_reg_info[i].num; 
		printk("mydata->regs[%d] == %x\n", 
		       addr, mydata->regs[addr]);
	}

	for (i = 0; i < REG_MAX; i++) {
		int v = cs4270_read_reg(clnt, i);
		printk("register %x: new value %x\n", i, v);
    }
#endif
}


/*
static void cs4270_power_down (struct i2c_client *clnt)
{
        cs4270_write_reg(clnt, REG_PWRCNT, REG_PC_PDN_ADC | REG_PC_PDN_DAC | REG_PC_PDN);
	}*/

#if 0 
static void cs4270_power_up (struct i2c_client *clnt)
{
        cs4270_write_reg(clnt, REG_PWRCNT, cs4270_reg_info[REG_PWRCNT].default_value);
}
#endif


static void 
cs4270_cmd_init( struct i2c_client *clnt ){
	cs4270_sync(clnt); 
}

int 
cs4270_update(struct i2c_client *clnt, int cmd, void *arg)
{
  return -EINVAL;
}

static int
cs4270_configure( struct i2c_client *clnt, struct cs4270_cfg *conf )
{
  return 0;
}



static struct i2c_driver cs4270_driver;

static struct i2c_client client_template = {
	name: "(unset)",
	driver: &cs4270_driver
};

static int cs4270_attach(struct i2c_adapter *adap, int addr, int zero_or_minus_one)		
{
	struct cs4270 *mydata;
	struct i2c_client *clnt;
	int i;

	printk("Attaching\n"); 
	mydata = kmalloc(sizeof(*mydata), GFP_KERNEL);
	if (!mydata)
		return -ENOMEM;
	printk("Did kmalloc\n"); 

	memset(mydata, 0, sizeof(*mydata));

	/* set default values of registers */
	for (i = 0; i < ARRAY_SIZE(cs4270_reg_info); i++) {
	  int addr = cs4270_reg_info[i].num;
	  mydata->regs[addr] = cs4270_reg_info[i].default_value;
	}

	clnt = &mydata->client;
	memcpy(clnt, &client_template, sizeof(*clnt));
	clnt->adapter = adap;
	clnt->addr = addr;
	strcpy(clnt->name, "cs4270"); 

	cs4270_i2c_client = clnt;	/* no i2c_get_client, so export */

	printk("Attaching client\n"); 

#ifndef CONFIG_L4
	/* Hacked up version for iBox
	i2c_attach_client(clnt);
	i2c_set_clientdata(clnt, mydata); 
#endif

	printk("Done attach\n"); 

	return 0;
}

static int cs4270_detach_client(struct i2c_client *clnt)
{
	i2c_detach_client(clnt);

	kfree(clnt);

	return 0;
}

/* Addresses to scan */
//static unsigned short force0[] = {ANY_I2C_BUS, 0x48, 0x49, 0x4A, 0x4B, 
//				               0x4C, 0x4D, 0x4E, 0x4F};
//static unsigned short force1[] = {I2C_CLIENT_END, I2C_CLIENT_END};
static unsigned short normal_i2c[] = {0x48, 0x49, 0x4A, 0x4B, 
				      0x4C, 0x4D, 0x4E, 0x4F,I2C_CLIENT_END};
static unsigned short probe[]        = { I2C_CLIENT_END, I2C_CLIENT_END };
static unsigned short ignore[]       = { I2C_CLIENT_END, I2C_CLIENT_END };
//static unsigned short *forces[]        = { force0, force1, 0 };

static struct i2c_client_address_data addr_data = {
	.normal_i2c = normal_i2c,
	.probe = probe,
	.ignore = ignore
	//	.forces = forces
};

static int cs4270_attach_adapter(struct i2c_adapter *adap)
{
	printk("cs4270: Attaching adapter\n"); 
	return i2c_probe(adap, &addr_data, cs4270_attach);
}

static int cs4270_open(struct i2c_client *clnt)
{
	cs4270_cmd_init( clnt );
	return 0;
}

static void cs4270_close(struct i2c_client *clnt)
{
	cs4270_power_down (clnt);
}

int cs4270_command(struct i2c_client *clnt, unsigned int cmd, void *arg)
{
	int ret = -EINVAL;
	
	/* Do some command here */ 
	if (cmd == I2C_CS4270_CONFIGURE)
	  ret = cs4270_configure(clnt, arg);
	else if (cmd == I2C_CS4270_OPEN)
	  ret = cs4270_open(clnt);
	else if (cmd == I2C_CS4270_CLOSE)
	  (ret = 0), cs4270_close(clnt);

	return ret;
}

static struct i2c_driver cs4270_driver = {
        .driver = {
           .name = CS4270_NAME
        },
	.name = CS4270_NAME,
	.id		= I2C_DRIVERID_CS4270,
	.class          = I2C_CLASS_SOUND,
	.attach_adapter	= cs4270_attach_adapter,
	.detach_client	= cs4270_detach_client,
	.command        = cs4270_command
};

int
snd_cs4270_activate( void )
{
    int ret;

#ifndef CONFIG_IBOX
    if (machine_is_ibox()) {
#endif
	    printk("cs4270: Adding driver\n"); 
        ret = i2c_add_driver( &cs4270_driver );
	if (ret != 0) return ret;
	mdelay(10);

	printk("cs4270: Opening cs4270\n"); 
        ret = cs4270_open( cs4270_i2c_client );

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
	if (cs4270_i2c_client != NULL)
	    cs4270_close( cs4270_i2c_client );
        i2c_del_driver( &cs4270_driver );
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
