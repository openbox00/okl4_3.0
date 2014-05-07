/*
 *  linux/drivers/video/igvfb.c -- Iguana Virtual frame buffer device
 *
 *      Copyright (C) 2005 Carl van Schaik
 *
 *  Taken from vfb.c
 *      Copyright (C) 2002 James Simmons
 *
 *	Copyright (C) 1997 Geert Uytterhoeven
 *
 *  This file is subject to the terms and conditions of the GNU General Public
 *  License. See the file COPYING in the main directory of this archive for
 *  more details.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <linux/fb.h>
#include <linux/init.h>

#include <l4/types.h>


#include <interfaces/vlcd_client.h>
#include <iguana/memsection.h>
#include <iguana/thread.h>
#include <iguana/hardware.h>
#include <iguana/object.h>
#include <iguana/env.h>
#include <iguana/physmem.h>
#include <interfaces/devicecore_client.h>
#include <lcd/lcd.h>


#define DRIVER_VERSION "v1.0"
#define DRIVER_AUTHOR "Open Kernel Labs Inc."
#define DRIVER_DESC "Iguana virtual frame buffer driver"
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");


#define VIDEOMEMSIZE	(320*240*2)	/* 75K = 320X240 */

static int vfb_enable;

/* FIXME:
 * Unforunately we cant grab a bunch of memory from Iguana and specify that
 * we want it physicaly contigous. So we declare it staticaly here to ensure
 * it is.
 * This should go away once the Iguana API has been fixed
 */


static u_long videomemorysize = VIDEOMEMSIZE;
MODULE_PARM(videomemorysize, "l");

static struct fb_var_screeninfo vfb_default __initdata = {
	.xres =		240,
	.yres =		320,
	.xres_virtual =	240,
	.yres_virtual =	320,
	.bits_per_pixel = 16,
	.red =		{ 11, 5, 0 },
	.green =	{ 5, 6, 0 },
	.blue =		{ 0, 5, 0 },
	.activate =	FB_ACTIVATE_NOW,
	.height =	-1,
	.width =	-1,
	.pixclock =	20000,
	.left_margin =	64,
	.right_margin =	64,
	.upper_margin =	32,
	.lower_margin =	32,
	.hsync_len =	64,
	.vsync_len =	2,
	.vmode =	FB_VMODE_NONINTERLACED,
};

static struct fb_fix_screeninfo vfb_fix __initdata = {
	.id =		"Virtual FB",
	.type =		FB_TYPE_PACKED_PIXELS,
	.visual =	FB_VISUAL_TRUECOLOR,
	.xpanstep =	1,
	.ypanstep =	1,
	.ywrapstep =	1,
	.accel =	FB_ACCEL_NONE,
};


    /*
     *  Interface used by the world
     */
int igvfb_init(void);
int igvfb_setup(char *);

static int vfb_check_var(struct fb_var_screeninfo *var,
			 struct fb_info *info);
static int vfb_set_par(struct fb_info *info);
static int vfb_setcolreg(u_int regno, u_int red, u_int green, u_int blue,
			 u_int transp, struct fb_info *info);
static int vfb_pan_display(struct fb_var_screeninfo *var,
			   struct fb_info *info);

struct ig_fb {
    L4_ThreadId_t server;   /**< The Iguana lcd server */
    objref_t dev;           /**< The Iguana lcd device */
    uintptr_t memsect_base; /**< The base of the shared memory section */
    /** Point to the control block */
    struct control_block *control;
    /** Free list of packets */
};

struct ig_fb * ig_fb;



int 
vfb_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
        unsigned long arg, struct fb_info *info){

    int x = arg & 0xffff;
    int y = (arg >> 16) & 0xffff;

    switch(cmd){
    default:
        printk("UNKOWN FB ioctl: cmd=%d on (%d,%d)\n", cmd, x, y);
        return -1;
    }
    return 0;
}



static struct fb_ops vfb_ops = {
	.fb_check_var	= vfb_check_var,
	.fb_set_par	= vfb_set_par,
	.fb_setcolreg	= vfb_setcolreg,
	.fb_pan_display	= vfb_pan_display,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
	.fb_cursor	= soft_cursor,
        .fb_ioctl       = vfb_ioctl,
};

    /*
     *  Internal routines
     */

static u_long get_line_length(int xres_virtual, int bpp)
{
	u_long length;

	length = xres_virtual * bpp;
	length = (length + 31) & ~31;
	length >>= 3;
	return (length);
}

    /*
     *  Setting the video mode has been split into two parts.
     *  First part, xxxfb_check_var, must not write anything
     *  to hardware, it should only verify and adjust var.
     *  This means it doesn't alter par but it does use hardware
     *  data from it to check this var. 
     */

static int vfb_check_var(struct fb_var_screeninfo *var,
			 struct fb_info *info)
{
	u_long line_length;

	/*
	 *  FB_VMODE_CONUPDATE and FB_VMODE_SMOOTH_XPAN are equal!
	 *  as FB_VMODE_SMOOTH_XPAN is only used internally
	 */

	if (var->vmode & FB_VMODE_CONUPDATE) {
		var->vmode |= FB_VMODE_YWRAP;
		var->xoffset = info->var.xoffset;
		var->yoffset = info->var.yoffset;
	}

	/*
	 *  Some very basic checks
	 */
	if (!var->xres)
		var->xres = 1;
	if (!var->yres)
		var->yres = 1;
	if (var->xres > var->xres_virtual)
		var->xres_virtual = var->xres;
	if (var->yres > var->yres_virtual)
		var->yres_virtual = var->yres;
	if (var->bits_per_pixel <= 1)
		var->bits_per_pixel = 1;
	else if (var->bits_per_pixel <= 8)
		var->bits_per_pixel = 8;
	else if (var->bits_per_pixel <= 16)
		var->bits_per_pixel = 16;
	else if (var->bits_per_pixel <= 24)
		var->bits_per_pixel = 24;
	else if (var->bits_per_pixel <= 32)
		var->bits_per_pixel = 32;
	else
		return -EINVAL;

	if (var->xres_virtual < var->xoffset + var->xres)
		var->xres_virtual = var->xoffset + var->xres;
	if (var->yres_virtual < var->yoffset + var->yres)
		var->yres_virtual = var->yoffset + var->yres;

	/*
	 *  Memory limit
	 */
	line_length =
	    get_line_length(var->xres_virtual, var->bits_per_pixel);
	if (line_length * var->yres_virtual > videomemorysize)
		return -ENOMEM;

	/*
	 * Now that we checked it we alter var. The reason being is that the video
	 * mode passed in might not work but slight changes to it might make it 
	 * work. This way we let the user know what is acceptable.
	 */
	switch (var->bits_per_pixel) {
	case 1:
	case 8:
		var->red.offset = 0;
		var->red.length = 8;
		var->green.offset = 0;
		var->green.length = 8;
		var->blue.offset = 0;
		var->blue.length = 8;
		var->transp.offset = 0;
		var->transp.length = 0;
		break;
	case 16:		/* RGBA 5551 */
		if (var->transp.length) {
			var->red.offset = 10;
			var->red.length = 5;
			var->green.offset = 5;
			var->green.length = 5;
			var->blue.offset = 0;
			var->blue.length = 5;
			var->transp.offset = 15;
			var->transp.length = 1;
		} else {	/* RGB 565 */
			var->red.offset = 11;
			var->red.length = 5;
			var->green.offset = 5;
			var->green.length = 6;
			var->blue.offset = 0;
			var->blue.length = 5;
			var->transp.offset = 0;
			var->transp.length = 0;
		}
		break;
	case 24:		/* RGB 888 */
		var->red.offset = 0;
		var->red.length = 8;
		var->green.offset = 8;
		var->green.length = 8;
		var->blue.offset = 16;
		var->blue.length = 8;
		var->transp.offset = 0;
		var->transp.length = 0;
		break;
	case 32:		/* RGBA 8888 */
		var->red.offset = 0;
		var->red.length = 8;
		var->green.offset = 8;
		var->green.length = 8;
		var->blue.offset = 16;
		var->blue.length = 8;
		var->transp.offset = 24;
		var->transp.length = 8;
		break;
	}
	var->red.msb_right = 0;
	var->green.msb_right = 0;
	var->blue.msb_right = 0;
	var->transp.msb_right = 0;

	return 0;
}

/* This routine actually sets the video mode. It's in here where we
 * the hardware state info->par and fix which can be affected by the 
 * change in par. For this driver it doesn't do much. 
 */
static int vfb_set_par(struct fb_info *info)
{
	info->fix.line_length = get_line_length(info->var.xres_virtual,
						info->var.bits_per_pixel);
	return 0;
}

    /*
     *  Set a single color register. The values supplied are already
     *  rounded down to the hardware's capabilities (according to the
     *  entries in the var structure). Return != 0 for invalid regno.
     */

static int vfb_setcolreg(u_int regno, u_int red, u_int green, u_int blue,
			 u_int transp, struct fb_info *info)
{
    return 0;

	if (regno >= 256)	/* no. of hw registers */
		return 1;
	/*
	 * Program hardware... do anything you want with transp
	 */

	/* grayscale works only partially under directcolor */
	if (info->var.grayscale) {
		/* grayscale = 0.30*R + 0.59*G + 0.11*B */
		red = green = blue =
		    (red * 77 + green * 151 + blue * 28) >> 8;
	}

	/* Directcolor:
	 *   var->{color}.offset contains start of bitfield
	 *   var->{color}.length contains length of bitfield
	 *   {hardwarespecific} contains width of RAMDAC
	 *   cmap[X] is programmed to (X << red.offset) | (X << green.offset) | (X << blue.offset)
	 *   RAMDAC[X] is programmed to (red, green, blue)
	 * 
	 * Pseudocolor:
	 *    uses offset = 0 && length = RAMDAC register width.
	 *    var->{color}.offset is 0
	 *    var->{color}.length contains widht of DAC
	 *    cmap is not used
	 *    RAMDAC[X] is programmed to (red, green, blue)
	 * Truecolor:
	 *    does not use DAC. Usually 3 are present.
	 *    var->{color}.offset contains start of bitfield
	 *    var->{color}.length contains length of bitfield
	 *    cmap is programmed to (red << red.offset) | (green << green.offset) |
	 *                      (blue << blue.offset) | (transp << transp.offset)
	 *    RAMDAC does not exist
	 */
#define CNVT_TOHW(val,width) ((((val)<<(width))+0x7FFF-(val))>>16)
	switch (info->fix.visual) {
	case FB_VISUAL_TRUECOLOR:
	case FB_VISUAL_PSEUDOCOLOR:
		red = CNVT_TOHW(red, info->var.red.length);
		green = CNVT_TOHW(green, info->var.green.length);
		blue = CNVT_TOHW(blue, info->var.blue.length);
		transp = CNVT_TOHW(transp, info->var.transp.length);
		break;
	case FB_VISUAL_DIRECTCOLOR:
		red = CNVT_TOHW(red, 8);	/* expect 8 bit DAC */
		green = CNVT_TOHW(green, 8);
		blue = CNVT_TOHW(blue, 8);
		/* hey, there is bug in transp handling... */
		transp = CNVT_TOHW(transp, 8);
		break;
	}
#undef CNVT_TOHW
	/* Truecolor has hardware independent palette */
	if (info->fix.visual == FB_VISUAL_TRUECOLOR) {
		u32 v;

		if (regno >= 16)
			return 1;

		v = (red << info->var.red.offset) |
		    (green << info->var.green.offset) |
		    (blue << info->var.blue.offset) |
		    (transp << info->var.transp.offset);
		switch (info->var.bits_per_pixel) {
		case 8:
			break;
		case 16:
			((u32 *) (info->pseudo_palette))[regno] = v;
			break;
		case 24:
		case 32:
			((u32 *) (info->pseudo_palette))[regno] = v;
			break;
		}
		return 0;
	}
	return 0;
}

    /*
     *  Pan or Wrap the Display
     *
     *  This call looks only at xoffset, yoffset and the FB_VMODE_YWRAP flag
     */

static int vfb_pan_display(struct fb_var_screeninfo *var,
			   struct fb_info *info)
{
	if (var->vmode & FB_VMODE_YWRAP) {
		if (var->yoffset < 0
		    || var->yoffset >= info->var.yres_virtual
		    || var->xoffset)
			return -EINVAL;
	} else {
		if (var->xoffset + var->xres > info->var.xres_virtual ||
		    var->yoffset + var->yres > info->var.yres_virtual)
			return -EINVAL;
	}
	info->var.xoffset = var->xoffset;
	info->var.yoffset = var->yoffset;
	if (var->vmode & FB_VMODE_YWRAP)
		info->var.vmode |= FB_VMODE_YWRAP;
	else
		info->var.vmode &= ~FB_VMODE_YWRAP;
	return 0;
}

int __init igvfb_setup(char *options)
{
	char *this_opt;

	vfb_enable = 1;

	if (!options || !*options)
		return 1;

	while ((this_opt = strsep(&options, ",")) != NULL) {
		if (!*this_opt)
			continue;
		if (!strncmp(this_opt, "disable", 7))
			vfb_enable = 0;
	}
	return 1;
}

uint16_t  *fbuffer;

extern L4_ThreadId_t timer_thread;
static int __init ig_fb_init(void){
    thread_ref_t server_;
    L4_ThreadId_t server;
	CORBA_Environment env;

    uintptr_t vbase, pbase, psize;
    memsection_ref_t ms;
    physmem_ref_t pm;
    struct fb_info *info = NULL;
    uint32_t x, y, bpp;
    int retval;

    info = framebuffer_alloc(sizeof(u32) * 256, NULL);
    if (!info){
        printk("FB: alloc info failed\n");
        return -ENOMEM;
    }

    ig_fb = kmalloc(sizeof(*ig_fb), GFP_KERNEL);
    if ( !ig_fb ){
        /* free stuff */
        return -ENOMEM;
    }
    memset(info, 0, sizeof(u32) * 256);
    memset(ig_fb, 0, sizeof(*ig_fb));

    /* First, we find the input device */
    memsection_lookup(env_memsection_base(iguana_getenv("OKL4_CORE_DEVICE_SERVER")),
                      &server_);
    server = thread_l4tid(server_);
    ig_fb->dev = device_core_get_vdevice(server, &ig_fb->server, &timer_thread, 8, "lcd",  &env);

    /* Now, we add the kernel memory section to the server */
    /* ?? */

    /* Initialise the control block */
    /* ?? */

    virtual_lcd_get_mode(ig_fb->server, ig_fb->dev, &x, &y, &bpp, &env);

    vfb_default.bits_per_pixel = bpp;
    vfb_default.xres = x;
    vfb_default.yres = y;
    vfb_default.xres_virtual = x;
    vfb_default.yres_virtual = y;
    
    videomemorysize = x*y*(bpp/8);

    ms = memsection_create_dma(videomemorysize, &vbase, &pm, L4_WriteThroughMemory);
    physmem_info(pm, &pbase, &psize);

    fbuffer = (uint16_t*)vbase;

    virtual_lcd_set_fb(ig_fb->server, ig_fb->dev, vbase, &env);


    info->screen_base = (void*)fbuffer;
    info->fbops = &vfb_ops;
    info->var = vfb_default;

//    retval = fb_find_mode(&info->var, info, NULL,NULL, 0, NULL, 16);
//    if (!retval || retval == 4)
//		info->var = vfb_default;
	info->fix = vfb_fix;
	vfb_set_par(info);

	info->fix.smem_len = videomemorysize;
	info->fix.smem_start = (unsigned long)info->screen_base;
	info->pseudo_palette = info->par;
	info->par = NULL;
	info->flags = FBINFO_FLAG_DEFAULT;

	retval = fb_alloc_cmap(&info->cmap, 256, 0);
	if (retval < 0)
        while (1) printk("alloc cmap failed\n");




	retval = register_framebuffer(info);
	if (retval < 0){
        printk("FB: register failed\n");
        return -ENOMEM;
    }


    printk(KERN_INFO DRIVER_DESC " " DRIVER_VERSION "\n");

    return 0;
}

static void __exit ig_fb_exit(void){
}
module_init(ig_fb_init);
module_exit(ig_fb_exit);

